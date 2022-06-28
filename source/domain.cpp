#include "domain.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области
 * (domain) вашего приложения и не зависят от транспортного справочника. Например Автобусные
 * маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

namespace domain {
	bool PointerStopDataCompare::operator()(const StopData* lhs, const StopData* rhs) const {
		return lhs->name == rhs->name;
	}

	bool PointerStopDataCompareLow::operator()(const StopData* lhs, const StopData* rhs) const {
		return lhs->name < rhs->name;
	}

	size_t PointerStopDataHasher::operator()(const StopData* stop_data) const {
		size_t result = 0;
		size_t multiply = 1;
		for (const auto& letter : stop_data->name) {
			result += static_cast<size_t>(letter - 'A') * multiply;
			multiply *= 100;
		}
		return result;
	}

	size_t RouteDataHasher::operator()(const RouteData& route_data) const {
		size_t result = 0;
		size_t multiply = 1;
		for (const auto& letter : route_data.name) {
			result += static_cast<size_t>(letter - 'A') * multiply;
			multiply *= 100;
		}
		return result;
	}

	bool PointerRouteDataCompareLow::operator()(const RouteData* lhs, const RouteData* rhs) const {
		return lhs->name < rhs->name;
	}

	bool PointerRouteDataCompareEqual::operator()(const RouteData* lhs, const RouteData* rhs) const {
		return lhs->name == rhs->name;
	}

	size_t PointerRouteDataHasher::operator()(const RouteData* route_data) const {
		size_t result = 0;
		size_t multiply = 1;
		static const size_t b = static_cast<size_t>('A');
		for (const auto& letter : route_data->name) {
			size_t a = static_cast<size_t>(letter);
			result += a < b ? a : a - b;
			multiply *= 100;
		}
		return result;
	}
} //namespace domain
