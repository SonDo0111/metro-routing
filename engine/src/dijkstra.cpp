#include <iostream>
#include <cmath>
#include <limits>
#include <vector>
#include <array>
#include <queue>
#include <string_view>
#include <cstddef>
#include <utility>
#include <algorithm>
#include "graph.hpp"

constexpr GTFSData::weight_t INF = std::numeric_limits<GTFSData::weight_t>::max() / 2;

struct QueueElement
{
    GTFSData::weight_t distance;
    GTFSData::node_t nodeID;

    bool operator>(const QueueElement &other) const
    {
        return this->distance > other.distance;
    }
};

struct PathResult
{
    GTFSData::weight_t total_time;
    std::vector<GTFSData::node_t> path_nodes;
};

PathResult dijkstra(const GTFSData::graph_t &graph, std::size_t start_node, std::size_t goal_node)
{
    // Vector to keep track information about the state
    std::vector<GTFSData::weight_t> d(graph.num_nodes, INF);
    std::vector<GTFSData::node_t> p(graph.num_nodes, std::numeric_limits<GTFSData::node_t>::max());

    // Min Priority Queue
    std::priority_queue<QueueElement, std::vector<QueueElement>, std::greater<QueueElement>> pq{};

    // Init
    d[start_node] = 0;
    pq.push({0, start_node});

    while (!pq.empty())
    {
        QueueElement current = pq.top();
        pq.pop();

        // Early exit
        if (current.nodeID == goal_node)
        {
            std::vector<GTFSData::node_t> path{};
            GTFSData::node_t curr{goal_node};

            while (curr != -1)
            {
                path.push_back(curr);
                curr = p[curr];
            }

            std::reverse(path.begin(), path.end());
            return {d[goal_node], path};
        }

        // Stale data check because using lazy dijkstra
        if (current.distance > d[current.nodeID])
        {
            continue;
        }

        for (std::size_t i = 0, n = graph.degree(current.nodeID); i < n; ++i)
        {
            GTFSData::edge_t edge{graph.adj_edge(current.nodeID, i)};
            GTFSData::node_t neighbor{edge.target};
            GTFSData::weight_t edge_weight{edge.weight};

            GTFSData::weight_t new_dist{current.distance + edge_weight};

            // Relaxation
            if (new_dist < d[neighbor])
            {
                d[neighbor] = new_dist;
                p[neighbor] = current.nodeID;

                // Push to the queue if found a strictly better path
                pq.push({new_dist, neighbor});
            }
        }
    }

    return {INF, {}};
}

void print_path(const PathResult &path)
{
    std::cout << "Path: ";
    for (std::size_t i = 0; i < path.path_nodes.size(); ++i)
    {
        std::cout << path.path_nodes[i];
        if (i < path.path_nodes.size() - 1)
            std::cout << " -> ";
    }
    std::cout << "\nDistance: " << "time" << std::endl;
}
