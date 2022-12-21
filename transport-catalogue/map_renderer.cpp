#include "map_renderer.h"

namespace renderer {

svg::Color RenderSettings::GetNextColorFromColorPalette(bool reset) const {
    static uint32_t index = 0;
    if (reset) index = 0;
    if (index == color_palette.size()) {
        index = 0;
    }
    return color_palette[index++];
}

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

RendererMap::RendererMap() {
}

void RendererMap::SetRendererSettings(RenderSettings rs) {
    rs_ = std::move(rs);
}

const RenderSettings& RendererMap::GetRendererSettings() const {
    return rs_;
}

std::vector<svg::Polyline> RendererMap::GetRouteLine(std::vector<const Bus*>& routes, const SphereProjector& proj) {
    std::vector<svg::Polyline> result;
    bool first_color = true;
    for (const Bus* bus : routes) {
        if (bus->routes.size() == 0) continue;
        svg::Color stroke_color = (first_color) ? rs_.GetNextColorFromColorPalette(first_color) : rs_.GetNextColorFromColorPalette();
        first_color = false;
        svg::Polyline polyline;
        polyline.SetStrokeWidth(rs_.line_width);
        polyline.SetFillColor("none");
        polyline.SetStrokeColor(stroke_color);
        polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        for (auto it = bus->routes.begin(); it != bus->routes.end(); ++it) {
            polyline.AddPoint(proj({ (*it)->latitude, (*it)->longitude }));
        }
        //есди мпршрут не кольцевой, то рисуем в обратную сторону без учёта конечной
        if (bus->type == RouteType::DIRECT) {
            for (auto it = ++bus->routes.rbegin(); it != bus->routes.rend(); ++it) {
                polyline.AddPoint(proj({ (*it)->latitude, (*it)->longitude }));
            }
        }
        result.push_back(polyline);
    }

    return result;
}

std::vector<svg::Text> RendererMap::GetRouteName(std::vector<const Bus*>& routes, const SphereProjector& proj) {
    std::vector<svg::Text> result;
    bool first_color = true;
    for (const Bus* bus : routes) {
        if (bus->routes.size() == 0) continue;
        svg::Color stroke_color = (first_color) ? rs_.GetNextColorFromColorPalette(first_color) : rs_.GetNextColorFromColorPalette();
        first_color = false;
        svg::Text bus_text_front;
        svg::Text bus_text_back;
        //bus_text_f
        bus_text_front.SetData(bus->bus_name);
        bus_text_front.SetPosition(proj({ bus->routes[0]->latitude, bus->routes[0]->longitude }));
        bus_text_front.SetFontFamily(rs_.font_family);
        bus_text_front.SetFontWeight(rs_.font_weight);
        bus_text_front.SetFontSize(rs_.bus_label_font_size);
        bus_text_front.SetOffset(rs_.bus_label_offset);
        bus_text_front.SetFillColor(stroke_color);
        //---------
        //bus_text_b
        bus_text_back.SetData(bus->bus_name);
        bus_text_back.SetPosition(proj({ bus->routes[0]->latitude, bus->routes[0]->longitude }));
        bus_text_back.SetStrokeWidth(rs_.underlayer_width);
        bus_text_back.SetFontFamily(rs_.font_family);
        bus_text_back.SetFontWeight(rs_.font_weight);
        bus_text_back.SetFontSize(rs_.bus_label_font_size);
        bus_text_back.SetOffset(rs_.bus_label_offset);
        bus_text_back.SetFillColor(rs_.underlayer_color);
        bus_text_back.SetStrokeColor(rs_.underlayer_color);
        bus_text_back.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        bus_text_back.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        //---------
        result.push_back(bus_text_back);
        result.push_back(bus_text_front);
        //есди маршрут не кольцевой, то отмечаем вторую конечную
        if (bus->type == RouteType::DIRECT && (bus->routes[0]->stop_name != bus->routes[bus->routes.size()-1]->stop_name)) {
            auto end_stop = bus->routes.rbegin();
            svg::Text txf = bus_text_front;
            txf.SetPosition(proj({ (*end_stop)->latitude, (*end_stop)->longitude }));
            svg::Text txb = bus_text_back;
            txb.SetPosition(proj({ (*end_stop)->latitude, (*end_stop)->longitude }));
            result.push_back(txb);
            result.push_back(txf);
        }
    }

    return result;
}

std::vector<svg::Circle> RendererMap::GetRoutePoint(std::vector<const Bus*>& routes, const SphereProjector& proj) {
    std::vector<svg::Circle> result;
    std::vector<const Stop*> all_stops;
    for (const Bus* bus : routes) {
        if (bus->routes.size() == 0) continue;
        for (const auto stop : bus->routes) {
            all_stops.push_back(stop);
        }
    }

    // сортируем остановки в лексиграфический порядок
    std::sort(all_stops.begin(), all_stops.end(), [](const Stop* lhs, const Stop* rhs) {
        return std::lexicographical_compare(lhs->stop_name.begin(), lhs->stop_name.end(), rhs->stop_name.begin(), rhs->stop_name.end());
    });

    auto last = std::unique(all_stops.begin(), all_stops.end());
    all_stops.erase(last, all_stops.end());

    //circle
    svg::Circle circle;
    circle.SetRadius(rs_.stop_radius);
    circle.SetFillColor("white");
    //---------

    for (const Stop* stop : all_stops) {
        circle.SetCenter(proj({ stop->latitude, stop->longitude }));
        result.push_back(circle);
    }

    return result;
}

std::vector<svg::Text> RendererMap::GetStopsName(std::vector<const Bus*>& routes, const SphereProjector& proj) {
    std::vector<const Stop*> all_stops;
    std::vector<svg::Text> result;

    for (const Bus* bus : routes) {
        if (bus->routes.size() == 0) continue;
        for (const auto stop : bus->routes) {
            all_stops.push_back(stop);
        }
    }

    // сортируем остановки в лексиграфический порядок
    std::sort(all_stops.begin(), all_stops.end(), [](const Stop* lhs, const Stop* rhs) {
        return std::lexicographical_compare(lhs->stop_name.begin(), lhs->stop_name.end(), rhs->stop_name.begin(), rhs->stop_name.end());
    });

    auto last = std::unique(all_stops.begin(), all_stops.end());
    all_stops.erase(last, all_stops.end());

    svg::Text stop_text_front;
    svg::Text stop_text_back;
    //stop_text_b
    stop_text_back.SetStrokeWidth(rs_.underlayer_width);
    stop_text_back.SetFontFamily(rs_.font_family);
    stop_text_back.SetFontSize(rs_.stop_label_font_size);
    stop_text_back.SetOffset(rs_.stop_label_offset);
    stop_text_back.SetFillColor(rs_.underlayer_color);
    stop_text_back.SetStrokeColor(rs_.underlayer_color);
    stop_text_back.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    stop_text_back.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    //---------
    //stop_text_f
    stop_text_front.SetFontFamily(rs_.font_family);
    stop_text_front.SetFontSize(rs_.stop_label_font_size);
    stop_text_front.SetOffset(rs_.stop_label_offset);
    stop_text_front.SetFillColor("black");
    //---------

    for (const Stop* stop : all_stops) {
        stop_text_back.SetData(stop->stop_name);
        stop_text_front.SetData(stop->stop_name);
        stop_text_back.SetPosition(proj({ stop->latitude, stop->longitude }));
        stop_text_front.SetPosition(proj({ stop->latitude, stop->longitude }));
        result.push_back(stop_text_back);
        result.push_back(stop_text_front);
    }
    return result;
}

void RendererMap::ParseOut(const std::deque<Bus>& routes) {
    std::vector<const Bus*> routes_to_print;
    std::vector<geo::Coordinates> geo_coords;

    for (const Bus& bus : routes) {
        for (const Stop* stop :  bus.routes) {
            geo_coords.push_back({stop->latitude, stop->longitude});
        }
    }

    for (const Bus& bus : routes) {
        routes_to_print.push_back(&bus);
    }

    // сортируем маршруты в лексиграфический порядок
    std::sort(routes_to_print.begin(), routes_to_print.end(), [](const Bus* lhs, const Bus* rhs) {
        return std::lexicographical_compare(lhs->bus_name.begin(), lhs->bus_name.end(), rhs->bus_name.begin(), rhs->bus_name.end());
    });

    // Создаём проектор сферических координат на карту
    const SphereProjector proj{geo_coords.begin(), geo_coords.end(), rs_.width, rs_.height, rs_.padding};

    for (svg::Polyline& pol : GetRouteLine(routes_to_print, proj)) {
        doc_.Add(std::move(pol));
    }   
    for (svg::Text& txt : GetRouteName(routes_to_print, proj)) {
        doc_.Add(std::move(txt));
    }
    for (svg::Circle& crc : GetRoutePoint(routes_to_print, proj)) {
        doc_.Add(std::move(crc));
    }
    for (svg::Text& txt : GetStopsName(routes_to_print, proj)) {
        doc_.Add(std::move(txt));
    }
}

json::Document RendererMap::GetMap(const std::deque<Bus>& routes, int id) {
    using namespace std::literals;
    std::stringstream ss;
    ParseOut(routes);
    doc_.Render(ss);
    return json::Document{
            json::Builder{}
            .StartDict()
            .Key("request_id"s).Value(id)
            .Key("map"s).Value(ss.str())
            .EndDict()
            .Build()
    };
}

void RendererMap::PrintMap(const std::deque<Bus>& routes) {
    std::stringstream ss;
    ParseOut(routes);
    doc_.Render(std::cout);
}

}
