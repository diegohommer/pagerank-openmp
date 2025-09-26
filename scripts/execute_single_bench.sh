#!/usr/bin/env bash
set -euo pipefail

# Dependências básicas
need_cmd() { command -v "$1" >/dev/null 2>&1 || { echo "Missing dependency: $1"; exit 1; }; }
need_cmd wget
need_cmd awk
if ! command -v gunzip >/dev/null 2>&1; then need_cmd gzip; fi
need_cmd make
need_cmd tee
need_cmd tr
need_cmd date


script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(cd "$script_dir/.." && pwd)"
data_dir="$project_root/data"
results_dir="$project_root/results"


# Vars para o script (valores padrão)
THREADS=1
GRAPH_NAME=''
GRAPH_URL=''
ANALYSIS_TYPE='performance-snapshot'

# Função para mostrar ajuda
show_help() {
    echo "Uso: $0 [opções]"
    echo "Opções:"
    echo "  -threads N        Número de threads (padrão: 1)"
    echo "  -graph-name NAME  Nome do grafo (obrigatório)"
    echo "  -graph-url URL    URL do grafo"
    echo "  -h, --help        Mostra esta ajuda"
}

# Função para validar parâmetros obrigatórios
validate_required_params() {
    local errors=0
    
    if [[ -z "$GRAPH_NAME" ]]; then
        echo "Erro: -graph-name é obrigatório"
        errors=1
    fi
    
    if [[ $errors -eq 1 ]]; then
        echo ""
        show_help
        exit 1
    fi
}

# Função para mostrar parâmetros parseados
show_parsed_params() {
    echo "[INFO] Parâmetros configurados:"
    echo "[INFO] THREADS=$THREADS"
    echo "[INFO] GRAPH_NAME=$GRAPH_NAME"
    echo "[INFO] GRAPH_URL=$GRAPH_URL"
    echo ""
}

# Função principal para parse dos argumentos
parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -analysis-type)
                if [[ -z "$2" || "$2" =~ ^- ]]; then
                    echo "Erro: -analysis-type requer um valor"
                    exit 1
                fi
                ANALYSIS_TYPE="$2"
                shift 2
                ;;
            -threads)
                if [[ -z "$2" || "$2" =~ ^- ]]; then
                    echo "Erro: -threads requer um valor"
                    exit 1
                fi
                THREADS="$2"
                shift 2
                ;;


            -graph-name)
                if [[ -z "$2" || "$2" =~ ^- ]]; then
                    echo "Erro: -graph-name requer um valor"
                    exit 1
                fi
                GRAPH_NAME="$2"
                shift 2
                ;;
            -graph-url)
                if [[ -z "$2" || "$2" =~ ^- ]]; then
                    echo "Erro: -graph-url requer um valor"
                    exit 1
                fi
                GRAPH_URL="$2"
                shift 2
                ;;


            -h|--help)
                show_help
                exit 0
                ;;
            *)
                echo "Parâmetro desconhecido: $1"
                echo "Use -h ou --help para ver as opções disponíveis"
                exit 1
                ;;
        esac
    done
}

get_graph_data() {
  # Pastas por grafo/kernel
  graph_dir="$data_dir/$GRAPH_NAME"
  mkdir -p "$graph_dir"
  

  # Baixar arquivo original (usa o nome do recurso da URL)
  download_path="$graph_dir/$(basename "$GRAPH_URL")"
  
  # Determina arquivo texto (descompacta se for .gz)
  text_path="$download_path"
  if [[ "$download_path" == *.gz ]]; then
    text_path="${download_path%.gz}"
  fi
  
  # Verifica se o arquivo .txt final já existe
  if [[ -s "$text_path" ]]; then
    echo "[INFO] Arquivo já existe: $text_path"
  else
    # Baixa o arquivo se não existir
    if [[ ! -s "$download_path" ]]; then
      echo "[INFO] Baixando ($GRAPH_NAME): $GRAPH_URL"
      wget -O "$download_path" "$GRAPH_URL"
    fi
    
    # Descompacta se for .gz
    if [[ "$download_path" == *.gz ]]; then
      if [[ ! -s "$text_path" ]]; then
        echo "[INFO] Descompactando -> $text_path"
        if command -v gunzip >/dev/null 2>&1; then
          gunzip -c "$download_path" > "$text_path"
        else
          gzip -dc "$download_path" > "$text_path"
        fi
      fi
    fi
  fi
}

# Executa o parse dos argumentos
parse_arguments "$@"

# Valida parâmetros obrigatórios
validate_required_params

# Mostra os parâmetros parseados
show_parsed_params

mkdir -p "$data_dir"
mkdir -p "$results_dir"
mkdir -p "$results_dir/"$ANALYSIS_TYPE"_"$GRAPH_NAME"_"$THREADS

results_dir="$results_dir/$ANALYSIS_TYPE"_"$GRAPH_NAME"_"$THREADS"

# Cria os dados do grafo
get_graph_data

# Executa kernel

export OMP_NUM_THREADS="$THREADS"


# Para rodar só o pagrank
# "$project_root/pagerank" -f "$text_path" -result-dir "$results_dir"


vtune -collect $ANALYSIS_TYPE -- "$project_root/pagerank" -f "$text_path" -result-dir "$results_dir"
# Gera o relatório
vtune -report summary \
  -result-dir "$results_dir" \
  -format csv \
  -report-output "$results_dir/report.csv"

  

