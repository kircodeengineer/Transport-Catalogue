#pragma once

#include "svg.h"
#include <cmath>
#include <iostream>
#include <algorithm>

//функции для работы с географическими координатами
namespace geo {

	/// @brief радиус Земли
	const double R_Z = 6371000.0;
	/// @brief погрешность
	const double EPSILON = 1e-6;
	/// @brief координаты автобусной остановки
	struct Coordinates {
		/// @brief широта
		double lat;
		/// @brief долгота
		double lng;

		bool operator==(const Coordinates& other) const;
		bool operator!=(const Coordinates& other) const;
	};

	double ComputeDistance(Coordinates from, Coordinates to);

	bool IsZero(double value);

	/// @brief проецирование сферы на плоскость
	/// @detail https://practicum.yandex.ru/trainer/cpp/lesson/36fd4e46-1caa-463b-924a-041e82de5640/task/dbc50f59-424c-4a0b-86b2-9c6ffeef96ed/?hideTheory=1
	class SphereProjector {
	private:
		double padding_;
		double min_lon_ = 0;
		double max_lat_ = 0;
		double zoom_coeff_ = 0;
	public:
		// points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
		template <typename PointInputIt>
		SphereProjector(PointInputIt points_begin, PointInputIt points_end,
			double max_width, double max_height, double padding);

		// Проецирует широту и долготу в координаты внутри SVG-изображения
		svg::Point operator()(geo::Coordinates coords) const;
	};

	template <typename PointInputIt>
	SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
		double max_width, double max_height, double padding)
		: padding_(padding) //
	{
		// Если точки поверхности сферы не заданы, вычислять нечего
		if (points_begin == points_end) {
			return;
		}

		// Находим точки с минимальной и максимальной долготой
		const auto [left_it, right_it] = std::minmax_element(
			points_begin, points_end,
			[](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
		min_lon_ = left_it->lng;
		const double max_lon = right_it->lng;

		// Находим точки с минимальной и максимальной широтой
		const auto [bottom_it, top_it] = std::minmax_element(
			points_begin, points_end,
			[](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
		const double min_lat = bottom_it->lat;
		max_lat_ = top_it->lat;

		// Вычисляем коэффициент масштабирования вдоль координаты x
		std::optional<double> width_zoom;
		if (!IsZero(max_lon - min_lon_)) {
			width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
		}

		// Вычисляем коэффициент масштабирования вдоль координаты y
		std::optional<double> height_zoom;
		if (!IsZero(max_lat_ - min_lat)) {
			height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
		}

		if (width_zoom && height_zoom) {
			// Коэффициенты масштабирования по ширине и высоте ненулевые,
			// берём минимальный из них
			zoom_coeff_ = std::min(*width_zoom, *height_zoom);
		}
		else if (width_zoom) {
			// Коэффициент масштабирования по ширине ненулевой, используем его
			zoom_coeff_ = *width_zoom;
		}
		else if (height_zoom) {
			// Коэффициент масштабирования по высоте ненулевой, используем его
			zoom_coeff_ = *height_zoom;
		}
	}
} // namespace geo

std::ostream& operator<<(std::ostream& out, const geo::Coordinates& coords);
