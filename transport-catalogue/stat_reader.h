#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <set>
#include <tuple>

#include "transport_catalogue.h"

namespace output {



    class Reader {
    public:
        std::string_view RemoveSpaces(std::string_view s);
        void ParseBusName(std::string_view str, TransportCatalogue& tc);
        void ParseStopName(std::string_view str, TransportCatalogue& tc);
        void ParseLine(std::string_view str, TransportCatalogue& tc);
        void ReadData(std::istream& input, TransportCatalogue& tc);

        std::vector<std::string>& GetBuses();
        std::vector<std::string>& GetStops();
    private:
        std::vector<std::string> buses_names;
        std::vector<std::string> stop_names;
    };

    void PrintStopInfo(std::string_view name_stop, TransportCatalogue& tc);
    void PrintBusInfo(std::string_view name_bus, TransportCatalogue& tc);
}

