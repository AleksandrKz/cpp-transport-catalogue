#include "transport_catalogue.h"

std::size_t DistanceHasher::operator()(std::pair<const Stop *, const Stop *> stop_par) const {
    return (hasher_(stop_par.first) + hasher_(stop_par.second) );
}

void TransportCatalogue::AddRoute(std::string bus_name, std::vector<std::string> bus_routes, bool is_roundtrip) {
    std::unordered_set<std::string_view> unique_stops;
    std::vector<const Stop*> routes;
    for(const auto& stop_name : bus_routes) {
        const Stop* stop = stopname_to_stop_.at(stop_name);
        unique_stops.insert(stop->stop_name);
        routes.push_back(stop);
    }
    RouteType type = (is_roundtrip) ? RouteType::RING : RouteType::DIRECT;
    double distance_geo = 0.;
    for(size_t i = 0; i < routes.size(); ++i){
        if(i + 1 == routes.size()) break;
        distance_geo += ComputeDistance({routes[i]->latitude, routes[i]->longitude}, {routes[i+1]->latitude, routes[i+1]->longitude});
    }
    if (type  == RouteType::DIRECT) {
        distance_geo *= 2;
    }

    double distance_real = 0.;
    for(size_t i = 0; i < routes.size(); ++i){
        if(i + 1 == routes.size()) break;
        auto dist = GetRealDistance(stopname_to_stop_[routes[i]->stop_name], stopname_to_stop_[routes[i + 1]->stop_name]);
        distance_real += dist;
    }
    //если маршрут прямой, то проходим ещё в обратную сторону
    if (type == RouteType::DIRECT) {
        for(size_t i = 0; i < routes.size(); ++i){
            if(i + 1 == routes.size()) break;
            auto dist = GetRealDistance(stopname_to_stop_[routes[i + 1]->stop_name] , stopname_to_stop_[routes[i]->stop_name]);
            distance_real += dist;
        }
    }

    uint64_t stops_count = (type == RouteType::RING) ? (routes.size()) : (routes.size() * 2 - 1);
    buses_.push_back(Bus(bus_name, routes, type, stops_count, unique_stops.size(), distance_geo, distance_real));
    Bus* bus = &buses_.back();
    busname_to_bus_[bus->bus_name] = bus;
}

void TransportCatalogue::AddStop(std::string stop_name, double latitude, double longitude) {
    stops_.push_back(Stop(stop_name, latitude, longitude));
    Stop* stop = &stops_.back();
    stopname_to_stop_[stop->stop_name] = stop;
}

const Bus* TransportCatalogue::SearchRoute(std::string_view route_name) const {
    if(busname_to_bus_.count(route_name))
        return busname_to_bus_.at(route_name);
    return nullptr;
}

const Stop* TransportCatalogue::SearchStop(std::string_view stop_name) const {
    if(stopname_to_stop_.count(stop_name))
        return stopname_to_stop_.at(stop_name);
    return nullptr;
}

const BusRouteInfo TransportCatalogue::GetBusRouteInfo(std::string_view route_name) const {
    const Bus* route = SearchRoute(route_name);
    if(!route) {
        return {"" , 0 , 0 , 0, 0.};
    }
    std::string name = route->bus_name;
    uint64_t stops_count = route->stops_count;
    uint64_t stops_unique = route->unique_stops;
    double distance_real = route->distance_real;
    double distance_geo = route->distance_geo;
    double curvature = distance_real / distance_geo;
    return { name, stops_count, stops_unique, distance_real, curvature};
}

const StopInfo TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
    std::set<std::string_view> buses_in_stop;

    if (!SearchStop(stop_name)) {
        buses_in_stop.insert(": not found");
        return {"", buses_in_stop};
    }

    for (const Bus& e : buses_) {
        if (std::find_if(e.routes.begin(), e.routes.end(), [stop_name](const Stop* stop){
            if (stop->stop_name == stop_name)
                return true;
            return false;
            }) != e.routes.end()) {
            buses_in_stop.insert(e.bus_name);
        }
    }
    return {stop_name, buses_in_stop};
}

void TransportCatalogue::AddRealDistance(std::string_view from, std::string_view to, uint64_t distance) {
    real_distance_between_[{stopname_to_stop_[from], stopname_to_stop_[to]}] = distance;
}

uint64_t TransportCatalogue::GetRealDistance(const Stop* from, const Stop* to) const {
    if (real_distance_between_.count({from, to})) {
        return real_distance_between_.at({ from, to });
    }
    else if (real_distance_between_.count({ to, from })) {
        return real_distance_between_.at({ to, from });
    }
    else
    return 0;
}

const std::deque<Bus>& TransportCatalogue::GetBuses() const {
    return buses_;
}

const std::unordered_map<std::string_view, const Stop*>& TransportCatalogue::GetAllStops() const {
    return stopname_to_stop_;
}

const std::unordered_map<std::string_view, const Bus*>& TransportCatalogue::GetAllBuses() const {
    return busname_to_bus_;
}

const std::unordered_map<std::pair<const Stop*, const Stop*>, uint64_t, DistanceHasher>& TransportCatalogue::GetAllDistance() const {
    return real_distance_between_;
}