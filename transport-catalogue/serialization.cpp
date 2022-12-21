#include "serialization.h"

void Serialize(const TransportCatalogue& tc, renderer::RendererMap& renderer, TransportRouter& tr, std::ostream& ofs) {
	serialize::TransportCatalogue db;
	for (const auto& [_, stop] : tc.GetAllStops()) {
		*db.add_stop() = Serialize(stop);
	}
	for (const auto& [_, bus] : tc.GetAllBuses()) {
		*db.add_bus() = Serialize(bus);
	}
	for (const auto& [stops, dist] : tc.GetAllDistance()) {
		*db.add_distance() = Serialize(stops.first->stop_name, stops.second->stop_name, dist);
	}
	
	*db.mutable_render_settings() = Serialize(renderer.GetRendererSettings());

	*db.mutable_router() = Serialize(tr);

	db.SerializeToOstream(&ofs);
}

std::tuple<TransportCatalogue, renderer::RendererMap, TransportRouter, Graph, StopIds> Deserialize(std::ifstream& ifs) {
	serialize::TransportCatalogue db;
	db.ParseFromIstream(&ifs);
	TransportCatalogue tc;
	renderer::RendererMap render;
	MakeTCFromDB(tc, db);
	MakeRMFromDB(render, db);
	TransportRouter router = MakeTRFromDB(tc, db);
	return { std::move(tc), std::move(render), std::move(router), GetGraphFromDB(db), GetStopIdsFromDB(db) };
}

serialize::Stop Serialize(const Stop* stop) {
	serialize::Stop result;
	result.set_name(stop->stop_name);
	result.add_coordinate(stop->latitude);
	result.add_coordinate(stop->longitude);
	return result;
}

serialize::Bus Serialize(const Bus* bus) {
	serialize::Bus result;
	result.set_name(bus->bus_name);
	for (const auto& stop : bus->routes) {
		result.add_stop(stop->stop_name);
	}
	result.set_is_circle(bus->type == RouteType::RING ? true : false);
	return result;
}

serialize::Distance Serialize(const std::string from, const std::string to, const uint64_t dist) {
	serialize::Distance result;
	result.set_from(from);
	result.set_to(to);
	result.set_distance(dist);
	return result;
}

void MakeTCFromDB(TransportCatalogue& tc, const serialize::TransportCatalogue& db) {
	for (size_t i = 0; i < db.stop_size(); ++i) {
		const serialize::Stop& stop = db.stop(i);
		tc.AddStop(stop.name(), stop.coordinate(0), stop.coordinate(1));
	}

	for (size_t i = 0; i < db.distance_size(); ++i) {
		const serialize::Distance& distance = db.distance(i);
		tc.AddRealDistance(distance.from(), distance.to(), distance.distance());
	}

	for (size_t i = 0; i < db.bus_size(); ++i) {
		const serialize::Bus& bus = db.bus(i);
		std::vector<std::string> route;
		for (size_t j = 0; j < bus.stop_size(); ++j) {
			route.push_back(bus.stop(j));
		}
		tc.AddRoute(bus.name(), route, bus.is_circle());
	}
}


serialize::Point Serialize(const svg::Point p) {
	serialize::Point result;
	result.set_x(p.x);
	result.set_y(p.y);
	return result;
}

serialize::Color Serialize(const svg::Color col) {
	serialize::Color result;
	if (std::holds_alternative<svg::Rgb>(col)) {
		svg::Rgb rgb = std::get<svg::Rgb>(col);
		serialize::RGB ser_rgb;
		ser_rgb.set_red(rgb.red);
		ser_rgb.set_green(rgb.green);
		ser_rgb.set_blue(rgb.blue);
		*result.mutable_rgb() = ser_rgb;
	}
	else if(std::holds_alternative<svg::Rgba>(col))	{
		svg::Rgba rgba = std::get<svg::Rgba>(col);
		serialize::RGBA ser_rgba;
		ser_rgba.set_red(rgba.red);
		ser_rgba.set_green(rgba.green);
		ser_rgba.set_blue(rgba.blue);
		ser_rgba.set_opacity(rgba.opacity);
		*result.mutable_rgba() = ser_rgba;
	}
	else if(std::holds_alternative<std::string>(col)){
		result.set_name(std::get<std::string>(col));
	}
	return result;
}

serialize::RenderSettings Serialize(const renderer::RenderSettings& rs) {
	serialize::RenderSettings result;
	result.set_font_family(rs.font_family);
	result.set_font_weight(rs.font_weight);

	result.set_width(rs.width);
	result.set_height(rs.height);

	result.set_padding(rs.padding);

	result.set_line_width(rs.line_width);
	result.set_stop_radius(rs.stop_radius);

	result.set_bus_label_font_size(rs.bus_label_font_size);
	*result.mutable_bus_label_offset() = Serialize(rs.bus_label_offset);

	result.set_stop_label_font_size(rs.stop_label_font_size);
	*result.mutable_stop_label_offset() = Serialize(rs.stop_label_offset);

	*result.mutable_underlayer_color() = Serialize(rs.underlayer_color);
	result.set_underlayer_width(rs.underlayer_width);
	for (const auto& color : rs.color_palette) {
		*result.add_color_palette() = Serialize(color);
	}
	return result;
}

svg::Color GetColorFromSerializeColor(const serialize::Color& color) {
	if (!color.name().empty()) {
		return svg::Color(color.name());
	}
	else if(color.has_rgb()) {
		const serialize::RGB& rgb = color.rgb();
		return svg::Color(svg::Rgb(rgb.red(), rgb.green(), rgb.blue()));
	}
	else if (color.has_rgba()) {
		const serialize::RGBA& rgba = color.rgba();
		return svg::Color(svg::Rgba(rgba.red(), rgba.green(), rgba.blue(), rgba.opacity()));
	}
	else {
		return svg::Color("none");
	}
}

void MakeRMFromDB(renderer::RendererMap& rm, const serialize::TransportCatalogue& db) {
	renderer::RenderSettings rs;
	const serialize::RenderSettings set = db.render_settings();
	rs.font_family = set.font_family();
	rs.font_weight = set.font_weight();

	rs.width = set.width();
	rs.height = set.height();

	rs.padding = set.padding();

	rs.line_width = set.line_width();
	rs.stop_radius = set.stop_radius();

	rs.bus_label_font_size = set.bus_label_font_size();
	rs.bus_label_offset = svg::Point(set.bus_label_offset().x(), set.bus_label_offset().y());

	rs.stop_label_font_size = set.stop_label_font_size();
	rs.stop_label_offset = svg::Point(set.stop_label_offset().x(), set.stop_label_offset().y());

	rs.underlayer_color = GetColorFromSerializeColor(set.underlayer_color());
	rs.underlayer_width = set.underlayer_width();
	for (size_t i = 0; i < set.color_palette_size(); ++i) {
		rs.color_palette.push_back(GetColorFromSerializeColor(set.color_palette(i)));
	}
	rm.SetRendererSettings(rs);
}


serialize::RoutingSettings Serialize(const RoutingSettings rs) {
	serialize::RoutingSettings result;
	result.set_bus_velocity(rs.bus_velocity);
	result.set_bus_wait_time(rs.bus_wait_time);
	return result;
}

serialize::StopId Serialize(const std::string& name, const size_t id) {
	serialize::StopId result;
	result.set_name(name);
	result.set_id(id);
	return result;
}

serialize::Edge Serialize(const graph::Edge<double>& edge) {
	serialize::Edge result;
	result.set_name(edge.name);
	result.set_quality(edge.quality);
	result.set_from(edge.from);
	result.set_to(edge.to);
	result.set_weight(edge.weight);
	return result;
}

serialize::Graph Serialize(const Graph& g) {
	serialize::Graph result;
	size_t vertex_count = g.GetVertexCount();
	size_t edge_count = g.GetEdgeCount();
	for (size_t i = 0; i < edge_count; ++i) {
		*result.add_edge() = Serialize(g.GetEdge(i));
	}
	for (size_t i = 0; i < vertex_count; ++i) {
		serialize::Vertex vertex;
		for (const auto& edge_id : g.GetIncidentEdges(i)) {
			vertex.add_edge_id(edge_id);
		}
		*result.add_vertex() = vertex;
	}
	return result;
}

serialize::Router Serialize(const TransportRouter& tr) {
	serialize::Router result;
	*result.mutable_routing_settings() = Serialize(tr.GetSettings());
	*result.mutable_graph() = Serialize(tr.GetGraph());
	for (const auto& [name, id] : tr.GetStopIds()) {
		*result.add_stop_id() = Serialize(name, id);
	}
	return result;
}

const RoutingSettings GetRouterSettingsFromDB(const serialize::TransportCatalogue& db) {
	RoutingSettings rs;
	const serialize::RoutingSettings& set = db.router().routing_settings();
	rs.bus_velocity = set.bus_velocity();
	rs.bus_wait_time = set.bus_wait_time();
	return rs;
}

Graph GetGraphFromDB(const serialize::TransportCatalogue& db) {
	const serialize::Graph& g = db.router().graph();
	std::vector<graph::Edge<double>> edges(g.edge_size());
	std::vector<std::vector<graph::EdgeId>> incidence_lists(g.vertex_size());
	for (size_t i = 0; i < edges.size(); ++i) {
		const serialize::Edge& e = g.edge(i);
		edges[i] = { e.name(), static_cast<size_t>(e.quality()),
		static_cast<size_t>(e.from()), static_cast<size_t>(e.to()), e.weight() };
	}
	for (size_t i = 0; i < incidence_lists.size(); ++i) {
		const serialize::Vertex& v = g.vertex(i);
		incidence_lists[i].reserve(v.edge_id_size());
		for (const auto& id : v.edge_id()) {
			incidence_lists[i].push_back(id);
		}
	}
	return Graph(edges, incidence_lists);
}

StopIds GetStopIdsFromDB(const serialize::TransportCatalogue& db) {
	StopIds result;
	for (const auto& s : db.router().stop_id()) {
		result[s.name()] = s.id();
	}
	return result;
}

TransportRouter MakeTRFromDB(const TransportCatalogue& tc, const serialize::TransportCatalogue& db) {
	return std::move(TransportRouter(tc, GetRouterSettingsFromDB(db)));
}