#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& out, StrokeLineCap lc) {
    using namespace std::literals;
    switch(lc) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
        }
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin lg) {
    using namespace std::literals;
    switch(lg) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
    }
    return out;
}

Rgb::Rgb(uint8_t red, uint8_t green, uint8_t blue)
    : red(red), green(green), blue(blue) {
}

std::ostream& operator<<(std::ostream& out, Rgb rgb) {
    using namespace std::literals;
    out << "rgb("sv 
        << std::to_string(rgb.red)
        << ","sv
        << std::to_string(rgb.green)
        << ","sv
        << std::to_string(rgb.blue)
        << ")"sv;
    return out;
}

Rgba::Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity) 
    : Rgb(red, green, blue), opacity(opacity) {
}

std::ostream& operator<<(std::ostream& out, Rgba rgba) {
    using namespace std::literals;
    out << "rgba("sv 
        << std::to_string(rgba.red)
        << ","sv
        << std::to_string(rgba.green)
        << ","sv
        << std::to_string(rgba.blue)
        << ","sv
        << rgba.opacity
        << ")"sv;
    return out;
}

std::ostream& operator<<(std::ostream& out, Color color) {
    std::visit(ColorVisitor{out}, color);
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
//<polyline points="15.2447,51.5451 10.4894,46.9098 17.0611,45.9549 20,40" />
    out << "<polyline";
    // Выводим атрибуты, унаследованные от PathProps
    out << " points=\""sv;
    bool space = false;
    for(const Point point : points_) {
        if(space)
            out << ' ';
        out << point.x << ',' << point.y;
        space = true;
    }
    out << "\"";
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Text ------------------

std::string Text::TextFormating(std::string& text) {
    std::string result;
    for(const char ch : text) {
        switch (ch) {
            case '"':
                result += "&quot;";
                break;

            case '\'':
                result += "&apos;";
                break;

            case '<':
                result += "&lt;";
                break;

            case '>':
                result += "&gt;";
                break;

            case '&':
                result += "&amp;";
                break;

            default:
                result += ch;
        }
    }
    return result;
}

Text& Text::SetPosition(Point pos) {
    position_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = std::move(TextFormating(data));
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
//<text x="35" y="20" dx="0" dy="6" font-size="12" font-family="Verdana" font-weight="bold">Hello C++</text>
    out << "<text"sv;
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);
    out << " x=\""sv << position_.x <<  "\""sv
        << " y=\""sv << position_.y << "\""sv
        << " dx=\""sv << offset_.x << "\""sv
        << " dy=\""sv << offset_.y << "\""sv
        << " font-size=\""sv << size_ << "\""sv;
    if(!font_family_.empty())
        out << " font-family=\""sv << font_family_ << "\""sv;
    if(!font_weight_.empty())
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    
    out << ">"sv
        << data_
        << "</text>"sv;
}


//---------------- Document ---------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    RenderContext context(out, 2, 2);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for(auto& obj : objects_) {
        obj->Render(context);
    }
    out << "</svg>"sv;
}

}  // namespace svg