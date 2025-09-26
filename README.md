# pagerank-openmp 
Intel Vtune necessário gulizada
## Overview

This project implements the PageRank algorithm in C++ with OpenMP for parallelization. It reads a directed graph from standard input, computes PageRank scores iteratively, and outputs the results along with runtime and convergence information.

## Directory Structure

```Folder
data/           # Example graph datasets in edge list format
pagerank.cpp    # OpenMP PageRank implementation
```

## Features
- **Parallel PageRank**
  - Uses OpenMP to distribute computation across multiple threads.
  - The number of threads can be set as a command-line argument (defaults to the maximum available):
    ```sh
    ./pagerank 4 < data/example_graph.txt
    ```

- **Graph Input Format**
  - Accepts edge list input from standard input.
  - Lines starting with `#` are ignored except the "Nodes/Edges" line.
  - Example format:
    ```
    # Nodes: 4 Edges: 5
    0 1
    0 2
    1 2
    2 0
    3 2
    ```

- **Outputs**
  - Converged PageRank values for all nodes.
  - Number of iterations until convergence.
  - Total execution time in seconds.

- **Convergence Criteria**
  - Iterates until the sum of absolute differences between consecutive PageRank vectors is less than `1e-6`.

## Build Instructions
Compile with GCC and OpenMP support:
```sh
g++ -std=c++17 -fopenmp -O2 pagerank.cpp -o pagerank
```

## @Dependencies

- GCC with C++17 support
- OpenMP

## References
- [The PageRank Citation Ranking: Bringing Order to the Web](http://ilpubs.stanford.edu:8090/422/1/1999-66.pdf) – Original PageRank paper by Page & Brin
- [The Google PageRank Algorithm](https://web.stanford.edu/class/cs54n/handouts/24-GooglePageRankAlgorithm.pdf) – Stanford CS54N
- [PageRank on Wikipedia](https://en.wikipedia.org/wiki/PageRank)
- [Stanford Network Analysis Project (SNAP)](https://snap.stanford.edu/data/) – Datasets for graph analysis

## Authors

Diego Hommerding Amorim [GitHub](https://github.com/diegohommer) • [Email](mailto:dieghommeramorim@gmail.com)

Developed as part of an academic project for the **Distributed and Parallel Programming** course at **UFRGS**.
