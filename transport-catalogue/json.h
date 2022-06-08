#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <utility>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using Number = std::variant<int, double>;
using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

struct ValuePrinter {
    std::ostream& out;
    void operator()(std::nullptr_t) const;
    void operator()(double d) const;
    void operator()(int i) const;
    void operator()(std::string s) const;
    void operator()(bool b) const;
    void operator()(Array arr) const;
    void operator()(Dict dic) const;
};


// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final : private Value {
public:
    using variant::variant;

    const std::string& AsString() const;
    const Dict& AsMap() const;
    const Array& AsArray() const;
    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;
    
    const Value& GetValue() const;

private:

};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

bool operator==(Document lhs, Document rhs);
bool operator!=(Document lhs, Document rhs);

bool operator==(Node lhs, Node rhs);
bool operator!=(Node lhs, Node rhs);

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

void PrintNode(const Node& node, std::ostream& out);

}  // namespace json