#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <tuple>

#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "transport_catalogue.pb.h"
#include "map_renderer.pb.h"
#include "transport_router.pb.h"

using Graph = graph::DirectedWeightedGraph<double>;
using StopIds = std::map<std::string, graph::VertexId>;

void Serialize(const TransportCatalogue& tc, renderer::RendererMap& renderer, TransportRouter& tr, std::ostream& ofs);
std::tuple<TransportCatalogue, renderer::RendererMap, TransportRouter, Graph, StopIds> Deserialize(std::ifstream& ifs);

serialize::Stop Serialize(const Stop* stop);
serialize::Bus Serialize(const Bus* bus);
serialize::Distance Serialize(const std::string from, const std::string to, const uint64_t dist);

serialize::RenderSettings Serialize(const renderer::RenderSettings& rs);
serialize::Point Serialize(const svg::Point p);
serialize::Color Serialize(const svg::Color col);
svg::Color GetColorFromSerializeColor(const serialize::Color& color);

serialize::Router Serialize(const TransportRouter& tr);
serialize::RoutingSettings Serialize(const RoutingSettings rs);
serialize::StopId Serialize(const std::string& name, const size_t id);
serialize::Edge Serialize(const graph::Edge<double>& edge);
serialize::Graph Serialize(const Graph& g);
const RoutingSettings GetRouterSettingsFromDB(const serialize::TransportCatalogue& db);
Graph GetGraphFromDB(const serialize::TransportCatalogue& db);
StopIds GetStopIdsFromDB(const serialize::TransportCatalogue& db);

void MakeTCFromDB(TransportCatalogue& tc, const serialize::TransportCatalogue& db);
void MakeRMFromDB(renderer::RendererMap& rm, const serialize::TransportCatalogue& db);
TransportRouter MakeTRFromDB(const TransportCatalogue& tc, const serialize::TransportCatalogue& db);