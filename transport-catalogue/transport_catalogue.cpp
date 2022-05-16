#include "transport_catalogue.h"
#include "geo.h"

Stop::Stop(std::string stop_name, double latitude, double longitude)
         : stop_name(std::move(stop_name)), 
           latitude(latitude), 
           longitude(longitude) {
}

Bus::Bus(std::string bus_name, std::vector<const Stop*> routes, RouteType type, uint64_t stops_count, uint64_t unique_stops, double distance_geo, double distance_real)
        : bus_name(std::move(bus_name)), 
          routes(std::move(routes)), 
          type(type), 
          stops_count(stops_count), 
          unique_stops(unique_stops), 
          distance_geo(distance_geo), 
          distance_real(distance_real) {
}

std::size_t DistanceHasher::operator()(std::pair<const Stop *, const Stop *> stop_par) const {
    return (hasher_(stop_par.first) + hasher_(stop_par.second) );
}

void TransportCatalogue::AddRoute(std::string bus_name, std::vector<std::string> bus_routes, char route_type) {
    std::unordered_set<std::string_view> unique_stops;
    std::vector<const Stop*> routes;
    for(const auto stop_name : bus_routes) {
        const Stop* stop = stopname_to_stop.at(stop_name);
        unique_stops.insert(stop->stop_name);
        routes.push_back(stop);
    }
    RouteType type = (route_type == '>') ? RouteType::RING : RouteType::DIRECT;
    double distance_geo = 0.;
    for(uint32_t i = 0; i < routes.size(); ++i){
        if(i + 1 == routes.size()) break;
        distance_geo += ComputeDistance({routes[i]->latitude, routes[i]->longitude}, {routes[i+1]->latitude, routes[i+1]->longitude});
    }
    if (type  == RouteType::DIRECT) {
        distance_geo *= 2;
    }

    double distance_real = 0.;
    for(uint32_t i = 0; i < routes.size(); ++i){
        if(i + 1 == routes.size()) break;
        auto dist = GetRealDistance({stopname_to_stop[routes[i]->stop_name] , stopname_to_stop[routes[i+1]->stop_name]}) != 0 ?
            GetRealDistance({stopname_to_stop[routes[i]->stop_name] , stopname_to_stop[routes[i+1]->stop_name]}) : 
            GetRealDistance({stopname_to_stop[routes[i+1]->stop_name] , stopname_to_stop[routes[i]->stop_name]});
        distance_real += dist;
    }
    //если маршрут прямой, то проходим ещё в обратную сторону
    if (type == RouteType::DIRECT) {
        for(uint32_t i = 0; i < routes.size(); ++i){
            if(i + 1 == routes.size()) break;
            auto dist = GetRealDistance({stopname_to_stop[routes[i+1]->stop_name] , stopname_to_stop[routes[i]->stop_name]}) != 0 ?
                GetRealDistance({stopname_to_stop[routes[i+1]->stop_name] , stopname_to_stop[routes[i]->stop_name]}) : 
                GetRealDistance({stopname_to_stop[routes[i]->stop_name] , stopname_to_stop[routes[i+1]->stop_name]});
            distance_real += dist;
        }
    }

    uint64_t stops_count = (type == RouteType::RING) ? (routes.size()) : (routes.size() * 2 - 1);
    buses.push_back(Bus(bus_name, routes, type, stops_count, unique_stops.size(), distance_geo, distance_real));
    Bus* bus = &buses.back();
    busname_to_bus[bus->bus_name] = bus;
}

void TransportCatalogue::AddStop(std::string stop_name, double latitude, double longitude) {
    stops.push_back(Stop(stop_name, latitude, longitude));
    Stop* stop = &stops.back();
    stopname_to_stop[stop->stop_name] = stop;
}

const Bus* TransportCatalogue::SearchRoute(std::string_view route_name) const {
    if(busname_to_bus.count(route_name))
        return busname_to_bus.at(route_name);
    return nullptr;
}

const Stop* TransportCatalogue::SearchStop(std::string_view stop_name) const {
    if(stopname_to_stop.count(stop_name))
        return stopname_to_stop.at(stop_name);
    return nullptr;
}

std::tuple<std::string, uint64_t, uint64_t, double, double> TransportCatalogue::GetBusRoteInfo(std::string_view route_name) {
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

std::tuple<std::string_view, std::set<std::string_view>> TransportCatalogue::GetStopInfo(std::string_view stop_name) {
    std::set<std::string_view> buses_in_stop;

    if (!SearchStop(stop_name)) {
        buses_in_stop.insert(": not found");
        return {"", buses_in_stop};
    }

    for (const Bus& e : buses) {
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
    real_distance_between_[{stopname_to_stop[from], stopname_to_stop[to]}] = distance;
}

uint64_t TransportCatalogue::GetRealDistance(std::pair<const Stop *, const Stop *> stops_pair) {
    if (real_distance_between_.count(stops_pair)) {
        return real_distance_between_[stops_pair];
    }
    return 0;
}

