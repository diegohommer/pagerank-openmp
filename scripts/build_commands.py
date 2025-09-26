# This module creates the build command for the benchmark
# 1. Reads the CSV file
# 2. Creates the build command for the benchmark
# 3. Creates a file with the commands

import csv

csv_file = "./experiments.csv"

def create_build_commands(csv_file):
    commands = []
    with open(csv_file, 'r') as file:
        reader = csv.reader(file)
        header = next(reader)
        for row in reader:
            graph_name = row[header.index("GRAPH_NAME")]
            graph_url = row[header.index("GRAPH_URL")]
            threads = row[header.index("THREADS")]
            analysis_type = row[header.index("ANALYSIS_TYPE")]
            command = f"./scripts/execute_single_bench.sh -graph-name {graph_name} -graph-url {graph_url} -threads {threads} -analysis-type {analysis_type}"
            commands.append(command)
    return commands


if __name__ == "__main__":
    commands = create_build_commands(csv_file)
    commands_formatted = ' && \n'.join(commands)
    
    # Cria arquivo com os comandos formatados
    with open('./scripts/commands.sh', 'w') as f:
        f.write(commands_formatted)
    

