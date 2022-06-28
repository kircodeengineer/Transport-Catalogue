//  класс транспортного справочника
#pragma once
#include "domain.h"
#include "geo.h"
#include <deque>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <string>


/// @brief Транспортный справочник
class TransportCatalogue {
private:
	/// @brief автобусные остановки
	std::unordered_map<std::string_view, domain::StopDataRoutes> stops_;

	/// @brief автобусные маршруты
	std::unordered_map<std::string_view, domain::RouteData> routes_;
public:
	/// @brief Добавление маршрутов в каталог
	/// @param parsedBusReq маршрут 
	/// @return константный указатель на добавленный маршрут
	const domain::RouteData* AddRoute(domain::RouteData&& parsedBusReq);

	/// @brief Добавление остановки в каталог
	/// @param parsedStopReq остановка
	/// /// @return константный указатель на добавленную остановку
	const domain::StopDataRoutes* AddStop(domain::StopDataRoutes&& parsedStopReq);

	/// @brief Задание реального расстояния между остановками A и B
	/// @param A_stop_name первая остановка (
	/// @param B_stop_name вторая остановка
	/// @param length реальное расстояние между остановками
	void AddLengthBetweenStops(std::string_view A_stop_name, std::string_view B_stop_name, int length);

	/// @brief Запрос автобусной остановки по имени
	/// @param stop_name имя автобусной остановки
	/// @return константный указатель на остановку
	const domain::StopDataRoutes* GetStop(std::string_view stop_name) const;

	/// @brief Запрос маршрута по имени
	/// @param route_name имя маршрута
	/// @return константный указатель на маршрут
	const domain::RouteData* GetRoute(std::string_view route_name) const;

	/// @brief Запрос числа остановок
	/// @return число остановок
	size_t GetStopsCount() const;

	/// @brief Запрос всех маршрутов
	/// @return константная ссылка на хранилище маршрутов
	const std::unordered_map<std::string_view, domain::RouteData>& GetAllRoutes() const;

	/// @brief Запрос всех остановок
	/// @return константная ссылка на хранилище остановок
	const std::unordered_map<std::string_view, domain::StopDataRoutes>& GetAllStops() const;
};
