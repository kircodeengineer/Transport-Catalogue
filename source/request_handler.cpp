#include "request_handler.h"
#include <sstream>
#include <memory>
//#include <fstream>
/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */
namespace request_handler {
	RequestHandler::RequestHandler() : transport_catalogue_(), transport_router_(transport_catalogue_), map_renderer_() {}

	std::optional<const domain::RouteData*> RequestHandler::GetBusStat(std::string_view bus_name) const {
		auto result = transport_catalogue_.GetRoute(bus_name);
		if (result != nullptr) {
			return result;
		}
		return std::nullopt;
	}

	std::optional<const domain::StopDataRoutes*> RequestHandler::GetStopStat(std::string_view stop_name) const {
		auto result = transport_catalogue_.GetStop(stop_name);
		if (result != nullptr) {
			return result;
		}
		return std::nullopt;
	}

	std::string RequestHandler::RenderMap() const {
		std::ostringstream strm_svg;
		map_renderer_.RenderMap(strm_svg);
		//std::ofstream log_svg("log.svg");
		//log_svg << strm_svg.str();
		return strm_svg.str();
	}

	std::optional<const std::set<const domain::RouteData*, domain::PointerRouteDataCompareLow>*> RequestHandler::GetBusesByStop(std::string_view stop_name) const {
		const auto stop_data = transport_catalogue_.GetStop(stop_name);
		if (stop_data != nullptr) {
			return &(stop_data->routes_);
		}
		return std::nullopt;
	}

	void RequestHandler::AddStopInTransportCatalogue(domain::StopDataRoutes&& stop_data) {
		transport_catalogue_.AddStop(std::move(stop_data));
	}

	void RequestHandler::AddRouteInTransportCatalogue(domain::RouteData&& route_data) {
		transport_catalogue_.AddRoute(std::move(route_data));
	}

	void RequestHandler::AddLengthBetweenStops(std::string_view stop_a, std::string_view stop_b, int distance) {
		transport_catalogue_.AddLengthBetweenStops(stop_a, stop_b, distance);
	}

	void RequestHandler::InitTransportRouter(double bus_wait_time, double bus_velocity) {
		transport_router_.Init(bus_wait_time, bus_velocity);
	}

	std::optional<TransportRouter::Path> RequestHandler::FindPath(const std::string& from, const std::string& to) const {
		return transport_router_.FindPath(from, to);
	}


	void RequestHandler::SetRenderSettings(map_renderer::RenderSettings&& render_settings) {
		map_renderer_.SetRenderSettings(std::move(render_settings));
	}


	void RequestHandler::PrepareSvgDoc() {
		map_renderer_.SetCoordinatesContainerSize(transport_catalogue_.GetStopsCount());

		auto all_stops = transport_catalogue_.GetAllStops();
		for (const auto& [stop_name, stop_data] : all_stops) {
			// нам нужны только координаты остановок входящих в какой-либо из маршрутов
			if (!stop_data.routes_.empty()) {
				map_renderer_.AddGeoCoords(stop_data.coordinates);
			}
		}

		auto all_routes = transport_catalogue_.GetAllRoutes();
		for (const auto& [route_name, route_data] : all_routes) {
			map_renderer_.AddRoute(&route_data);
		}

		map_renderer_.PrepareSvgDoc();
	}
} //namespace request_handler
