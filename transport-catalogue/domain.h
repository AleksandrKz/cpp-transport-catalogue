#pragma once

#include <string_view>
#include <string>
#include <vector>
#include <set>

#include "geo.h"

enum class RouteType {
    DIRECT,
    RING
};

struct StopInfo {
    std::string_view stop_name;
    std::set<std::string_view> buses_in_stop;
};

struct BusRouteInfo {
    std::string name;
    uint64_t stops_count;
    uint64_t stops_unique;
    double distance_real;
    double curvature;
};

struct Stop {
    Stop(std::string& stop_name, double latitude, double longitude);
    std::string stop_name = "";
    double latitude = 0;
    double longitude = 0;
};

struct Bus {
    Bus(std::string& bus_name, std::vector<const Stop*>& routes, RouteType type, uint64_t stops_count, uint64_t unique_stops, double distance_geo, double distance_real);
    std::string bus_name = "";
    std::vector<const Stop*> routes;
    RouteType type;
    uint64_t stops_count = 0;
    uint64_t unique_stops = 0;
    double distance_geo = 0.;
    double distance_real = 0.;
};

