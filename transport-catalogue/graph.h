#pragma once

#include "ranges.h"

#include <utility>
#include <cstdlib>
#include <vector>
#include <string>
#include <string_view>

namespace graph {

    using VertexId = size_t; //id вершины
    using EdgeId = size_t; // id ребра

    // структура ребра
    template <typename Weight> //вес ребра
    struct Edge {
        std::string name;
        size_t quality;
        VertexId from;
        VertexId to;
        Weight weight;
    };

    // направленный взвешенный граф
    template <typename Weight>
    class DirectedWeightedGraph {
    private:
        using IncidenceList = std::vector<EdgeId>; // вектор с id рёбер
        using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>; // итератор

    public:
        DirectedWeightedGraph() = default;
        explicit DirectedWeightedGraph(size_t vertex_count); // кол-во вершин
        explicit DirectedWeightedGraph(std::vector<Edge<Weight>> edges,
        std::vector<std::vector<EdgeId>> incidence_lists); // вектор рёбер и вектор векторов id рёбер
        EdgeId AddEdge(Edge<Weight>&& edge); // добавить ребро в граф

        size_t GetVertexCount() const; // вернуть кол-во вершин
        size_t GetEdgeCount() const; // вернуть кол-во рёбер
        const Edge<Weight>& GetEdge(EdgeId edge_id) const; //вернуть ребро по id
        IncidentEdgesRange GetIncidentEdges(VertexId vertex) const; // вернуть итератор на ребро

        //const std::vector<Edge<Weight>>& GetEdgTest() const { return edges_; }
        void ResizeGraph(size_t vertex_count);

    private:
        std::vector<Edge<Weight>> edges_; //вектор структур рёбер
        std::vector<IncidenceList> incidence_lists_; //вектор векторов с id рёбер
    };

    template <typename Weight>
    void DirectedWeightedGraph<Weight>::ResizeGraph(size_t vertex_count) {
        incidence_lists_.resize(vertex_count);
        edges_.reserve(vertex_count * vertex_count);
    }

    template <typename Weight>
    DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
        : incidence_lists_(vertex_count) {
        edges_.reserve(vertex_count * vertex_count);
    }

    template <typename Weight>
    DirectedWeightedGraph<Weight>::DirectedWeightedGraph(std::vector<Edge<Weight>> edges,
        std::vector<std::vector<EdgeId>> incidence_lists)
        : edges_(edges)
        , incidence_lists_(incidence_lists) {}

    template <typename Weight>
    EdgeId DirectedWeightedGraph<Weight>::AddEdge(Edge<Weight>&& edge) {
        edges_.emplace_back(std::move(edge));
        const EdgeId id = edges_.size() - 1;
        incidence_lists_.at(edges_.back().from).push_back(id);
        return id;
    }

    template <typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
        return incidence_lists_.size();
    }

    template <typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
        return edges_.size();
    }

    template <typename Weight>
    const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
        return edges_.at(edge_id);
    }

    template <typename Weight>
    typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
        DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
        return ranges::AsRange(incidence_lists_.at(vertex));
    }
}  // namespace graph
