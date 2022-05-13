#include "stat_reader.h"

#include <cstdint>
#include <string>
#include <vector>
#include <stack>
#include <iomanip>//setprecision

using namespace output;

std::vector<std::string>& Reader::GetStops() {
    return stop_names;
}

std::vector<std::string>& Reader::GetBuses() {
    return buses_names;
}

std::string_view Reader::RemoveSpaces(std::string_view s){
    auto first_space = s.find_first_not_of(' ');
    auto last_space = s.find_last_not_of(' ');
    if (first_space != last_space)
        return s.substr(first_space, last_space - first_space + 1);
    else if(!s.empty())
    	return s.substr(first_space, first_space+1);
    else
        return s;
}

void Reader::ParseStopName(std::string_view str, TransportCatalogue& tc) {
    str.remove_prefix(4);
    auto begin_pos = str.find_first_not_of(' ');
    auto end_pos = str.find_last_not_of(' ');
    std::string stop = std::string(str.substr(begin_pos, end_pos));
    PrintStopInfo(stop, tc);
}

void Reader::ParseBusName(std::string_view str, TransportCatalogue& tc) {
    str.remove_prefix(3);
    auto begin_pos = str.find_first_not_of(' ');
    auto end_pos = str.find_last_not_of(' ');
    std::string bus = std::string(str.substr(begin_pos, end_pos));
    PrintBusInfo(bus, tc);
}

void Reader::ParseLine(std::string_view str, TransportCatalogue& tc) {
    std::string_view line = RemoveSpaces(str);
    if(line.find("Stop") == 0 ) {
        return ParseStopName(line, tc);
    } else {
        return ParseBusName(line, tc);
    }
}

void Reader::ReadData(std::istream &input, TransportCatalogue& tc) {
    std::string line;
    std::getline(input, line);
    if(!line.empty()) {
        ParseLine(line, tc);
    }

}

void output::PrintStopInfo(std::string_view name_stop, TransportCatalogue& tc) {
    const auto [name, routes] = tc.GetStopInfo(name_stop);
    if (name.empty()) {
        std::cout << "Stop " << name_stop << ": not found" << std::endl;
    } else if (routes.empty()) {
        std::cout << "Stop " << name_stop << ": no buses" << std::endl;
    } else {
        std::cout << "Stop " << name << ": buses ";
        bool space = false;
        for (const auto bus : routes) {
            if (space) {
                std::cout << " ";
            }
            std::cout << bus;
            space = true;
        }
        std::cout << std::endl;
    }
}

void output::PrintBusInfo(std::string_view name_bus, TransportCatalogue& tc) {
    const auto [route_name, stops_count, stops_unique, distance_real, curvature] = tc.GetBusRoteInfo(name_bus);
    if (route_name.empty()) {
        std::cout << "Bus " << name_bus << ": not found" << std::endl;
    } else {
        std::cout << std::setprecision(6);
        std::cout << "Bus " << route_name << ": " << stops_count << " stops on route, " << stops_unique << " unique stops, " << distance_real << " route length, " << curvature << " curvature" << std::endl;
    }
}

