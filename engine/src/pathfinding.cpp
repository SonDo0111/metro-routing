#include <iostream>
#include <limits>
#include <vector>
#include <array>
#include <queue>
#include <string_view>
#include <cstddef>
#include <utility>
#include <algorithm>
#include <chrono>
#include "../include/graph.hpp"
#include "../include/pathfinding.hpp"
#include "../include/helper.hpp"

std::size_t transfer_count(const std::vector<GTFSData::node_t> &path)
{
    std::size_t transfer_count{0};
    if (path.size() > 1)
    {
        std::string_view current_route = GTFSData::nodes[path[0]].route;
        for (std::size_t i{0}; i < path.size(); ++i)
        {
            std::string_view next_route = GTFSData::nodes[path[i]].route;
            if (current_route != next_route)
            {
                ++transfer_count;
                current_route = next_route;
            }
        }
    }
    return transfer_count;
}

std::vector<GTFSData::node_t> get_platforms_from_station(std::string_view parent_id)
{
    std::vector<GTFSData::node_t> platforms;
    // O(N) linear search on 804 items is virtually instantaneous in C++
    for (std::size_t i = 0; i < GTFSData::NUM_NODES; ++i)
    {
        if (GTFSData::nodes[i].parent_station == parent_id)
        {
            platforms.push_back(i);
        }
    }
    return platforms;
}

PathResult a_star(const GTFSData::graph_t &graph, GTFSData::node_t start_node,
                  GTFSData::node_t goal_node,
                  GTFSData::weight_t transfer_penalty)
{
    // Vector to keep track information about the state
    // This tell us about how many seconds we need to travel to the node
    std::vector<GTFSData::weight_t> time_taken_to(graph.num_nodes, INF);
    // This tell us the node previous to the node
    std::vector<GTFSData::node_t> p(graph.num_nodes, std::numeric_limits<GTFSData::node_t>::max());

    // Min Priority Queue
    std::priority_queue<QueueElement, std::vector<QueueElement>, std::greater<QueueElement>> pq{};

    // Init
    time_taken_to[start_node] = 0;
    pq.push({0, 0, start_node});

    while (!pq.empty())
    {
        QueueElement current = pq.top();
        pq.pop();

        // Early exit
        if (current.nodeID == goal_node)
        {
            std::vector<GTFSData::node_t> path{};

            // Construct path from p vector
            GTFSData::node_t current{goal_node};

            while (current != std::numeric_limits<GTFSData::node_t>::max())
            {
                path.push_back(current);
                current = p[current];
            }

            std::reverse(path.begin(), path.end());
            // Note: The return is true time estimated in second but not the cost used in pq
            return {time_taken_to[goal_node] - transfer_count(path) * transfer_penalty, path};
        }

        // Stale data check because using lazy dijkstra
        if (current.time_taken > time_taken_to[current.nodeID])
        {
            continue;
        }

        for (std::size_t i = 0, n = graph.degree(current.nodeID); i < n; ++i)
        {
            GTFSData::weight_t penalty = 0;
            GTFSData::edge_t edge{graph.adj_edge(current.nodeID, i)};
            GTFSData::node_t neighbor{edge.target};
            GTFSData::weight_t edge_weight{edge.weight};

            if (GTFSData::nodes[current.nodeID].route != GTFSData::nodes[neighbor].route)
            {
                penalty = transfer_penalty;
            }

            GTFSData::weight_t new_time_taken{current.time_taken + edge_weight + penalty};

            // Relaxation
            if (new_time_taken < time_taken_to[neighbor])
            {
                time_taken_to[neighbor] = new_time_taken;
                p[neighbor] = current.nodeID;

                GTFSData::weight_t heuristic{static_cast<GTFSData::weight_t>(heuristic_time(neighbor, goal_node))};
                // Push to the queue if found a strictly better path
                pq.push({new_time_taken + heuristic, new_time_taken, neighbor});
            }
        }
    }

    return {INF, {}};
}

PathResult a_star(const GTFSData::graph_t &graph, std::vector<GTFSData::node_t> start_nodes,
                  std::string_view goal_station,
                  GTFSData::node_t representative_goal_node,
                  GTFSData::weight_t transfer_penalty)
{
    // Vector to keep track information about the state
    // This tell us about how many seconds we need to travel to the node
    std::vector<GTFSData::weight_t> time_taken_to(graph.num_nodes, INF);
    // This tell us the node previous to the node
    std::vector<GTFSData::node_t> p(graph.num_nodes, std::numeric_limits<GTFSData::node_t>::max());

    // Min Priority Queue
    std::priority_queue<QueueElement, std::vector<QueueElement>, std::greater<QueueElement>> pq{};

    // Init
    for (GTFSData::node_t start_node : start_nodes)
    {
        time_taken_to[start_node] = 0;

        // h(n) uses the representative goal coordinate
        GTFSData::weight_t heuristic = static_cast<GTFSData::weight_t>(
            heuristic_time(start_node, representative_goal_node));

        pq.push({heuristic, 0, start_node});
    }

    while (!pq.empty())
    {
        QueueElement current = pq.top();
        pq.pop();

        // Early exit
        if (GTFSData::nodes[current.nodeID].parent_station == goal_station)
        {
            std::vector<GTFSData::node_t> path{};
            GTFSData::node_t current_node{current.nodeID};

            // Construct path from p vector
            while (current_node != std::numeric_limits<GTFSData::node_t>::max())
            {
                path.push_back(current_node);
                current_node = p[current_node];
            }

            std::reverse(path.begin(), path.end());
            // Note: The return is true time estimated in second but not the cost used in pq
            return {time_taken_to[current.nodeID] - transfer_count(path) * transfer_penalty, path};
        }

        // Stale data check because using lazy dijkstra
        if (current.time_taken > time_taken_to[current.nodeID])
        {
            continue;
        }

        for (std::size_t i = 0, n = graph.degree(current.nodeID); i < n; ++i)
        {
            GTFSData::weight_t penalty = 0;
            GTFSData::edge_t edge{graph.adj_edge(current.nodeID, i)};
            GTFSData::node_t neighbor{edge.target};
            GTFSData::weight_t edge_weight{edge.weight};

            if (GTFSData::nodes[current.nodeID].route != GTFSData::nodes[neighbor].route)
            {
                penalty = transfer_penalty;
            }

            GTFSData::weight_t new_time_taken{current.time_taken + edge_weight + penalty};

            // Relaxation
            if (new_time_taken < time_taken_to[neighbor])
            {
                time_taken_to[neighbor] = new_time_taken;
                p[neighbor] = current.nodeID;

                GTFSData::weight_t heuristic{static_cast<GTFSData::weight_t>(heuristic_time(neighbor, representative_goal_node))};
                // Push to the queue if found a strictly better path
                pq.push({new_time_taken + heuristic, new_time_taken, neighbor});
            }
        }
    }

    return {INF, {}};
}

void export_to_geojson(const PathResult &result)
{
    if (result.path_nodes.empty())
    {
        std::cout << "{ \"error\": \"No path found\" }\n";
        return;
    }

    std::cout << "{\n";
    std::cout << "  \"type\": \"FeatureCollection\",\n";
    std::cout << "  \"features\": [\n";

    // --- 1. THE ROUTE LINE (LineString) ---
    std::cout << "    {\n";
    std::cout << "      \"type\": \"Feature\",\n";
    std::cout << "      \"properties\": {\n";
    std::cout << "        \"stroke\": \"#0055FF\",\n"; // Blue line
    std::cout << "        \"stroke-width\": 4\n";
    std::cout << "      },\n";
    std::cout << "      \"geometry\": {\n";
    std::cout << "        \"type\": \"LineString\",\n";
    std::cout << "        \"coordinates\": [\n";

    for (std::size_t i = 0; i < result.path_nodes.size(); ++i)
    {
        const auto &node = GTFSData::nodes[result.path_nodes[i]];
        // WARNING: GeoJSON requires [Longitude, Latitude]
        std::cout << "          [" << node.lon << ", " << node.lat << "]";
        if (i < result.path_nodes.size() - 1)
            std::cout << ",";
        std::cout << "\n";
    }

    std::cout << "        ]\n";
    std::cout << "      }\n";
    std::cout << "    }";

    // --- 2. THE STATIONS (Points) ---
    for (std::size_t i = 0; i < result.path_nodes.size(); ++i)
    {
        std::cout << ",\n"; // Comma to separate features
        const auto &node = GTFSData::nodes[result.path_nodes[i]];

        std::cout << "    {\n";
        std::cout << "      \"type\": \"Feature\",\n";
        std::cout << "      \"properties\": {\n";
        std::cout << "        \"marker-color\": \"#FF0000\",\n"; // Red dots for stations
        std::cout << "        \"name\": \"" << node.stop_name << "\",\n";
        std::cout << "        \"line\": \"" << node.route << "\"\n";
        std::cout << "      },\n";
        std::cout << "      \"geometry\": {\n";
        std::cout << "        \"type\": \"Point\",\n";
        std::cout << "        \"coordinates\": [" << node.lon << ", " << node.lat << "]\n";
        std::cout << "      }\n";
        std::cout << "    }";
    }

    std::cout << "\n  ]\n";
    std::cout << "}\n";
}
