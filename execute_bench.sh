#!/bin/bash
source /home/intel/oneapi/vtune/2021.1.1/vtune-vars.sh

make clean
make 
python3 scripts/build_commands.py


echo "Executando todos os benchmarks..."
eval ./scripts/commands.sh