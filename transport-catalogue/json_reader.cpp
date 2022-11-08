#include "json_reader.h"

namespace ReaderJson {

Stop::Stop(std::string stop_name, double latitude, double longitude, std::vector<std::pair<std::string, uint64_t>> real_distance_to_stop) : 
    stop_name(std::move(stop_name)),
    latitude(latitude), 
    longitude(longitude), 
    real_distance_to_stop(std::move(real_distance_to_stop)) {
}

Bus::Bus(std::string bus_name, std::vector<std::string> routes, bool is_roundtrip) : 
    bus_name(std::move(bus_name)), 
    routes(std::move(routes)), 
    is_roundtrip(is_roundtrip) {
}

Reader::Reader(TransportCatalogue& tc, TransportRouter& tr, renderer::RendererMap& rm, RequestHandler& rh) : tc_(tc), tr_(tr), rm_(rm), rh_(rh) {
}

RoutingSettings Reader::GetRoutingSettings() const {
    return routing_settings_;
}

void Reader::ParseRoutingSettings(const Node& n) {
    routing_settings_.bus_velocity = n.AsDict().at("bus_velocity").AsDouble();
    routing_settings_.bus_wait_time = n.AsDict().at("bus_wait_time").AsInt();
}

std::vector<ReaderJson::Bus> Reader::GetBuses() {
    return buses_routes_;
}

void Reader::ParseRendererSettigs(const Node& n) {
    renderer::RenderSettings rs;
    
    rs.width = n.AsDict().at("width").AsDouble();
    rs.height = n.AsDict().at("height").AsDouble();
    rs.padding = n.AsDict().at("padding").AsDouble();
    rs.line_width = n.AsDict().at("line_width").AsDouble();
    rs.stop_radius = n.AsDict().at("stop_radius").AsDouble();
    rs.bus_label_font_size = n.AsDict().at("bus_label_font_size").AsInt();
    rs.bus_label_offset.x = n.AsDict().at("bus_label_offset").AsArray()[0].AsDouble();
    rs.bus_label_offset.y = n.AsDict().at("bus_label_offset").AsArray()[1].AsDouble();
    rs.stop_label_font_size = n.AsDict().at("stop_label_font_size").AsInt();
    rs.stop_label_offset.x = n.AsDict().at("stop_label_offset").AsArray()[0].AsDouble();
    rs.stop_label_offset.y = n.AsDict().at("stop_label_offset").AsArray()[1].AsDouble();
    if (n.AsDict().at("underlayer_color").IsString()) {
        rs.underlayer_color = n.AsDict().at("underlayer_color").AsString();
    }
    else if (n.AsDict().at("underlayer_color").IsArray()) {
        if (n.AsDict().at("underlayer_color").AsArray().size() == 3) {
            uint8_t r = n.AsDict().at("underlayer_color").AsArray()[0].AsInt();
            uint8_t g = n.AsDict().at("underlayer_color").AsArray()[1].AsInt();
            uint8_t b = n.AsDict().at("underlayer_color").AsArray()[2].AsInt();
            rs.underlayer_color = svg::Color(svg::Rgb(r, g, b));
        }
        else {
            uint8_t r = n.AsDict().at("underlayer_color").AsArray()[0].AsInt();
            uint8_t g = n.AsDict().at("underlayer_color").AsArray()[1].AsInt();
            uint8_t b = n.AsDict().at("underlayer_color").AsArray()[2].AsInt();
            double a = n.AsDict().at("underlayer_color").AsArray()[3].AsDouble();
            rs.underlayer_color = svg::Color(svg::Rgba(r, g, b, a));
        }
    }

    rs.underlayer_width = n.AsDict().at("underlayer_width").AsDouble();

    for (const auto& col : n.AsDict().at("color_palette").AsArray()) {
        if (col.IsString()) {
            rs.color_palette.push_back(svg::Color(col.AsString()));
        }
        else if (col.IsArray()) {
            if (col.AsArray().size() == 3) {
                uint8_t r = col.AsArray()[0].AsInt();
                uint8_t g = col.AsArray()[1].AsInt();
                uint8_t b = col.AsArray()[2].AsInt();
                rs.color_palette.push_back(svg::Color(svg::Rgb(r, g, b)));
            }
            else {
                uint8_t r = col.AsArray()[0].AsInt();
                uint8_t g = col.AsArray()[1].AsInt();
                uint8_t b = col.AsArray()[2].AsInt();
                double a = col.AsArray()[3].AsDouble();
                rs.color_palette.push_back(svg::Color(svg::Rgba(r, g, b, a)));
            }
        }
    }

    rm_.SetRendererSettings(rs);
}

void Reader::ParseStatRequest(const Node& n) {
    for (const auto& node : n.AsArray()) {
        if (node.AsDict().at("type").AsString() == "Stop") {
            rh_.AddToRequest({ "Stop", node.AsDict().at("name").AsString(), node.AsDict().at("id").AsInt(), {} });
        } else if (node.AsDict().at("type").AsString() == "Bus") {
            rh_.AddToRequest({ "Bus", node.AsDict().at("name").AsString(), node.AsDict().at("id").AsInt(), {} });
        } else if (node.AsDict().at("type").AsString() == "Map") {
            rh_.AddToRequest({ "Map", "", node.AsDict().at("id").AsInt(), {} });
        } else if (node.AsDict().at("type").AsString() == "Route")
            rh_.AddToRequest({ "Route", "", node.AsDict().at("id").AsInt(), {node.AsDict().at("from").AsString(), node.AsDict().at("to").AsString()}});
    }
}

void Reader::ParseStop(const Node& n) {
    std::string stop_name = n.AsDict().at("name").AsString();
    double latitude = n.AsDict().at("latitude").AsDouble();
    double longitude = n.AsDict().at("longitude").AsDouble();
    std::vector<std::pair<std::string, uint64_t>> real_distance;
    for (const auto& [name_stop, road_dist] : n.AsDict().at("road_distances").AsDict()) {
        real_distance.push_back({name_stop, road_dist.AsInt()});
    }
    stops_.push_back(ReaderJson::Stop(stop_name, latitude, longitude, real_distance));
}

void Reader::ParseBusRoute(const Node& n) {
    std::string bus_rotute = n.AsDict().at("name").AsString();;
    bool is_roundtrip = n.AsDict().at("is_roundtrip").AsBool();
    std::vector<std::string> stops;
    for (const auto& stop : n.AsDict().at("stops").AsArray()) {
        stops.push_back(stop.AsString());
    }
    buses_routes_.push_back(Bus(bus_rotute, stops, is_roundtrip));
}

void Reader::ReadData(std::istream& input) {
    Document file_json = Load(input);
    Node data = file_json.GetRoot();
    Node base_requests;
    Node stat_requests;
    Node render_settings;
    Node routing_settings;
    for (const auto& [key, node] : data.AsDict()) {
        if (key == "base_requests") {
            base_requests = node;
        }
        else if (key == "render_settings") {
            render_settings = node;
        }
        else if (key == "stat_requests") {
            stat_requests = node;
        }
        else if (key == "routing_settings")
        {
            routing_settings = node;
        }
        else
        {
            continue;
        }
    }

    //собираем данные 
    for (const auto& node : base_requests.AsArray()) {
        if (node.AsDict().at("type").AsString() == "Stop") {
            ParseStop(node);
        } else if (node.AsDict().at("type").AsString() == "Bus") {
            ParseBusRoute(node);
        }
    }

    // вбиваем данные в каталог
    //вбиваем инфу по остановкам
    for(const auto& e : stops_) {
        tc_.AddStop(e.stop_name, e.latitude, e.longitude);
    }
    //добавляем инфу по дорожному расстоянию меж остановками
    for(const auto& e : stops_) {
        for (const auto& [name_stop, dist] : e.real_distance_to_stop) {
            tc_.AddRealDistance(e.stop_name, name_stop, dist);
        }
    }
    //вбиваем информацию  по маршрутам
    for(const auto& e : buses_routes_) {
        tc_.AddRoute(e.bus_name, e.routes, e.is_roundtrip);
    }

    //настройки для рендера
    ParseRendererSettigs(render_settings);

    //статистика
    ParseStatRequest(stat_requests);

    //читаем настройки роутинга и устанавливаем, строим граф
    ParseRoutingSettings(routing_settings);
    tr_.SetSettings(routing_settings_);
    tr_.BuildGraph();
}

} // namespace Reader
