#pragma once

#include <iostream>
#include <sstream>
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "json.h"

class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& tc, renderer::RendererMap& renderer, const TransportRouter& tr);

    void AddToRequest(Request&& rq);
    void AddAllRequest(std::vector<Request>&& rq);
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