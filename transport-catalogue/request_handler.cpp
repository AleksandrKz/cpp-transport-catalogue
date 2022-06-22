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
    using namespace std::literals;
    //std::stringstream ss;
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
    //std::stringstream ss;
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
        //bool space = false;
        for (const auto bus : routes) {
            //if (space) {
                //std::cout << ", ";
            //}
            //space = true;
            arr.push_back(std::string(bus));
            //ss << bus;
        }
        //json::Node buses = json::Load(ss);
        json::Print(
            json::Document{
                json::Builder{}
                .StartDict()
                .Key("request_id"s).Value(rq.request_id)
                .Key("buses"s)
                //.StartArray()
                //.Value(ss.str())
                .Value(arr)
                //.EndArray()
                .EndDict()
                .Build()
            }, std::cout
        );
    }

}

void RequestHandler::ParseRequestMap(const Request& rq) const {
    using namespace std::literals;
    //json::Document doc = renderer_.GetMap(tc_.GetBuses());
    json::Document doc = renderer_.GetMap(tc_.GetBuses(), rq.request_id);
    json::Print(doc, std::cout);
    /*
    std::cout << "\t{\n";
    std::cout << "\t\t\"request_id\" : " << rq.request_id << ',' << std::endl;
    std::cout << "\t\t\"map\" : ";
    json::Print(doc, std::cout);
    std::cout << "\n\t}" << std::endl;
    */
}