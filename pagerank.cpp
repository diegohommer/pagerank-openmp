#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main(int argc, char const* argv[])
{
    std::vector<int> outgoing_links;
    std::vector<std::vector<int>> page_graph_t;
    int num_pages = 0;

    /******************* INPUT HANDLING + NUM OF NODES *******************/

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
                outgoing_links.resize(num_pages);
                page_graph_t.resize(num_pages);
            }
            continue;
        }

        // Read edge u->v and insert reverse edge into transposed page graph
        int u, v;
        if (!(ss >> u >> v)) continue;

        outgoing_links[u]++;
        page_graph_t[v].push_back(u);
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

        // compute contributions from all nodes
        for (int i = 0; i < num_pages; ++i) {
            if (outgoing_links[i] == 0) {
                dangling_sum += page_rank[i];
                continue;
            }

            double share = damping_factor * page_rank[i] / outgoing_links[i];
            for (int ref : page_graph_t[i]) {
                new_page_rank[ref] += share;
            }
        }

        // redistribute dangling mass equally to all nodes
        double dangling_contrib = damping_factor * dangling_sum / num_pages;
        for (int i = 0; i < num_pages; ++i) {
            new_page_rank[i] += dangling_contrib;
        }

        double diff = 0.0;
        for (int i = 0; i < num_pages; ++i) {
            diff += std::fabs(new_page_rank[i] - page_rank[i]);
        }
        if (diff < 1e-6) break;

        page_rank.swap(new_page_rank);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    /*************************** OUTPUT RESULTS **************************/

    std::cout << "PageRank converged in " << iterations << " iterations.\n";
    std::cout << "Final PageRank values:\n[";
    for (int i = 0; i < num_pages; ++i) {
        std::cout << page_rank[i];
        if (i != num_pages - 1) std::cout << ", ";
    }
    std::cout << "]\n";
    std::cout << "Time elapsed: " << elapsed.count() << " seconds.\n";

    return 0;
}
