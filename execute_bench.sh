make clean
make 
python3 scripts/build_commands.py
echo "Executando todos os benchmarks..."
eval ./scripts/commands.sh