#pragma once

#include <iostream>
#include <sstream>
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_builder.h"

struct Request {
    std::string type;
    std::string name;
    int request_id;
};

class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& tc, renderer::RendererMap& renderer);
    void AddToRequest(Request&& rq);
    void PrintStat();

private:
    void ParseRequestBus(const Request& rq) const;
    void ParseRequestStop(const Request& rq) const;
    void ParseRequestMap(const Request& rq) const;

    
    const TransportCatalogue& tc_;
    renderer::RendererMap& renderer_;
    std::vector<Request> request_;
};