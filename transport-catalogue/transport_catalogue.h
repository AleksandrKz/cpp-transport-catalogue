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

enum class RouteType {
    DIRECT,
    RING
};

struct Stop {
    Stop(std::string stop_name, double latitude, double longitude);
    std::string stop_name = "";
    double latitude = 0;
    double longitude = 0;
};

struct Bus {
    Bus(std::string bus_name, std::vector<const Stop*> routes, RouteType type, uint64_t stops_count, uint64_t unique_stops, double distance_geo, double distance_real);
    std::string bus_name = "";
    std::vector<const Stop*> routes;
    RouteType type;
    uint64_t stops_count = 0;
    uint64_t unique_stops = 0;
    double distance_geo = 0.;
    double distance_real = 0.;
};

struct DistanceHasher {
    std::size_t operator()(std::pair<const Stop *, const Stop *> stop_par) const;
    std::hash<const void*> hasher_;
};

class TransportCatalogue {
public:
    void AddRealDistance(std::string_view from, std::string_view to, uint64_t distance);
    void AddRoute(std::string bus_name, std::vector<std::string> bus_routes, char route_type);
    void AddStop(std::string stop_name, double latitude, double longitude);
    const Bus* SearchRoute(std::string_view route_name) const;
    const Stop* SearchStop(std::string_view stop_name) const;
    std::tuple<std::string, uint64_t, uint64_t, double, double>  GetBusRoteInfo(std::string_view route_name);
    std::tuple<std::string_view, std::set<std::string_view>> GetStopInfo(std::string_view stop_name);
    uint64_t GetRealDistance(std::pair<const Stop *, const Stop *> stops_pair);

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


