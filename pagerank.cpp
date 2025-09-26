#include <omp.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main(int argc, char const* argv[])
{
    std::string filename;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-f" && i + 1 < argc) {
            filename = argv[i + 1];
            i++; // Skip next argument as it's the filename
        }
    }
    
    if (filename.empty()) {
        std::cerr << "Usage: " << argv[0] << " -f <graph_file>" << std::endl;
        return 1;
    }
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return 1;
    }

    std::vector<std::vector<int>> page_graph;
    int num_pages = 0;

    /******************* INPUT HANDLING + NUM OF NODES *******************/

    std::string line;
    while (std::getline(file, line)) {
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

        // Verificar se os índices estão dentro dos limites
        if (u >= 0 && u < num_pages && v >= 0 && v < num_pages) {
            page_graph[u].push_back(v);
        }
    }
    
    file.close();

    // Verificar se o número de páginas foi configurado
    if (num_pages <= 0) {
        std::cerr << "Erro: Número de páginas não foi encontrado no arquivo ou é inválido." << std::endl;
        std::cerr << "Certifique-se de que o arquivo contém uma linha como '# Nodes: N Edges: M'" << std::endl;
        return 1;
    }

    std::cout << "Número de páginas: " << num_pages << std::endl;

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

    std::cout << "Time elapsed: " << elapsed.count() << " seconds." << std::endl;

    return 0;
}
