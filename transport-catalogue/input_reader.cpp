#include "input_reader.h"

#include <cstdint>
#include <string>
#include <vector>

using namespace input;


std::vector<Stop>& Reader::GetStops() {
    return stops;
}
std::vector<Bus>& Reader::GetRoutes() {
    return buses_routes;
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

void Reader::ParseStop(std::string_view str) {
    //Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino
    str.remove_prefix(4);
    auto begin_pos = str.find_first_not_of(' ');
    auto split_pos = str.find(':');
    auto end_pos = str.find_last_not_of(' ', split_pos - 1);
    std::string stop = std::string(str.substr(begin_pos, end_pos));

    str.remove_prefix(split_pos + 1);
    begin_pos = str.find_first_not_of(' ');
    split_pos = str.find(',');
    end_pos = str.find_last_not_of(' ', split_pos - 1);
    double latitude = std::stod(std::string(str.substr(begin_pos, end_pos)));

    str.remove_prefix(split_pos + 1);
    begin_pos = str.find_first_not_of(' ');
    split_pos = str.find(',');
    end_pos = str.find_last_not_of(' ', split_pos - 1);
    double longitude = std::stod(std::string(str.substr(begin_pos, end_pos)));

    std::vector<std::pair<std::string, uint64_t>> real_distance;
    str.remove_prefix(split_pos + 1);
    while(str.find(" to ") != std::string_view::npos) {
        begin_pos = str.find_first_not_of(' ');
        end_pos = str.find_first_of('m');
        auto dist = std::stoi(std::string(str.substr(begin_pos, end_pos)));
        
        begin_pos = str.find_first_not_of(" to ", end_pos);
        str.remove_prefix(begin_pos + 5);

        begin_pos = str.find_first_not_of(' ');
        split_pos = str.find_first_of(',', begin_pos);
        
        auto name = std::string(str.substr(begin_pos, split_pos));

        real_distance.push_back({name, dist});

        if (split_pos == std::string_view::npos) {
            break;
        } else {
            str.remove_prefix(split_pos + 1);
        }
    }
    stops.push_back(Stop(stop, latitude, longitude, real_distance));
}

void Reader::ParseBusRoute(std::string_view str) {
    //Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam
    //Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka
    char split_symbol = str.find('>') != std::string_view::npos ? '>' : '-';
    std::vector<std::string> stops;
    str.remove_prefix(3);
    auto begin_pos = str.find_first_not_of(' ');
    auto split_pos = str.find(':');
    auto end_pos = str.find_last_not_of(' ', split_pos - 1);
    std::string bus = std::string(str.substr(begin_pos, end_pos));

    str.remove_prefix(split_pos + 1);
    while((split_pos = str.find(split_symbol)) != std::string_view::npos) {
        begin_pos = str.find_first_not_of(' ');
        end_pos = str.find_last_not_of(' ', split_pos - 1);
        stops.push_back(std::string(str.substr(begin_pos, end_pos)));
        str.remove_prefix(split_pos + 1);
    }
    str.remove_prefix(split_pos + 1);
    begin_pos = str.find_first_not_of(' ');
    end_pos = str.find_last_not_of(' ');
    stops.push_back(std::string(str.substr(begin_pos, end_pos)));

    buses_routes.push_back(Bus(bus, stops, split_symbol));
}


void Reader::ParseLine(std::string_view str) {
    std::string_view line = RemoveSpaces(str);
    if(line.find("Stop") == 0) {
        ParseStop(line);
    } else if(line.find("Bus") == 0 && line.find(':') != std::string_view::npos) {
        ParseBusRoute(line);
    }
}

void Reader::ReadData(std::istream& input) {
    uint64_t count_line;
    input >> count_line;
    input.ignore();
    std::string line;
    while (count_line--) {
        std::getline(input, line);
        if(!line.empty())
            ParseLine(line);
    }
}
