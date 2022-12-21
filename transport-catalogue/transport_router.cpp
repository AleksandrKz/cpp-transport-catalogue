#include "transport_router.h"

TransportRouter::TransportRouter(const TransportCatalogue& tc, const RoutingSettings& rs) : tc_(tc)
{
	SetSettings(rs);
}

TransportRouter::~TransportRouter()
{
	delete router_ptr_;
}

void TransportRouter::SetSettings(const RoutingSettings& rs) {
	bus_wait_time_ = rs.bus_wait_time;
	bus_velocity_ = rs.bus_velocity;
}

void TransportRouter::SetGraph(graph::DirectedWeightedGraph<double>&& graph, std::map<std::string, graph::VertexId>&& stop_ids) {
    graph_ = std::move(graph);
    stop_ids_ = std::move(stop_ids);
    router_ptr_ = new graph::Router<double>(graph_);
}

const RoutingSettings TransportRouter::GetSettings() const {
    return {bus_wait_time_, bus_velocity_};
}

const std::map<std::string, graph::VertexId>& TransportRouter::GetStopIds() const {
    return stop_ids_;
}

void TransportRouter::BuildGraph() {
    using namespace std;
    const unordered_map<string_view, const Stop*>& all_stops = tc_.GetAllStops();
    const unordered_map<string_view, const Bus*>& all_buses = tc_.GetAllBuses();
    graph_.ResizeGraph(all_stops.size() * 2);
    map<string, graph::VertexId> stop_ids;
    graph::VertexId vertex_id = 0;
    for (const auto& [stop_name, stop_ptr] : all_stops) {
        stop_ids[stop_ptr->stop_name] = vertex_id;
        graph_.AddEdge({ stop_ptr->stop_name,
                              0,
                              vertex_id,
                              ++vertex_id,
                              static_cast<double>(bus_wait_time_)}
        );
        ++vertex_id;
    }
    stop_ids_ = move(stop_ids);

    for (const auto& item : all_buses) {
        const Bus* bus_ptr = item.second;
        const std::vector<const Stop*>& stops = bus_ptr->routes;
        size_t stops_count = bus_ptr->routes.size();
        for (size_t i = 0; i < stops_count; ++i) {
            for (size_t j = i + 1; j < stops_count; ++j) {
                const Stop* stop_from = stops[i];
                const Stop* stop_to = stops[j];
                double dist_sum_forward = 0;
                double dist_sum_backward = 0;
                for (size_t k = i + 1; k <= j; ++k) {
                    dist_sum_forward += static_cast<double>(tc_.GetRealDistance(stops[k - 1], stops[k]));
                    if (bus_ptr->type == RouteType::DIRECT)
                        dist_sum_backward += static_cast<double>(tc_.GetRealDistance(stops[k], stops[k - 1]));
                }

                graph_.AddEdge({ bus_ptr->bus_name,
                                      j - i,
                                      stop_ids_.at(stop_from->stop_name) + 1,
                                      stop_ids_.at(stop_to->stop_name),
                                      GetTravelTime(dist_sum_forward) });

                if (bus_ptr->type == RouteType::DIRECT) {
                    graph_.AddEdge({ bus_ptr->bus_name,
                                      j - i,
                                      stop_ids_.at(stop_to->stop_name) + 1,
                                      stop_ids_.at(stop_from->stop_name),
                                      GetTravelTime(dist_sum_backward) });
                }
            }
        }
    }
    router_ptr_ = new graph::Router<double>(graph_);
}

json::Array TransportRouter::GetEdgesItems(const std::vector<graph::EdgeId>& edges) const {
    using namespace std;
    json::Array items_array;
    items_array.reserve(edges.size());
    for (auto& edge_id : edges) {
        const graph::Edge<double>& edge = graph_.GetEdge(edge_id);
        if (edge.quality == 0) {
            items_array.emplace_back(json::Node(json::Dict{
                {{"stop_name"s},{static_cast<string>(edge.name)}},
                {{"time"s},{edge.weight}},
                {{"type"s},{"Wait"s}}
                }));
        }
        else {
            items_array.emplace_back(json::Node(json::Dict{
                {{"bus"s},{static_cast<string>(edge.name)}},
                {{"span_count"s},{static_cast<int>(edge.quality)}},
                {{"time"s},{edge.weight}},
                {{"type"s},{"Bus"s}}
                }));
        }
    }
    return items_array;
}

std::optional<graph::Router<double>::RouteInfo> TransportRouter::GetRouteInfo(const Stop* from, const Stop* to) const {
    return router_ptr_->BuildRoute(stop_ids_.at(from->stop_name), stop_ids_.at(to->stop_name));
}

const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
    return graph_;
}

double TransportRouter::GetTravelTime(double way)
{
    const double min_in_hour = 60.;
    const double m_in_km = 1000.;
    return (way  / (bus_velocity_ * m_in_km) * min_in_hour);
}
