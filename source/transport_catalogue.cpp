#include "transport_catalogue.h"




const domain::RouteData* TransportCatalogue::AddRoute(domain::RouteData&& parsedBusReq) {
	domain::RouteData& route = routes_[parsedBusReq.name];
	route = std::move(parsedBusReq);
	for (auto& stop : route.unique_stops) {
		stops_[stop->name].routes_.insert(&route);
	}
	return &route;
}

const domain::StopDataRoutes* TransportCatalogue::AddStop(domain::StopDataRoutes&& parsedStopReq) {
	//stops_[parsedStopReq.name] = std::move(parsedStopReq);
	auto result = stops_.insert({ parsedStopReq.name, std::move(parsedStopReq) });
	return &(result.first->second);
}

void TransportCatalogue::AddLengthBetweenStops(std::string_view A_stop_name,
	std::string_view B_stop_name,
	int length) {
	// Задание расстояния от А до B
	auto& A_stop = stops_[A_stop_name];
	auto& B_stop = stops_[B_stop_name];
	A_stop.lengths_to_stops[B_stop.name] = length;

	// Задание расстояния от B до A (если расстояние задано, значит его перезаписывать не надо)

	if (B_stop.lengths_to_stops.find(A_stop.name) == B_stop.lengths_to_stops.end()) {
		B_stop.lengths_to_stops[A_stop.name] = length;
	}
}

const domain::StopDataRoutes* TransportCatalogue::GetStop(std::string_view stop_name) const {
	if (stops_.find(stop_name) != stops_.end()) {
		return &stops_.at(stop_name);
	}
	return nullptr;
}

const domain::RouteData* TransportCatalogue::GetRoute(std::string_view route_name) const {
	if (routes_.find(route_name) != routes_.end()) {
		return &routes_.at(route_name);
	}
	return nullptr;
}

size_t TransportCatalogue::GetStopsCount() const {
	return stops_.size();
}

const std::unordered_map<std::string_view, domain::RouteData>& TransportCatalogue::GetAllRoutes() const {
	return routes_;
}

const std::unordered_map<std::string_view, domain::StopDataRoutes>& TransportCatalogue::GetAllStops() const {
	return stops_;
}
