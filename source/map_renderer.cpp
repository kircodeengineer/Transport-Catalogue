#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace map_renderer {

	void MapRenderer::SetRenderSettings(RenderSettings&& render_settings) {
		render_settings_ = std::move(render_settings);
	}

	void MapRenderer::SetCoordinatesContainerSize(int coordinates_count) {
		geo_coords_.reserve(coordinates_count);
	}

	void MapRenderer::RenderMap(std::ostream& out) const {
		doc_.Render(out);
	}

	void MapRenderer::AddGeoCoords(const geo::Coordinates& coordinates) {
		geo_coords_.push_back(coordinates);
	}

	void MapRenderer::AddRoute(const domain::RouteData* route) {
		routes_.insert(route);
	}

	void MapRenderer::SetPolylineSettings(svg::Polyline& polyline, size_t color_id) {
		polyline.SetFillColor("none");
		polyline.SetStrokeColor(render_settings_.color_palette.at(color_id));
		polyline.SetStrokeWidth(render_settings_.stroke_width);
		polyline.SetStrokeLineCap(render_settings_.stroke_linecap);
		polyline.SetStrokeLineJoin(render_settings_.stroke_linejoin);
	}

	void MapRenderer::SetUnderlayerRouteTextSettings(svg::Text& text, const domain::RouteData* route_data, const geo::SphereProjector& proj) {
		using namespace std::literals;
		text =
			svg::Text()
			.SetFillColor(render_settings_.underlayer_stroke_color)
			.SetStrokeColor(render_settings_.underlayer_stroke_color)
			.SetStrokeWidth(render_settings_.underlayer_stroke_width)
			.SetStrokeLineCap(render_settings_.stroke_linecap)
			.SetStrokeLineJoin(render_settings_.stroke_linejoin)
			.SetPosition(proj((*route_data->stops_.begin())->coordinates))
			.SetOffset(render_settings_.route_offset)
			.SetFontSize(render_settings_.route_size)
			.SetFontFamily("Verdana"s)
			.SetFontWeight("bold"s)
			.SetData(route_data->name);
	}

	void MapRenderer::SetMainRouteTextSettings(svg::Text& text, const domain::RouteData* route_data, const geo::SphereProjector& proj, size_t color_id) {
		using namespace std::literals;
		text =
			svg::Text()
			.SetFillColor(render_settings_.color_palette.at(color_id))
			.SetPosition(proj((*route_data->stops_.begin())->coordinates))
			.SetOffset(render_settings_.route_offset)
			.SetFontSize(render_settings_.route_size)
			.SetFontFamily("Verdana"s)
			.SetFontWeight("bold")
			.SetData(route_data->name);
	}

	void MapRenderer::SetUnderlayerStopTextSettings(svg::Text& text, const domain::StopData* stop_data, const geo::SphereProjector& proj) {
		using namespace std::literals;
		text =  // подложка остановки
			svg::Text()
			.SetFillColor(render_settings_.underlayer_stroke_color)
			.SetStrokeColor(render_settings_.underlayer_stroke_color)
			.SetStrokeWidth(render_settings_.underlayer_stroke_width)
			.SetStrokeLineCap(render_settings_.stroke_linecap)
			.SetStrokeLineJoin(render_settings_.stroke_linejoin)
			.SetPosition(proj(stop_data->coordinates))
			.SetOffset(render_settings_.stop_offset)
			.SetFontSize(render_settings_.stop_size)
			.SetFontFamily("Verdana"s)
			.SetData(stop_data->name);
	}

	void MapRenderer::SetMainStopTextSettings(svg::Text& text, const domain::StopData* stop_data, const geo::SphereProjector& proj) {
		using namespace std::literals;
		text =   // остановка
			svg::Text()
			.SetFillColor("black")
			.SetPosition(proj(stop_data->coordinates))
			.SetOffset(render_settings_.stop_offset)
			.SetFontSize(render_settings_.stop_size)
			.SetFontFamily("Verdana"s)
			.SetData(stop_data->name);
	}

	void MapRenderer::PrepareSvgDoc() {
		using namespace std::literals;
		// настройки линии маршрута
		if (geo_coords_.empty()) {
			return;
		}

		// Создаём проектор сферических координат на карту
		const geo::SphereProjector proj{
			geo_coords_.begin(), geo_coords_.end(), render_settings_.width, render_settings_.height, render_settings_.padding
		};

		std::list<svg::Text> texts;
		size_t color_id = 0;
		std::set<const domain::StopData*, domain::PointerStopDataCompareLow> stops_for_svg;
		for (const auto& route_data : routes_) {
			if (route_data->stops_count == 0)
				continue;
			svg::Polyline polyline;
			SetPolylineSettings(polyline, color_id);

			// формирование названий маршрутов
			svg::Text underlayer_text; // подложка маршрута
			SetUnderlayerRouteTextSettings(underlayer_text, route_data, proj);
			texts.push_back(underlayer_text);
			svg::Text main_text; // маршрут
			SetMainRouteTextSettings(main_text, route_data, proj, color_id);
			texts.push_back(main_text); // первая остановка маршута
			color_id = (++color_id == render_settings_.color_palette.size()) ? 0 : color_id;
			for (const auto& stop : route_data->stops_) {
				stops_for_svg.insert(stop); // уникальные остановки для вывода в svg
				const svg::Point screen_coord = proj(stop->coordinates);
				polyline.AddPoint(screen_coord);
			}
			if (route_data->type == domain::route_marks::TO_AND_BACK) {// не кольцевой маршрут
				// Линии
				for (auto it = std::next(route_data->stops_.crbegin(), 1); it != route_data->stops_.rend(); std::advance(it, 1)) {
					const svg::Point screen_coord = proj((*it)->coordinates);
					polyline.AddPoint(screen_coord);
				}
				// Название маршрута на конечной
				if ((*route_data->stops_.cbegin())->name != (*route_data->stops_.crbegin())->name) { // остановки не должны совпадать
					underlayer_text.SetPosition(proj((*route_data->stops_.rbegin())->coordinates));
					texts.push_back(underlayer_text);
					main_text.SetPosition(proj((*route_data->stops_.rbegin())->coordinates));
					texts.push_back(main_text); // конечная
				}
			}
			doc_.Add(polyline);
		}
		// добавляем к списку на рендер названия маршрутов
		for (const auto& text : texts) {
			doc_.Add(text);
		}
		// добавляем к списку на рендер метки остановок
		for (const auto& stop : stops_for_svg) {
			doc_.Add(svg::Circle().SetCenter(proj(stop->coordinates)).SetRadius(render_settings_.stop_radius).SetFillColor("white"));
		}
		// добавляем к списку на рендер названия остановок
		for (const auto& stop_data : stops_for_svg) {
			svg::Text underlayer_text;  // подложка остановки
			SetUnderlayerStopTextSettings(underlayer_text, stop_data, proj);
			svg::Text main_text;   // остановка
			SetMainStopTextSettings(main_text, stop_data, proj);
			doc_.Add(underlayer_text);
			doc_.Add(main_text);
		}
	}

}
