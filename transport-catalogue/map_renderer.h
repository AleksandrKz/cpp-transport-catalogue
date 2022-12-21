#pragma once

#include <algorithm>
#include <utility>
#include <iterator>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <unordered_set>
#include <variant>
#include <string>
#include <array>
#include <deque>

#include <sstream>

#include "geo.h"
#include "domain.h"
#include "svg.h"
#include "json_builder.h"


namespace renderer {

class SphereProjector;

struct RenderSettings {
    std::string font_family = "Verdana";
    std::string font_weight = "bold";

    double width = 0.;
    double height = 0.;

    double padding = 0.;

    double line_width = 0.;
    double stop_radius = 0.;

    uint32_t bus_label_font_size = 1;
    svg::Point bus_label_offset;

    uint32_t stop_label_font_size = 1;
    svg::Point stop_label_offset;

    svg::Color underlayer_color;
    double underlayer_width = 0.;
    std::vector<svg::Color> color_palette;

    svg::Color GetNextColorFromColorPalette(bool reset = false) const;
};


class RendererMap {
public:
    RendererMap();

    void ParseOut(const std::deque<Bus>& routes) ;

    void SetRendererSettings(RenderSettings rs);
    const RenderSettings& GetRendererSettings() const;

    void PrintMap(const std::deque<Bus>& routes);

    json::Document GetMap(const std::deque<Bus>& routes, int id) ;

private:
    std::vector<svg::Polyline> GetRouteLine(std::vector<const Bus*>& routes, const SphereProjector& proj) ;
    std::vector<svg::Text> GetRouteName(std::vector<const Bus*>& routes, const SphereProjector& proj) ;
    std::vector<svg::Circle> GetRoutePoint(std::vector<const Bus*>& routes, const SphereProjector& proj) ;
    std::vector<svg::Text> GetStopsName(std::vector<const Bus*>& routes, const SphereProjector& proj) ;

    RenderSettings rs_;
    svg::Document doc_;
};

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

} //namespace renderer
