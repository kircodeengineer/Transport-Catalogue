#pragma once
#include "geo.h"
#include <string>
#include <deque>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <string>

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */
namespace domain {

	struct RouteData;
	struct PointerStopDataHasher;

	/// @brief автобусная остановка
	struct StopData {
		/// @brief координаы данной остановки
		geo::Coordinates coordinates;
		/// @brief имя остановки
		std::string name;
		/// @brief расстояния до остановок
		//std::unordered_map<const StopData*, int, PointerStopDataHasher> lengths_to_stops;
		std::unordered_map<std::string_view, int> lengths_to_stops;
	};

	/// @brief компаратор для сравнения двух остановок 
	/// @param lhs указатель на остановку
	/// @param rhs указатель на остановку
	/// @return результат сравнения имён остановок
	struct PointerStopDataCompare {
		bool operator()(const StopData* lhs, const StopData* rhs) const;
	};

	/// @brief компаратор для сравнения двух остановок (set)
	/// @param lhs указатель на остановку
	/// @param rhs указатель на остановку
	/// @return результат сравнения имён остановок
	struct PointerStopDataCompareLow {
		bool operator()(const StopData* lhs, const StopData* rhs) const;
	};

	/// @brief хеш-функция для расчёта хеша указателя на структуру остановки StopData
	/// @param stop_data остановка
	/// @return хеш
	struct PointerStopDataHasher {
		size_t operator()(const StopData* stop_data) const;
	};


	/// @brief Маршрут
	struct RouteData {
		/// @brief автобусные остановки
		std::deque<const StopData*> stops_;
		/// @brief уникальные автобусные остановки
		std::unordered_set<const StopData*, PointerStopDataHasher, PointerStopDataCompare> unique_stops;
		/// @brief число уникальных автобусных остановок маршрута
		size_t unique_stops_count;
		/// @brief число автобусных остановок маршрута
		size_t stops_count;
		/// @brief номер маршруты
		std::string name;
		/// @brief длина маршрута
		double length_straight = 0;
		/// @brief длина маршрута
		double length = 0;
		/// @brief кривизна маршрута
		double curvature = 0;
		/// @brief тип маршрута
		char type;

	};

	/// @brief хеш-функция для расчёта хеша указателя на структуру маршрута RouteData
	/// @param route_data маршрут
	/// @return хеш
	struct RouteDataHasher {
		size_t operator()(const RouteData& route_data) const;
	};

	/// @brief компаратор для сравнения двух маршрутов (set)
	/// @param lhs указатель на маршрут
	/// @param rhs указатель на маршрут
	/// @return результат сравнения номеров маршрутов
	struct PointerRouteDataCompareLow {
		bool operator()(const RouteData* lhs, const RouteData* rhs) const;
	};

	/// @brief компаратор для сравнения двух маршрутов (unordered_set)
	/// @param lhs указатель на маршрут
	/// @param rhs указатель на маршрут
	/// @return результат сравнения номеров маршрутов
	struct PointerRouteDataCompareEqual {
		bool operator()(const RouteData* lhs, const RouteData* rhs) const;
	};

	/// @brief хеш-функция для расчёта хеша указателя на структуру маршрута RouteData
	/// @param route_data указатель на маршрут
	/// @return хеш
	struct PointerRouteDataHasher {
		size_t operator()(const RouteData* route_data) const;
	};

	/// @brief автобусная остановка вместе с маршрутами, проходящими через неё
	struct StopDataRoutes : StopData {
		/// @brief координаты и имя автобусной остановки
		//StopData stop_data;
		/// @brief маршруты в которых присутствует данная остановка
		std::set<const RouteData*, PointerRouteDataCompareLow> routes_;
	};

	/// @brief типы команд
	namespace commands {
		// ввод маршрута
		const std::string_view BUS = "Bus";
		// ввод координат остановки
		const std::string_view STOP = "Stop";
		// число символов команд для парсинга
		namespace size {
			const size_t BUS = commands::BUS.size();
			const size_t STOP = commands::STOP.size();
		}
	}

	/// @brief символы определяющие тип маршрута
	namespace route_marks {
		// КОЛЬЦЕВОЙ
		const char CIRCLE = '>';
		// ТУДА-ОБРАТНО
		const char TO_AND_BACK = '-';
	}

} // namespace domain
