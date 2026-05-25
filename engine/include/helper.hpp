#include <iostream>
#include <cmath>
#include "graph.hpp"

constexpr double EARTH_RADIUS_METERS{6371000.00};
constexpr double METRO_MAX_SPEED{30.00};

inline double deg_to_rad(double deg)
{
    return deg * (M_PI / 180.0);
}

// For the sanity of the author of this helper code, we don't write the collapsed one line of this function
inline double haversine_distance(GTFSData::node_t node_a, GTFSData::node_t node_b)
{
    const auto &a{GTFSData::nodes[node_a]};
    const auto &b{GTFSData::nodes[node_b]};

    double phi1{deg_to_rad(a.lat)};
    double phi2{deg_to_rad(b.lat)};
    double lambda1{deg_to_rad(a.lon)};
    double lambda2{deg_to_rad(b.lon)};

    double delta_phi{phi1 - phi2};
    double delta_lambda{lambda1 - lambda2};

    double a_val{std::sin(delta_phi / 2) * std::sin(delta_phi / 2) + std::cos(phi1) * std::cos(phi2) * std::sin(delta_lambda / 2) * std::sin(delta_lambda / 2)};

    double c{2 * std::atan2(std::sqrt(a_val), std::sqrt(1 - a_val))};

    return EARTH_RADIUS_METERS * c;
}

inline double heuristic_time(GTFSData::node_t node_a, GTFSData::node_t node_b)
{
    return haversine_distance(node_a, node_b) / METRO_MAX_SPEED;
};
