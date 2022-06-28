#include "transport_router.h"
#include "router.h"
#include <stdexcept>

namespace items {
	Bus::Bus(std::string_view bus, int span_count, double time) : bus_(bus), span_count_(span_count), time_(time) {}
	Wait::Wait(std::string_view stop_name, double time) : stop_name_(stop_name), time_(time) {}
}


Item::Item()
	: value_(nullptr) {
}

Item::Item(std::nullptr_t value)
	: value_(std::move(value)) {
}

Item::Item(items::Bus bus)
	: value_(std::move(bus)) {
}

Item::Item(items::Wait wait)
	: value_(std::move(wait)) {
}

const items::Bus& Item::AsBus() const {
	using namespace std::literals;
	if (!IsBus()) {
		throw std::logic_error("items::Bus logic error, odnako\n"s);
	}
	return std::get<items::Bus>(value_);
}

bool Item::IsBus() const {
	return std::holds_alternative<items::Bus>(value_);
}

const items::Wait& Item::AsWait() const {
	using namespace std::literals;
	if (!IsWait()) {
		throw std::logic_error("items::Wait logic error, odnako\n"s);
	}
	return std::get<items::Wait>(value_);
}

bool Item::IsWait() const {
	return std::holds_alternative<items::Wait>(value_);
}

bool TransportRouter::Weight::operator<(const TransportRouter::Weight& other)const {
	return time_ < other.time_;
}

bool TransportRouter::Weight::operator>(const TransportRouter::Weight& other)const {
	return time_ > other.time_;
}

TransportRouter::Weight operator+(const TransportRouter::Weight& lhs, const TransportRouter::Weight& rhs) {
	TransportRouter::Weight result;
	// пересели на новый маршрут
	result.bus_name_ = rhs.bus_name_;
	// маршруту проедем заданное число остановок
	result.stops_count_ = rhs.stops_count_;
	// остановка пересадки
	result.stop_name_ = rhs.stop_name_;
	// суммарное время в пути
	result.time_ = lhs.time_ + rhs.time_;
	return result;
}


TransportRouter::TransportRouter(const TransportCatalogue& transport_catalogue) : transport_catalogue_(transport_catalogue) {}

void TransportRouter::Init(double bus_wait_time, double bus_velocity) {
	bus_wait_time_ = bus_wait_time;
	// переводим скорость из км/ч -> м/мин
	bus_velocity_ = bus_velocity * 1000.0 / 60.0;
	graph_uptr_ = std::make_unique<graph::DirectedWeightedGraph<Weight>>(transport_catalogue_.GetStopsCount());
	id_to_stop_name_.reserve(transport_catalogue_.GetStopsCount());
	auto all_stops = transport_catalogue_.GetAllStops();
	// генерируем id остановок и переводчики к ним
	for (const auto& [stop_name, stop_data] : all_stops) {
		stop_name_to_id_[stop_name] = id_to_stop_name_.size();
		id_to_stop_name_.push_back(stop_name);
	}
	// генерируем рёбра графа
	auto all_routes = transport_catalogue_.GetAllRoutes();
	for (const auto& [route_name, route] : all_routes) {
		auto edges_count_to_add = AddRouteToGraph(route.stops_.begin(), route.stops_.end(), route_name);
		// если маршрут туда и обратно, то нужно её ещё его и в обратном направлении добавить в граф
		if (route.type == domain::route_marks::TO_AND_BACK) {
			AddRouteToGraph(route.stops_.rbegin(), route.stops_.rend(), route_name, edges_count_to_add);
		}
	}
	// передаем в маршрутизатор граф
	router_uptr_ = std::make_unique<graph::Router<Weight>>(*graph_uptr_);
}

std::optional<TransportRouter::Path> TransportRouter::FindPath(const std::string& from, const std::string& to) const {
	if (router_uptr_ == nullptr) {
		throw std::logic_error("ERROR router wasn't initiated");
	}

	auto graph_result = router_uptr_->BuildRoute(stop_name_to_id_.at(from), stop_name_to_id_.at(to));
	if (graph_result == std::nullopt) {
		return std::nullopt;
	}
	TransportRouter::Path return_result;

	return_result.total_time = (*graph_result).weight.time_;

	for (const auto& edge_id : (*graph_result).edges) {
		auto edge = graph_uptr_->GetEdge(edge_id);
		return_result.items.emplace_back(items::Wait{ edge.weight.stop_name_, bus_wait_time_ });
		// -bus_wait_time_ , т.к. мы учли его в ребре графа
		return_result.items.emplace_back(items::Bus{ edge.weight.bus_name_, edge.weight.stops_count_, edge.weight.time_ - bus_wait_time_ });
	}
	return return_result;
}
