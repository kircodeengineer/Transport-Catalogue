#pragma once
#include "graph.h"
#include "router.h"
#include "domain.h"
#include "transport_catalogue.h" // переход на request
#include <set>
#include <string>
#include <variant>
#include <vector>
#include <optional>

namespace items {
	namespace  types {
		using namespace std::literals;
		static const std::string bus = "Bus"s;
		static const std::string wait = "Wait"s;
	}

	/// @brief Структура Bus для вывода на печать в json-документ
	struct Bus {
		const std::string_view type_ = types::bus;
		std::string_view bus_;
		int span_count_;
		double time_;
		explicit Bus(std::string_view bus, int span_count, double time);
	};

	/// @brief Структура Wait для вывода на печать в json-документ
	struct Wait {
		const std::string_view type_ = types::wait;
		std::string_view stop_name_;
		double time_;
		explicit Wait(std::string_view stop_name, double time);
	};
}

class Item final : private std::variant<std::nullptr_t, items::Bus, items::Wait>
{
public:
	using variant::variant;
	const variant& GetValue() const { return value_; }

	Item();
	Item(std::nullptr_t value);
	Item(items::Bus bus);
	Item(items::Wait wait);

	const items::Bus& AsBus() const;
	const items::Wait& AsWait() const;

	bool IsBus() const;
	bool IsWait() const;
private:
	variant value_;
};

/// @brief Генератор пути по автобусным маршрутам
class TransportRouter {
public:
	// вес ребра графа
	struct Weight {
		// время в пути
		double time_ = 0.0;
		// название маршрута
		std::string_view bus_name_ = "kek";
		// название остановки отправления
		std::string_view stop_name_ = "kek";
		// число посещенных остановок
		int stops_count_ = 1;

		bool operator<(const Weight& other) const;

		bool operator>(const Weight& other) const;
	};
private:
	// трнаспортный каталог
	const TransportCatalogue& transport_catalogue_;
	// время ожидания автобуса 
	double bus_wait_time_ = 0.0;
	// скорость автобуса
	double bus_velocity_ = 0.0;
	// перевод id остановки в имя
	std::vector<std::string_view> id_to_stop_name_;
	// перевод имени остановки в её id
	std::unordered_map<std::string_view, size_t> stop_name_to_id_;
	// умный указатель на граф с маршрутами
	std::unique_ptr<graph::DirectedWeightedGraph<Weight>> graph_uptr_;
	// умный указатель на маршрутизатор
	std::unique_ptr<graph::Router<Weight>> router_uptr_;

public:
	TransportRouter(const TransportCatalogue& transport_catalogue);
	struct Path {
		std::vector<Item> items;
		double total_time = 0.0;
	};

	/// @brief Инициализация класса
	/// @param bus_wait_time время ожидания автобуса
	/// @param bus_velocity скорость автобуса
	void Init(double bus_wait_time, double bus_velocity);

	/// @brief Поиск пути в графе
	/// @param from название остановки отправления
	/// @param to название остановки прибытия
	/// @return найденный пути
	std::optional<Path> FindPath(const std::string& from, const std::string& to) const;

	/// @brief добавление маршрута в граф поиска пути
	/// @tparam Iterator тип итератора
	/// @param begin начало интервала
	/// @param end конец интервала
	/// @return количество рёбер, которое нужно добавить, если идём в обратном пути
	template<typename Iterator>
	size_t AddRouteToGraph(Iterator begin, Iterator end, std::string_view bus_name, size_t edges_count_to_add = 0);
};

TransportRouter::Weight operator+(const TransportRouter::Weight& lhs, const TransportRouter::Weight& rhs);

template<typename Iterator>
size_t TransportRouter::AddRouteToGraph(Iterator begin, Iterator end, std::string_view bus_name, size_t edges_count_to_add) {
	// Обработка текущего маршрута
	auto it_prev = begin;
	// id самой первой остановки
	size_t id_prev = stop_name_to_id_[(*it_prev)->name];
	auto it_cur = std::next(begin, 1);
	// id текущей остановки
	size_t id_cur = stop_name_to_id_[(*it_cur)->name];
	for (; it_cur != end; std::advance(it_cur, 1), std::advance(it_prev, 1)) {
		id_prev = stop_name_to_id_[(*it_prev)->name];
		id_cur = stop_name_to_id_[(*it_cur)->name];
		auto length = (*it_prev)->lengths_to_stops.at((*it_cur)->name);
		auto time = static_cast<double>(length) / (bus_velocity_);
		Weight weight{ time + bus_wait_time_, bus_name ,(*it_prev)->name };
		auto edge_id = graph_uptr_->AddEdge({ id_prev, id_cur, weight });
		// количество рёбер графа, которые нужно добавить
		auto edges_count_to_update_cur = edges_count_to_add;
		while (edges_count_to_update_cur) {
			--edge_id;
			auto edge = graph_uptr_->GetEdge(edge_id);
			weight = edge.weight;
			weight.stops_count_ += 1;
			weight.time_ += time;
			graph_uptr_->AddEdge({ edge.from, id_cur, weight });
			--edges_count_to_update_cur;
		}
		++edges_count_to_add;
	}
	return edges_count_to_add;
}
