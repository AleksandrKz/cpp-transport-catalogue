#pragma once

#include <utility>
#include <vector>
#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <string_view>
#include <algorithm>
#include <tuple>
#include <functional>

#include "domain.h"

using namespace geo;

struct DistanceHasher {
    std::size_t operator()(std::pair<const Stop *, const Stop *> stop_par) const;
    std::hash<const void*> hasher_;
};

class TransportCatalogue {
public:
    void AddRealDistance(std::string_view from, std::string_view to, uint64_t distance);
    void AddRoute(std::string bus_name, std::vector<std::string> bus_routes, bool is_roundtrip );
    void AddStop(std::string stop_name, double latitude, double longitude);
    const Bus* SearchRoute(std::string_view route_name) const;
    const Stop* SearchStop(std::string_view stop_name) const;
    const BusRouteInfo  GetBusRouteInfo(std::string_view route_name) const;
    const StopInfo GetStopInfo(std::string_view stop_name) const;
    uint64_t GetRealDistance(std::pair<const Stop *, const Stop *> stops_pair);

    const std::deque<Bus>& GetBuses() const;

private:
    //дорожное расстояние
    std::unordered_map<std::pair<const Stop *, const Stop *>, uint64_t, DistanceHasher> real_distance_between_;
    //-----------------------------------------------------------------------
    //автобусные остановки
    std::deque<Stop> stops;
    std::unordered_map<std::string_view,const Stop*> stopname_to_stop;
    //------------------------------------------------------------------------

    //-----------------------------------------------------------------------
    //автобусные маршруты
    std::deque<Bus> buses;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus;
    //-----------------------------------------------------------------------
};


