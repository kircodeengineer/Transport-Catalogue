#pragma once

#include "json.h"
#include "json_builder.h"
#include "request_handler.h"
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

 /// @brief класс для 1. наполнения транспортного справочника данными из JSON;
 /// 2. обработки запросов к базе
 /// 3. формирования массива ответов в формате JSON
class JsonReader {
private:
	// json-документ с запросами
	const json::Document& doc_;

	// обрботчик запросов к транспортному каталогу и генератору карт
	request_handler::RequestHandler request_handler;
public:
	JsonReader(const json::Document& doc);

	/// @brief Обработка json-документа
	/// @return json-документ для печати
	json::Document ProcessJsonDoc();

private:
	/// @brief Обработка запросов на ввод информации
	void ProcessInputRequests();

	/// @brief Обработка запросов на вывод информации в транспортный справочник
	void ProcessTransportCatalogueInputData();

	/// @brief Обработка запросов на вывод информации в генератор SVG карт
	void ProcessMapRendererInputData();

	/// @brief Обработка json-документа с запросами на добавление остановок
	/// @param номер запроса на ввод маршрута в транспортный справочник
	/// @param номер запроса на ввод остановки в транспортный справочник
	void ProcessInputStopRequest(std::list<size_t>& route_requests_ids, std::list<size_t>& stop_requests_ids);

	/// @brief Обработка запросов на добавление расстояния между остановками
	/// @param нмоера запросов
	void ProcessInputStopsLengthsRequest(const std::list<size_t>& stop_requests_ids);

	/// @brief Обработка запросов на добавление маршрутов
	/// @param нмоера запросов
	void ProcessInputRouteRequest(const std::list<size_t>& route_requests_ids);

	/// @brief Чтение параметров настройки маршрутизатора
	void ProcessRoutingSettings();

	/// @brief Обработка запросов на добавление параметров настройки генератора SVG карт
	/// @param настройки генератора SVG карт
	map_renderer::RenderSettings ProcessRenderSettings();

	/// @brief Обработка запросов на вывод информации
	/// @return json-документ для печати
	json::Document ProcessStatRequests();

	/// @brief Обработка запроса на вывод информации о маршруте
	/// @param Заполняемая нода (блок) json-докмента
	/// @param Запрос
	void ProcessBusStatRequest(json::Builder& node, const json::Dict& request);

	/// @brief Обработка запроса на вывод информации об остановке
	/// @param Заполняемая нода (блок) json-докмента
	/// @param Запрос
	void ProcessStopStatRequest(json::Builder& node, const json::Dict& request);

	/// @brief Обработка запроса на вывод карты маршрутов
	/// @param Заполняемая нода (блок) json-докмента
	/// @param Запрос
	void ProcessMapStatRequest(json::Builder& node, const json::Dict& request);

	/// @brief Обработка запроса на вывод найденного пути между остановками
	/// @param Заполняемая нода (блок) json-докмента
	/// @param Запрос
	void ProcessRouteStatRequest(json::Builder& node, const json::Dict& request);
};
