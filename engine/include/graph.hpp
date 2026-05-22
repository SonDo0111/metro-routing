#pragma once
#include <array>
#include <cstddef>
#include "generated/edge.hpp"
#include "generated/node.hpp"

namespace GTFSData
{
    struct graph_t
    {
        // Graph member
        static constexpr std::size_t num_nodes = NUM_NODES;
        static constexpr std::size_t num_edges = NUM_EDGES;

        const std::array<std::size_t, NUM_NODES + 1> &offsets;
        const std::array<std::size_t, NUM_EDGES> &targets;
        const std::array<std::size_t, NUM_EDGES> &weights;

        // Constructor
        constexpr graph_t(
            const std::array<std::size_t, NUM_NODES + 1> &off,
            const std::array<std::size_t, NUM_EDGES> &tgt,
            const std::array<std::size_t, NUM_EDGES> &w) noexcept
            : offsets(off), targets(tgt), weights(w) {}

        constexpr std::size_t degree(node_t u) const noexcept { return offsets[u + 1] - offsets[u]; }

        // Return the i-th outgoing edge from source node u.
        constexpr edge_t adj_edge(node_t u, std::size_t i) const noexcept
        {
            const auto idx = offsets[u] + i;
            return {targets[idx], weights[idx]};
        }
    };

    static_assert(node_offset.size() == NUM_NODES + 1);
    static_assert(targets.size() == NUM_EDGES);
    static_assert(weights.size() == NUM_EDGES);

    constexpr graph_t graph{node_offset, targets, weights};
}
