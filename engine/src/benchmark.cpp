#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <random>
#include "../include/graph.hpp"
#include "../include/pathfinding.hpp"

// Biến volatile để chống trình biên dịch tối ưu hóa (Dead Code Elimination)
volatile double anti_optimization_sum = 0.0;

void run_all_pairs_benchmark()
{

    std::cout << "1. Bat dau Warm-up CPU Cache..." << std::endl;
    // Sử dụng bộ sinh số ngẫu nhiên chuẩn C++11
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::size_t> dist(0, GTFSData::NUM_NODES - 1);

    for (std::size_t i = 0; i < 10000; ++i)
    {
        std::size_t u = dist(gen);
        std::size_t v = dist(gen);
        if (u == v)
            continue;

        PathResult res_d = dijkstra(GTFSData::graph, u, v, 300);
        PathResult res_a = a_star(GTFSData::graph, u, v, 300);
        anti_optimization_sum += static_cast<double>(res_d.total_time + res_a.total_time);
    }
    std::cout << "=> Warm-up hoan tat!" << std::endl;

    std::size_t total_pairs = GTFSData::NUM_NODES * (GTFSData::NUM_NODES - 1);
    std::cout << "[2/2] Bat dau All-Pairs Benchmark (" << total_pairs << " cap)..." << std::endl;

    std::ofstream out("benchmark_results.csv");
    out << "source,target,dijkstra_time_ns,dijkstra_nodes,astar_time_ns,astar_nodes\n";

    for (std::size_t u = 0; u < GTFSData::NUM_NODES; ++u)
    {
        for (std::size_t v = 0; v < GTFSData::NUM_NODES; ++v)
        {
            if (u == v)
                continue;

            // --- ĐO DIJKSTRA ---
            auto start_d = std::chrono::high_resolution_clock::now();
            PathResult res_d = dijkstra(GTFSData::graph, u, v, static_cast<GTFSData::weight_t>(RoutingProfile::BALANCED));
            auto end_d = std::chrono::high_resolution_clock::now();
            long long time_d = std::chrono::duration_cast<std::chrono::nanoseconds>(end_d - start_d).count();

            // --- ĐO A* ---
            auto start_a = std::chrono::high_resolution_clock::now();
            PathResult res_a = a_star(GTFSData::graph, u, v, static_cast<GTFSData::weight_t>(RoutingProfile::BALANCED));
            auto end_a = std::chrono::high_resolution_clock::now();
            long long time_a = std::chrono::duration_cast<std::chrono::nanoseconds>(end_a - start_a).count();

            anti_optimization_sum += static_cast<double>(res_d.total_time + res_a.total_time);

            out << u << "," << v << ","
                << time_d << "," << res_d.node_expanded << ","
                << time_a << "," << res_a.node_expanded << "\n";
        }
    }

    out.close();
    std::cout << "=> Benchmark hoan tat! Da luu vao benchmark_results.csv" << std::endl;
}
