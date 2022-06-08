#include "request_handler.h"

RequestHandler::RequestHandler(const TransportCatalogue& tc, renderer::RendererMap& renderer) : tc_(tc), renderer_(renderer) {
}


void RequestHandler::AddToRequest(Request&& rq) {
    request_.push_back(rq);
}

void RequestHandler::PrintStat() {
    std::cout << "[\n";
    bool space = false;
    for (const Request& req : request_) {
        if (space) {
            std::cout << ',';
        }
        space = true;
        if (req.type == "Bus")
            ParseRequestBus(req);
        else if(req.type == "Stop")
            ParseRequestStop(req);
        else if (req.type == "Map")
            ParseRequestMap(req);
    }
    std::cout << "\n]";
}

void RequestHandler::ParseRequestBus(const Request& rq) const {
    const auto [route_name, stops_count, stops_unique, distance_real, curvature] = tc_.GetBusRouteInfo(rq.name);
    std::cout << "\t{\n";
    std::cout << "\t\t\"request_id\" : " << rq.request_id << ',' << std::endl;
    if (route_name.empty()) {
        std::cout << "\t\t\"error_message\" : " << "\"not found\"" << std::endl;
    }
    else {
        //out << std::setprecision(6);
        std::cout << "\t\t\"curvature\" : " << curvature << ',' << std::endl
            << "\t\t\"unique_stop_count\" : " << stops_unique << ',' << std::endl
            << "\t\t\"stop_count\" : " << stops_count << ',' << std::endl
            << "\t\t\"route_length\" : " << distance_real << std::endl;
    }
    std::cout << "\n\t}";
}
void RequestHandler::ParseRequestStop(const Request& rq) const {
    const auto [name, routes] = tc_.GetStopInfo(rq.name);
    std::cout << "\t{\n";
    std::cout << "\t\t\"request_id\" : " << rq.request_id << ',' << std::endl;
    if (name.empty()) {
        std::cout << "\t\t\"error_message\" : " << "\"not found\"" << std::endl;
    }
    else if (routes.empty()) {
        std::cout << "\t\t\"buses\" : " << "[]" << std::endl;
    }
    else {
        std::cout << "\t\t\"buses\" : " << "[";
        bool space = false;
        for (const auto bus : routes) {
            if (space) {
                std::cout << ", ";
            }
            std::cout << "\"" << bus << "\"";
            space = true;
        }
        std::cout << "]" << std::endl;
    }
    std::cout << "\n\t}" << std::endl;
}

void RequestHandler::ParseRequestMap(const Request& rq) const {
    
    json::Document doc = renderer_.GetMap(tc_.GetBuses());

    std::cout << "\t{\n";
    std::cout << "\t\t\"request_id\" : " << rq.request_id << ',' << std::endl;
    std::cout << "\t\t\"map\" : ";
    json::Print(doc, std::cout);
    std::cout << "\n\t}" << std::endl;
}