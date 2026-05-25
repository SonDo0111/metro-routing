#include <iostream>
#include <limits>
#include <vector>
#include <queue>
#include <string_view>
#include <cstddef>
#include <algorithm>
#include "graph.hpp"

enum class RoutingProfile
{
    COMMUTER = 0,        // "Return the fastest itinerary"
    BALANCED = 300,      // "Return the balanced itinerary, it's equivalent to if transfer(transfer + wait) take less than 5 mins then it's acceptable" (Default)
    TOURIST = 900,       // "Return the slower itinerary but less transfer"
    NO_TRANSFERS = 99999 // "Retunrn the itinerary without transfer unless it is literally the only way."
};

constexpr GTFSData::weight_t INF = std::numeric_limits<GTFSData::weight_t>::max() / 2;

struct QueueElement
{
    GTFSData::weight_t estimated_time_taken;
    GTFSData::weight_t time_taken;
    GTFSData::node_t nodeID;

    bool operator>(const QueueElement &other) const
    {
        return this->estimated_time_taken > other.estimated_time_taken;
    }
};

struct PathResult
{
    GTFSData::weight_t total_time;
    std::vector<GTFSData::node_t> path_nodes;
};

std::size_t transfer_count(const std::vector<GTFSData::node_t> &path);

PathResult a_star(const GTFSData::graph_t &graph,
                  GTFSData::node_t start_node, GTFSData::node_t goal_node,
                  GTFSData::weight_t transfer_penalty = static_cast<int>(RoutingProfile::BALANCED));

void export_to_geojson(const PathResult &result);
