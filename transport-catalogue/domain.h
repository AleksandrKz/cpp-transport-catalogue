#pragma once

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */
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

