#include <omp.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main(int argc, char const* argv[])
{
    std::vector<std::vector<int>> page_graph;
    int num_pages = 0;

    /******************* INPUT HANDLING + NUM OF NODES *******************/
    int num_threads = omp_get_max_threads();
    if (argc > 1) {
        num_threads = std::stoi(argv[1]);
        if (num_threads <= 0) {
            std::cerr << "Number of threads must be positive.\n";
            return 1;
        }
    }
    omp_set_num_threads(num_threads);
    std::cout << "Using " << num_threads << " threads.\n";

    std::string line;
    while (std::getline(std::cin, line)) {
        std::stringstream ss(line);

        // Skip comments except the "Nodes/Edges" line
        if (line.rfind("#", 0) == 0) {
            std::string tag;
            ss >> tag;

            if (line.find("Nodes:") != std::string::npos) {
                std::string tmp;
                ss >> tmp >> num_pages >> tmp >> tmp;  // "# Nodes: N Edges: M"
                page_graph.resize(num_pages);
            }
            continue;
        }

        // Read edge u->v and insert reverse edge into transposed page graph
        int u, v;
        if (!(ss >> u >> v)) continue;

        page_graph[u].push_back(v);
    }

    /*************************** PageRank LOOP  **************************/

    double damping_factor = 0.85;
    std::vector<double> page_rank(num_pages, 1.0 / num_pages);
    std::vector<double> new_page_rank(num_pages, 0.0);

    int iterations = 0;

    auto start_time = std::chrono::high_resolution_clock::now();

    while (true) {
        iterations++;

        std::fill(new_page_rank.begin(), new_page_rank.end(), (1.0 - damping_factor) / num_pages);
        double dangling_sum = 0.0;

        // Compute contributions from all nodes
        #pragma omp parallel for reduction(+:dangling_sum) schedule(static)
        for (int i = 0; i < num_pages; ++i) {
            int outdeg = page_graph[i].size();
            if (outdeg == 0) {
                dangling_sum += page_rank[i];
                continue;
            }

            double share = damping_factor * page_rank[i] / outdeg;
            for (int ref : page_graph[i]) {
                #pragma omp atomic
                new_page_rank[ref] += share;
            }
        }

        // Redistribute dangling mass equally to all nodes
        double dangling_contrib = damping_factor * dangling_sum / num_pages;
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < num_pages; ++i) {
            new_page_rank[i] += dangling_contrib;
        }

        double diff = 0.0;
        #pragma omp parallel for reduction(+:diff) schedule(static)
        for (int i = 0; i < num_pages; ++i) {
            diff += std::fabs(new_page_rank[i] - page_rank[i]);
        }
        if (diff < 1e-6) break;

        page_rank.swap(new_page_rank);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    /*************************** OUTPUT RESULTS **************************/

    std::cout << "PageRank converged in " << iterations << " iterations." << std::endl;
    std::cout << "Final PageRank values:\n";
    for (int i = 0; i < num_pages; ++i) {
        std::cout << std::fixed << std::setprecision(6) << page_rank[i] << std::endl;
    }
    std::cout << "Time elapsed: " << elapsed.count() << " seconds." << std::endl;

    return 0;
}
