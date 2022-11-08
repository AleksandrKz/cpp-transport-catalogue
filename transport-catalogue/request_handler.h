#pragma once

#include <iostream>
#include <sstream>
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "json.h"

struct RouteFromTo {
    std::string from;
    std::string to;
};

struct Request {
    std::string type;
    std::string name;
    int request_id;
    RouteFromTo route;
};

class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& tc, const TransportRouter& tr, renderer::RendererMap& renderer);
    void AddToRequest(Request&& rq);
    void PrintStat();

private:
    void ParseRequestBus(const Request& rq) const;
    void ParseRequestStop(const Request& rq) const;
    void ParseRequestMap(const Request& rq) const;
    void ParseRequestRouting(const Request& rq) const;

    
    const TransportCatalogue& tc_;
    const TransportRouter& tr_;
    renderer::RendererMap& renderer_;
    std::vector<Request> request_;
};