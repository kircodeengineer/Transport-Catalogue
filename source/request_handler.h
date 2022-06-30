#pragma once
/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "domain.h"
#include <string>

namespace map_renderer {
	class MapRenderer;
	struct RenderSettings;
}

namespace request_handler {
	/// @brief Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а с другими подсистемами приложения.
	/// @details Обработчик запросов к базе, содержащего логику, которую не хотелось бы помещать ни в transport_catalogue, ни в json reader
	/// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
	class RequestHandler {
	public:
		RequestHandler();

		/// @brief Возвращает информация о маршруте
		/// @param имя маршрута
		/// @return информация о маршруте
		std::optional<const domain::RouteData*> GetBusStat(std::string_view bus_name) const;

		/// @brief Возвращает информация об остановке
		/// @param имя остановки
		/// @return информация об остановке
		std::optional<const domain::StopDataRoutes*> GetStopStat(std::string_view stop_name) const;

		/// @brief Возвращает маршруты, проходящие через остановку
		/// @param имя остановки
		/// @return set маршрутов
		std::optional<const std::set<const domain::RouteData*, domain::PointerRouteDataCompareLow>*> GetBusesByStop(std::string_view stop_name) const;

		/// @brief Добавить остановку в трансопртный справочник
		/// @param информация об остановке
		void AddStopInTransportCatalogue(domain::StopDataRoutes&& stop_data);

		/// @brief Добавить в трансопртный справочник расстояние между остановками
		/// @param stop_a откуда
		/// @param stop_b куда
		/// @return distance расстояние
		void AddLengthBetweenStops(std::string_view stop_a, std::string_view stop_b, int distance);

		/// @brief Добавить маршрут в трансопртный справочник
		/// @param инфомрация о маршруте
		void AddRouteInTransportCatalogue(domain::RouteData&& route_data);

		/// @brief Инициализация генератора путей
		/// @param bus_wait_time время ожидания автобуса
		/// @param bus_velocity скорость автобуса
		void InitTransportRouter(double bus_wait_time, double bus_velocity);

		/// @brief Поиск пути в графе
		/// @param from название остановки отправления
		/// @param to название остановки прибытия
		/// @return найденный пути
		std::optional<TransportRouter::Path> FindPath(const std::string& from, const std::string& to) const;

		/// @brief установить настройки визуализатора
		/// @param render_settings настройки
		void SetRenderSettings(map_renderer::RenderSettings&& render_settings);

		/// @brief сформировать SVG документ с маршрутами (визуализация)
		void PrepareSvgDoc();

		/// @brief рендер SVG с картой маршрутов
		/// @return SVG строка с картой маршрутов
		std::string RenderMap() const;
	private:
		// RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
		// транспортный каталог
		TransportCatalogue transport_catalogue_;
		// генератор путей между атобусными остановками
		TransportRouter transport_router_;
		// генератор SVG карты
		map_renderer::MapRenderer map_renderer_;
	};
}//namespace request_handler
