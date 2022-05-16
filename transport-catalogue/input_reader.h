#pragma once

#include <istream>
#include <vector>
#include <string>
#include <utility>
#include <string_view>
#include <tuple>
#include <unordered_map>

#include "transport_catalogue.h"

namespace input {
    struct Stop {
        Stop(std::string stop_name, double latitude, double longitude, std::vector<std::pair<std::string, uint64_t>> real_distance_to_stop);
        std::string stop_name = "";
        double latitude = 0;
        double longitude = 0;
        std::vector<std::pair<std::string, uint64_t>> real_distance_to_stop;
    };

    struct Bus {
        Bus(std::string bus_name, std::vector<std::string> routes, char routes_type);
        std::string bus_name = "";
        std::vector<std::string> routes;
        char routes_type;
    };


    class Reader {
    public:
        std::string_view RemoveSpaces(std::string_view s);
        void ParseStop(std::string_view str);
        void ParseBusRoute(std::string_view str);
        void ParseLine(std::string_view str);
        void ReadData(std::istream& input, TransportCatalogue& tc);

        std::vector<Stop>& GetStops();
        std::vector<Bus>& GetRoutes();
        std::vector<std::string>& GetBuses();
        std::vector<std::pair<std::string, uint64_t>>& GetRealDistance();
    private:
        std::vector<Stop> stops;
        std::vector<Bus> buses_routes;
        std::vector<std::string> buses_names;
    };
}


