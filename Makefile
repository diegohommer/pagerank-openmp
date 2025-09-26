# Makefile simples para PageRank com OpenMP
CXX = g++
CXXFLAGS = -std=c++17 -O3 -fopenmp
TARGET = pagerank
SOURCE = pagerank.cpp

# Target padrão
all: $(TARGET)

# Compilar o programa
$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

# Limpar arquivos compilados
clean:
	rm -f $(TARGET)
