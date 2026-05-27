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

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;

    std::string command = argv[1];

    if (command == "route" && argc == 4)
    {
        GTFSData::node_t start = std::stoull(argv[2]);
        GTFSData::node_t goal = std::stoull(argv[3]);

        PathResult result = a_star(GTFSData::graph, start, goal, 300);
        export_to_geojson(result);
        return 0;
    }
    else if (command == "nearest" && argc == 4)
    {
        double lat = std::stod(argv[2]);
        double lon = std::stod(argv[3]);

        GTFSData::node_t nearest = find_nearest_station(lat, lon);

        // Output just the ID as a simple JSON for Python to read
        std::cout << "{ \"nearest_node_id\": " << nearest << " }\n";
        return 0;
    }

    std::cerr << "Invalid command.\n";
    return 1;
}
