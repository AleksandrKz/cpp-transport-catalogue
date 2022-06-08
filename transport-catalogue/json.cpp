#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadNull(std::istream& input) {
    using namespace std::literals;
    string line;
    while (std::isalpha(input.peek())) {
        line.push_back(static_cast<char>(input.get()));
    }
    
    if(line == "null"sv)
        return Node(nullptr);
    else
        throw ParsingError("String parsing error");
}

Node LoadBool(std::istream& input) {
    using namespace std::literals;
    string line;
    while (std::isalpha(input.peek())) {
        line.push_back(static_cast<char>(input.get()));
    }

    if(line == "true"sv) {
        return Node(true);
    }
    else if(line == "false"sv) {
        return Node(false);
    }
    else {
        throw ParsingError("String parsing error");
    }
}

Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(std::stoi(parsed_num));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(std::stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
Node LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node(move(s));
}

Node LoadArray(istream& input) {
    Array result;
    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if(!input) {
        throw ParsingError("Array parsing error");
    }

    return Node(move(result));
}

Node LoadDict(istream& input) {
    Dict result;

    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }
        std::string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }

    if(!input) {
        throw ParsingError("Dictonary parsing error");
    }

    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if(c == 'n') {
        input.putback(c);
        return LoadNull(input);
    } else if(c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    } else {
        input.putback(c);
        return LoadNumber(input);
    }
}

}  // namespace

const Value& Node::GetValue() const {
    return *this;
}

const Array& Node::AsArray() const {
    using namespace std::literals;
    if(!IsArray()) {
        throw std::logic_error("Not an array"s);
    }
    return std::get<Array>(*this);
}

const Dict& Node::AsMap() const {
    using namespace std::literals;
    if(!IsMap()) {
        throw std::logic_error("Not an dictonary"s);
    }
    return std::get<Dict>(*this);
}

int Node::AsInt() const {
    using namespace std::literals;
    if (!IsInt()) {
        throw std::logic_error("Not an int"s);
    }
    return std::get<int>(*this);
}

double Node::AsDouble() const {
    using namespace std::literals;
    if(!IsDouble()) {
        throw std::logic_error("Not an double"s);
    }
    if(IsPureDouble()) {
        return std::get<double>(*this);
    } else {
        return static_cast<double>(std::get<int>(*this));
    }
}

const std::string& Node::AsString() const {
    using namespace std::literals;
    if(!IsString()) {
        throw std::logic_error("Not an string");
    }
    return std::get<std::string>(*this);
}

bool Node::AsBool() const {
    using namespace std::literals;
    if(!IsBool()) {
        throw std::logic_error("Not an bool");
    }
    return std::get<bool>(*this);
}


bool Node::IsInt() const {
    return std::holds_alternative<int>(*this);
}

bool Node::IsDouble() const {
    return std::holds_alternative<double>(*this) || std::holds_alternative<int>(*this);
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(*this);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(*this);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(*this);
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(*this);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(*this);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(*this);
}

bool operator==(Node lhs, Node rhs) {
    return lhs.GetValue() == rhs.GetValue();
}

bool operator!=(Node lhs, Node rhs) {
    return lhs.GetValue() != rhs.GetValue();
}

bool operator==(Document lhs, Document rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

bool operator!=(Document lhs, Document rhs) {
    return lhs.GetRoot() != rhs.GetRoot();
}


Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}


void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), output);
}


void PrintNode(const Node& node, std::ostream& out) {
    std::visit(ValuePrinter{out}, node.GetValue());
} 


void ValuePrinter::operator()(std::nullptr_t) const {
    using namespace std::literals;
    out << "null"sv;
}
void ValuePrinter::operator()(double d) const {
    out << d;
}
void ValuePrinter::operator()(int i) const {
    out << i;
}
void ValuePrinter::operator()(std::string str) const {
    out << "\"";
    for (auto ch : str) {
        switch (ch) {
            case '\"':
                out << "\\\"";
                break;
            case '\\':
                out << "\\\\";
                break;
            case '\n':
                out << "\\n";
                break;
            case '\r':
                out << "\\r";
                break;
            case '\t':
                out << "\t";
                break;
            default:
                out << ch;
        }
    }
    out << "\"";
}


void ValuePrinter::operator()(bool b) const {
    out << std::boolalpha << b;
}

void ValuePrinter::operator()(Array array) const {
    bool space = false;
    out << "[";
    for (const auto& arr : array) {
        if (space) {
            out << ", ";
        }
        space = true;
        PrintNode(arr, out);
    }
    out << "]";
}

void ValuePrinter::operator()(Dict dic) const {
    bool space = false;
    out << "{ ";
    for (const auto& [key, value] : dic) {
        if (space) {
                out << ", ";
        }
        space = true;
        out << '\"' << key << '\"';
        out << ": ";
        PrintNode(value, out);
    }
    out << "}";   
}

}  // namespace json