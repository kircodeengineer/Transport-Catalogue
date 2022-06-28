#pragma once
#include "svg.h"
#include "json.h"
#include "domain.h"
#include <vector>
#include <set>
#include <sstream>
#include <list>

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */


namespace map_renderer {
	/// @brief настройки визуализатора маршрутов
	struct RenderSettings {
		double width = 0.0;
		double height = 0.0;
		double padding = 0.0;
		double stroke_width = 0.0;
		svg::StrokeLineCap stroke_linecap = svg::StrokeLineCap::ROUND;
		svg::StrokeLineJoin stroke_linejoin = svg::StrokeLineJoin::ROUND;
		std::vector<svg::Color> color_palette;
		std::string underlayer_stroke_color;
		double underlayer_stroke_width = 0.0;
		svg::Point route_offset;
		int route_size = 0;
		svg::Point stop_offset;
		int stop_size = 0;
		double stop_radius = 0.0;
	};

	/// @brief Визуализатор маршрутов
	class MapRenderer {
	private:
		RenderSettings render_settings_{};
		svg::Document doc_{};
		std::vector<geo::Coordinates> geo_coords_{};
		std::set<const domain::RouteData*, domain::PointerRouteDataCompareLow> routes_{};

	public:
		MapRenderer() = default;

		/// @brief установить настройки визуализатора
		/// @param render_settings настройки 
		void SetRenderSettings(RenderSettings&& render_settings);

		/// @brief визуализировать карту (svg) в поток вывода
		/// @param out поток вывода
		void RenderMap(std::ostream& out) const;

		/// @brief установить размер контейнера координат
		/// @param число координат
		void SetCoordinatesContainerSize(int coordinates_count);

		/// @brief добавить ссылку на координату в транспортном справочнике
		/// @param coordinates координаты
		void AddGeoCoords(const geo::Coordinates& coordinates);

		/// @brief добавить маршрут из БД
		/// @param route маршрут из БД
		void AddRoute(const domain::RouteData* route);

		void SetPolylineSettings(svg::Polyline& polyline, size_t color_id);
		void SetUnderlayerRouteTextSettings(svg::Text& text, const domain::RouteData* route_data, const geo::SphereProjector& proj);
		void SetMainRouteTextSettings(svg::Text& text, const domain::RouteData* route_data, const geo::SphereProjector& proj, size_t color_id);
		void SetUnderlayerStopTextSettings(svg::Text& text, const domain::StopData* stop_data, const geo::SphereProjector& proj);
		void SetMainStopTextSettings(svg::Text& text, const domain::StopData* stop_data, const geo::SphereProjector& proj);

		/// @brief сформировать SVG документ с маршрутами (визуализация)
		void PrepareSvgDoc();

	};
} // namespace map_renderer
