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

int node_expanded{0};

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

PathResult dijkstra(const GTFSData::graph_t &graph, GTFSData::node_t start_node,
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
        node_expanded++;

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

void print_itinerary(const PathResult &result)
{
    if (result.total_time == INF)
    {
        std::cout << "No path found!\n";
        return;
    }

    std::cout << "========================================\n";
    std::cout << "🚇 TRIP ITINERARY\n";
    std::cout << "⏱️  Estimated Time: " << result.total_time / 60 << " minutes\n";
    std::cout << "========================================\n";

    std::string_view current_line = "";

    for (std::size_t i = 0; i < result.path_nodes.size(); ++i)
    {
        GTFSData::node_t node_id = result.path_nodes[i];

        // Look up the node data from your generated .hpp file!
        const auto &node_data = GTFSData::nodes[node_id];

        // Transfer Detection Logic
        if (node_data.route != current_line)
        {
            if (i != 0)
            {
                std::cout << "  🚶 [TRANSFER] \n";
            }
            std::cout << "🟢 BOARD LINE: " << node_data.route << '\n';
            current_line = node_data.route;
        }

        std::cout << node_id << "   📍 " << node_data.stop_name << '\n';
    }
    std::cout << "========================================\n";
    std::cout << "🏁 ARRIVED AT DESTINATION\n";
}

int main()
{
    GTFSData::node_t start = 0; // Barbara
    GTFSData::node_t goal = 90; // Malesherbes
    GTFSData::weight_t penalty = 300;

    // --- Start the Clock ---
    auto start_time = std::chrono::high_resolution_clock::now();

    // Run the algorithm
    PathResult result = dijkstra(GTFSData::graph, start, goal, penalty);

    // --- Stop the Clock ---
    auto end_time = std::chrono::high_resolution_clock::now();

    // Calculate duration in microseconds
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

    std::cout << "Algorithm finished in: " << duration << " microseconds.\n";

    // print_itinerary(result); // Don't include printing in the benchmark!
    return 0;
}
