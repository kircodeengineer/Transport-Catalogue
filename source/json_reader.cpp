#include "json_reader.h"
#include "json_builder.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

JsonReader::JsonReader(const json::Document& doc) :
	doc_(doc),
	request_handler() {}

void JsonReader::ProcessInputStopRequest(std::list<size_t>& route_requests_ids, std::list<size_t>& stop_requests_ids) {
	const json::Node& input_requests = doc_.GetRoot().AsDict().at("base_requests").AsArray();
	size_t id = 0;
	for (const auto& request : input_requests.AsArray()) {
		const auto& request_map = request.AsDict();
		// запрос ввода маршрута
		if (request_map.at("type").AsString() == "Bus") {
			route_requests_ids.push_back(id);
		}
		// запрос ввода остановки
		else if (request_map.at("type").AsString() == "Stop") {
			stop_requests_ids.push_back(id);
			domain::StopDataRoutes stop;
			std::string stop_name(request_map.at("name").AsString());
			// вызов данного метода пугает компилятор и тот отключает SSO (оптимизацию коротких строк)
			// при коротких строках, вызывался memcpy заместо swap укзателей.
			stop_name.reserve(sizeof(stop_name) + 1);
			stop.name = std::move(stop_name);
			stop.coordinates.lat = request_map.at("latitude").AsDouble();
			stop.coordinates.lng = request_map.at("longitude").AsDouble();
			// запись в транспортный справочник остановки
			request_handler.AddStopInTransportCatalogue(std::move(stop));
		}
		++id;
	}
}

void JsonReader::ProcessInputStopsLengthsRequest(const std::list<size_t>& stop_requests_ids) {
	const json::Node& input_requests = doc_.GetRoot().AsDict().at("base_requests").AsArray();
	for (const auto& id : stop_requests_ids) {
		const auto& request_map = input_requests.AsArray().at(id).AsDict();
		const auto& road_distances = request_map.at("road_distances").AsDict();
		std::string_view stop_a(request_map.at("name").AsString());
		for (const auto& [stop, distance] : road_distances) {
			std::string_view stop_b(stop);
			request_handler.AddLengthBetweenStops(stop_a, stop_b, distance.AsInt());
		}
	}
}

void JsonReader::ProcessInputRouteRequest(const std::list<size_t>& route_requests_ids) {
	const json::Node& input_requests = doc_.GetRoot().AsDict().at("base_requests").AsArray();
	for (const auto& id : route_requests_ids) {
		const auto& request_map = input_requests.AsArray().at(id).AsDict();
		if (request_map.at("type").AsString() == "Bus") {
			domain::RouteData route_data;
			route_data.type = request_map.at("is_roundtrip").AsBool() == true ? domain::route_marks::CIRCLE : domain::route_marks::TO_AND_BACK;
			auto route_name = request_map.at("name").AsString();
			// вызов данного метода пугает компилятор и тот отключает SSO (оптимизацию коротких строк)
			// при коротких строках, вызывался memcpy заместо swap укзателей.
			route_name.reserve(sizeof(route_name) + 1);
			route_data.name = std::move(route_name);
			const auto& stops_ = request_map.at("stops").AsArray();
			for (const auto& stop_node : stops_) {
				const auto& stop_name = stop_node.AsString();
				const auto& stop = request_handler.GetStopStat(stop_name);
				if (stop == std::nullopt) {
					continue;
				}
				auto it = route_data.stops_.insert(route_data.stops_.end(), *stop);
				route_data.unique_stops.insert(*stop);
				if (route_data.stops_.size() > 1) {
					auto it_prev = std::next(it, -1);
					route_data.length_straight += geo::ComputeDistance((*it_prev)->coordinates, (*it)->coordinates);
					route_data.length += ((*it_prev)->lengths_to_stops.at((*it)->name));
				}
			}
			route_data.stops_count = route_data.stops_.size();
			// вторая половина маршрута в обратном направление, если маршрут типа ТУДА-И-ОБРАТНО
			if (route_data.stops_count > 0 && route_data.type == domain::route_marks::TO_AND_BACK) {
				// расстояние между остановками по географическим координатам увеличивается в двое
				route_data.length_straight *= 2.0;
				// добавляем число остановок в обратном направление
				route_data.stops_count *= 2;
				// убираем остановку с которой начинаем обратный путь
				--route_data.stops_count;
				for (auto it = std::next(route_data.stops_.crbegin(), 1); it != route_data.stops_.rend(); std::advance(it, 1)) {
					auto it_prev = std::next(it, -1);
					route_data.length += ((*it_prev)->lengths_to_stops.at((*it)->name));
				}
			}
			route_data.curvature = route_data.length / route_data.length_straight;
			route_data.unique_stops_count = route_data.unique_stops.size();
			// Добавляем маршрут в справочник
			request_handler.AddRouteInTransportCatalogue(std::move(route_data));
		}
	}
}

void JsonReader::ProcessTransportCatalogueInputData() {
	std::list<size_t> stop_requests_ids;
	std::list<size_t> route_requests_ids;
	ProcessInputStopRequest(route_requests_ids, stop_requests_ids);
	ProcessInputStopsLengthsRequest(stop_requests_ids);
	ProcessInputRouteRequest(route_requests_ids);
}

void JsonReader::ProcessRoutingSettings() {
	const json::Dict& routing_settings = doc_.GetRoot().AsDict().at("routing_settings").AsDict();
	request_handler.InitTransportRouter(routing_settings.at("bus_wait_time").AsDouble(), routing_settings.at("bus_velocity").AsDouble());
}

/// @brief обработка цвета
/// @param color цвет в json формате
/// @return строка с SVG цветом
std::string ProcessColor(const json::Node& color) {
	using namespace std::literals;
	std::ostringstream strm;
	strm.str(std::string());
	if (color.IsArray()) {
		bool is_rgba = color.AsArray().size() == 4 ? true : false;
		if (is_rgba) {
			strm << "rgba("sv << color.AsArray().at(0).AsInt() << ','
				<< color.AsArray().at(1).AsInt() << ','
				<< color.AsArray().at(2).AsInt() << ','
				<< color.AsArray().at(3).AsDouble() << ')';
		}
		else {
			strm << "rgb("sv << color.AsArray().at(0).AsInt() << ','
				<< color.AsArray().at(1).AsInt() << ','
				<< color.AsArray().at(2).AsInt() << ')';
		}
	}
	else {
		strm << color.AsString();
	}
	return strm.str();
}

map_renderer::RenderSettings JsonReader::ProcessRenderSettings() {
	map_renderer::RenderSettings result;
	const json::Dict& render_settings = doc_.GetRoot().AsDict().at("render_settings").AsDict();

	result.width = render_settings.at("width").AsDouble();
	result.height = render_settings.at("height").AsDouble();
	result.padding = render_settings.at("padding").AsDouble();
	result.stroke_width = render_settings.at("line_width").AsDouble();
	result.stroke_linecap = svg::StrokeLineCap::ROUND;
	result.stroke_linejoin = svg::StrokeLineJoin::ROUND;

	for (size_t i = 0; i < render_settings.at("color_palette").AsArray().size(); ++i) {
		result.color_palette.push_back(ProcessColor(render_settings.at("color_palette").AsArray().at(i)));
	}

	result.underlayer_stroke_color = ProcessColor(render_settings.at("underlayer_color"));

	result.underlayer_stroke_width = render_settings.at("underlayer_width").AsDouble();
	result.route_offset.x = render_settings.at("bus_label_offset").AsArray().at(0).AsDouble();
	result.route_offset.y = render_settings.at("bus_label_offset").AsArray().at(1).AsDouble();
	result.route_size = render_settings.at("bus_label_font_size").AsInt();
	result.stop_offset.x = render_settings.at("stop_label_offset").AsArray().at(0).AsDouble();
	result.stop_offset.y = render_settings.at("stop_label_offset").AsArray().at(1).AsDouble();
	result.stop_size = render_settings.at("stop_label_font_size").AsInt();
	result.stop_radius = render_settings.at("stop_radius").AsDouble();

	return result;
}

void JsonReader::ProcessMapRendererInputData() {
	// настройки визуализации карты
	request_handler.SetRenderSettings(ProcessRenderSettings());
	request_handler.PrepareSvgDoc();
}

void JsonReader::ProcessInputRequests() {
	ProcessTransportCatalogueInputData();
	ProcessMapRendererInputData();
	ProcessRoutingSettings();
}

json::Document JsonReader::ProcessStatRequests() {
	using namespace std::literals;
	const auto& stat_requests = doc_.GetRoot().AsDict().at("stat_requests"s);
	json::Builder node;
	node.StartArray();

	for (const auto& stat_request : stat_requests.AsArray()) {
		const auto& request = stat_request.AsDict();
		const auto& request_id = request.at("id"s).AsInt();
		const auto& type = request.at("type"s);

		// foo
		if (type == "Bus"s) {
			std::string name = request.at("name"s).AsString();
			auto route_data = request_handler.GetBusStat(name);
			if (route_data != std::nullopt) {
				json::Node value_node{ json::Builder{}.StartDict()
						.Key("request_id"s).Value(request_id)
						.Key("curvature"s).Value((*route_data)->curvature)
						.Key("route_length"s).Value((*route_data)->length)
						.Key("stop_count"s).Value(static_cast<int>((*route_data)->stops_count))
						.Key("unique_stop_count"s).Value(static_cast<int>((*route_data)->unique_stops_count))
						.EndDict().Build() };
				node.Value(value_node);
			}
			else {
				json::Node value_node{ json::Builder{}.StartDict()
							.Key("request_id"s).Value(request_id)
							.Key("error_message"s).Value("not found"s)
							.EndDict().Build() };
				node.Value(value_node);
			}
		}

		// foo
		if (type == "Stop"s) {
			std::string name = request.at("name"s).AsString();
			const auto& stop_data = request_handler.GetStopStat(name);
			if (stop_data != std::nullopt) {
				json::Array routes;
				for (const auto& route_data : **request_handler.GetBusesByStop((*stop_data)->name)) {
					routes.push_back(route_data->name);
				}
				json::Node value_node{ json::Builder{}.StartDict()
							.Key("request_id"s).Value(request_id)
							.Key("buses"s).Value(routes)
							.EndDict().Build() };
				node.Value(value_node);
			}
			else {
				json::Node value_node{ json::Builder{}.StartDict()
							.Key("request_id"s).Value(request_id)
							.Key("error_message"s).Value("not found"s)
							.EndDict().Build() };
				node.Value(value_node);
			}
		}

		// foo
		if (type == "Map"s) {
			json::Node value_node{ json::Builder{}.StartDict()
						.Key("request_id"s).Value(request_id)
						.Key("map"s).Value(request_handler.RenderMap())
						.EndDict().Build() };
			node.Value(value_node);
		}

		// foo
		if (type == "Route"s) {
			auto route = request_handler.FindPath(request.at("from"s).AsString(), request.at("to"s).AsString());
			if (route != std::nullopt) {
				json::Array routes;
				for (const auto& item : (*route).items) {
					if (item.IsWait()) {
						auto wait = item.AsWait();
						json::Node item_node{ json::Builder{}.StartDict()
									.Key("type"s).Value(std::string(wait.type_))
									.Key("stop_name"s).Value(std::string(wait.stop_name_))
									.Key("time"s).Value(wait.time_)
									.EndDict().Build() };
						routes.push_back(item_node);
					}
					else if (item.IsBus()) {
						auto bus = item.AsBus();
						json::Node item_node{ json::Builder{}.StartDict()
									.Key("type"s).Value(std::string(bus.type_))
									.Key("bus"s).Value(std::string(bus.bus_))
									.Key("span_count"s).Value(bus.span_count_)
									.Key("time"s).Value(bus.time_)
									.EndDict().Build() };
						routes.push_back(item_node);
					}
				}
				json::Node value_node{ json::Builder{}.StartDict()
							.Key("request_id"s).Value(request_id)
							.Key("total_time"s).Value((*route).total_time)
							.Key("items"s).Value(routes)
							.EndDict().Build() };
				node.Value(value_node);
			}
			else {
				json::Node value_node{ json::Builder{}.StartDict()
							.Key("request_id"s).Value(request_id)
							.Key("error_message"s).Value("not found"s)
							.EndDict().Build() };
				node.Value(value_node);
			}
		}
	}
	node.EndArray();
	return json::Document{ node.Build() };
}

json::Document JsonReader::ProcessJsonDoc() {
	ProcessInputRequests();
	auto result = ProcessStatRequests();
	return result;
}
