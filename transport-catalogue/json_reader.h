#pragma once

#include <sstream>
#include <iostream>

#include <cstdint>
#include <string>
#include <vector>
#include <utility>

#include "transport_catalogue.h"
#include "transport_router.h"
#include "request_handler.h"
#include "json.h"
#include "map_renderer.h"

using namespace json;

namespace ReaderJson {

struct Stop {
    Stop(std::string stop_name, double latitude, double longitude, std::vector<std::pair<std::string, uint64_t>> real_distance_to_stop);
    std::string stop_name = "";
    double latitude = 0;
    double longitude = 0;
    std::vector<std::pair<std::string, uint64_t>> real_distance_to_stop;
};

struct Bus {
    Bus(std::string bus_name, std::vector<std::string> routes, bool is_roundtrip);
    std::string bus_name = "";
    std::vector<std::string> routes;
    bool is_roundtrip;
};

class Reader {
public:
    explicit Reader(std::istream& input);

    void ReadData(std::istream& input);

    void FillCatalogue(TransportCatalogue& tc);
    void FillRequest(RequestHandler& rq);

    std::vector<Bus> GetBuses();

    RoutingSettings GetRoutingSettings() const;
    const renderer::RenderSettings& GetRendererSettings() const;
    SerializationSettings GetSerializationSettings() const;

private:
    //для каталога
    void ParseStop(const Node& n);
    void ParseBusRoute(const Node& n);

    //для статистики
    void ParseStatRequest(const Node& n);

    //для map renderer
    void ParseRendererSettigs(const Node& n);

    //для routing settings
    void ParseRoutingSettings(const Node& n);

    //для serialization settings
    void ParseSerializationSettings(const Node& n);

    //для каталога
    std::vector<Stop> stops_;
    std::vector<Bus> buses_routes_;

    //запросы
    std::vector<Request> request_;

    //настройки рендера
    renderer::RenderSettings render_settings_;

    //настройки routing_settings
    RoutingSettings routing_settings_;

    //настройки SerializationSettings
    SerializationSettings serialization_settings_;
};

} // namespace Reader
