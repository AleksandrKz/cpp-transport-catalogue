#include "domain.h"

Stop::Stop(std::string& stop_name, double latitude, double longitude)
         : stop_name(std::move(stop_name)), 
           latitude(latitude), 
           longitude(longitude) {
}

Bus::Bus(std::string& bus_name, std::vector<const Stop*>& routes, RouteType type, uint64_t stops_count, uint64_t unique_stops, double distance_geo, double distance_real)
        : bus_name(std::move(bus_name)), 
          routes(std::move(routes)), 
          type(type), 
          stops_count(stops_count), 
          unique_stops(unique_stops), 
          distance_geo(distance_geo), 
          distance_real(distance_real) {
}

