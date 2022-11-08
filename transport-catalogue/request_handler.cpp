#include "request_handler.h"

RequestHandler::RequestHandler(const TransportCatalogue& tc, const TransportRouter& tr, renderer::RendererMap& renderer) : tc_(tc), tr_(tr), renderer_(renderer) {
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
        else if (req.type == "Stop")
            ParseRequestStop(req);
        else if (req.type == "Map")
            ParseRequestMap(req);
        else if (req.type == "Route")
            ParseRequestRouting(req);
    }
    std::cout << "\n]";
}

void RequestHandler::ParseRequestBus(const Request& rq) const {
    using namespace std::literals;
    const auto [route_name, stops_count, stops_unique, distance_real, curvature] = tc_.GetBusRouteInfo(rq.name);
    if (route_name.empty()) {
        json::Print(
            json::Document{
                json::Builder{}
                .StartDict()
                .Key("request_id"s).Value(rq.request_id)
                .Key("error_message"s).Value("not found"s)
                .EndDict()
                .Build()
            }, std::cout
        );
    }
    else
    {
        json::Print(
            json::Document{
                json::Builder{}
                .StartDict()
                .Key("request_id"s).Value(rq.request_id)
                .Key("curvature"s).Value(curvature)
                .Key("unique_stop_count"s).Value(static_cast<int>(stops_unique))
                .Key("stop_count"s).Value(static_cast<int>(stops_count))
                .Key("route_length"s).Value(distance_real)
                .EndDict()
                .Build()
            }, std::cout
        );
    }
}
void RequestHandler::ParseRequestStop(const Request& rq) const {
    using namespace std::literals;
    const auto [name, routes] = tc_.GetStopInfo(rq.name);
    if (name.empty()) {
        json::Print(
            json::Document{
                json::Builder{}
                .StartDict()
                .Key("request_id"s).Value(rq.request_id)
                .Key("error_message"s).Value("not found"s)
                .EndDict()
                .Build()
            }, std::cout
        );
    }
    else if(routes.empty())
    {
        json::Print(
            json::Document{
                json::Builder{}
                .StartDict()
                .Key("request_id"s).Value(rq.request_id)
                .Key("buses"s)
                    .StartArray()
                    .EndArray()
                .EndDict()
                .Build()
            }, std::cout
        );
    }
    else
    {
        json::Array arr;
        for (const auto bus : routes) {
            arr.push_back(std::string(bus));
        }
        json::Print(
            json::Document{
                json::Builder{}
                .StartDict()
                .Key("request_id"s).Value(rq.request_id)
                .Key("buses"s)
                .Value(arr)
                .EndDict()
                .Build()
            }, std::cout
        );
    }

}

void RequestHandler::ParseRequestMap(const Request& rq) const {
    json::Document doc = renderer_.GetMap(tc_.GetBuses(), rq.request_id);
    json::Print(doc, std::cout);
}

void RequestHandler::ParseRequestRouting(const Request& rq) const {
    using namespace std;
    if (const Stop* stop_from = tc_.SearchStop(rq.route.from)) {
        if (const Stop* stop_to = tc_.SearchStop(rq.route.to)) {
            if (auto route_info = tr_.GetRouteInfo(stop_from, stop_to)) {
                auto [wieght, edges] = route_info.value();

                json::Print(
                    json::Document{
                    json::Builder{}.StartDict()
                    .Key("items"s).Value(tr_.GetEdgesItems(edges))
                    .Key("total_time"s).Value(wieght)
                    .Key("request_id"s).Value(rq.request_id).EndDict().Build()
                    }, std::cout
                    );
                return;
            }
        }
    } 
    json::Print(
        json::Document{
            json::Builder{}.StartDict()
            .Key("error_message"s).Value("not found"s)
            .Key("request_id"s).Value(rq.request_id)
            .EndDict().Build()
        }, std::cout
    );
}