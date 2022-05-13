//sprint 9 itog понять и простить)
 
#include <iostream>


#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

using namespace std;

int main() {

    TransportCatalogue catalogue;

    input::Reader data;
    data.ReadData(std::cin);

    for(const auto& e : data.GetStops()) {
        catalogue.AddStop(e.stop_name, e.latitude, e.longitude);
    }

    for(const auto& e : data.GetStops()) {
        for (const auto& [name_stop, dist] : e.real_distance_to_stop) {
            catalogue.AddRealDistance(e.stop_name, name_stop, dist);
        }
    }

    for(const auto& e : data.GetRoutes()) {
        catalogue.AddRoute(e.bus_name, e.routes, e.routes_type);
    }


    output::Reader stat;
    uint64_t count_line;
    std::cin >> count_line;
    std::cin.ignore();
    while (count_line--) {
        stat.ReadData(std::cin, catalogue);
    }
    
    return 0;
}
