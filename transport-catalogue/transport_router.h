#pragma once

//#include <iomanip>

#include <optional>
#include <string>
#include <string_view>
#include <map>

#include "json.h"
#include "domain.h"
#include "router.h"
#include "graph.h"
#include "transport_catalogue.h"

class TransportRouter
{
public:
	TransportRouter() = default;
	explicit TransportRouter(const TransportCatalogue& tc);
	explicit TransportRouter(const RoutingSettings& rs, const TransportCatalogue& tc);
	~TransportRouter();

	void SetSettings(const RoutingSettings& rs);
	const RoutingSettings GetSettings() const;
	void BuildGraph();

	json::Array GetEdgesItems(const std::vector<graph::EdgeId>& edges) const;

	std::optional<graph::Router<double>::RouteInfo> GetRouteInfo(const Stop* from, const Stop* to) const;


private:
	double GetTravelTime(double way);

	int bus_wait_time_ = 1;
	double bus_velocity_ = 1.0;

	const TransportCatalogue& tc_;

	graph::DirectedWeightedGraph<double> graph_;
	std::map<std::string, graph::VertexId> stop_ids_;

	graph::Router<double>* router_ptr_ = nullptr;
};
