#!/bin/bash

# Script simples para rodar todas as instâncias Copa APA
echo "=== Executando Solver nas instâncias Copa APA ==="

# Run from repo root
cd "$(dirname "$0")/.."

# Cria diretório de resultados
mkdir -p results_copa

# Instâncias Copa APA
instances=("instancia1.txt" "instancia2.txt" "instancia3.txt")
methods=("nearest" "insertion")

for instance in "${instances[@]}"; do
    echo ""
    echo "Processando: $instance"

    for method in "${methods[@]}"; do
        echo "  Método: $method"

        # Executa o solver
        ./bin/apa_jpbike \
            --instance "instances/copa/$instance" \
            --seed 42 \
            --constructive "$method" \
            --out "results_copa/" \
            > "results_copa/${instance%.*}_${method}.log"

        # Mostra resultado rápido
        custo=$(tail -10 "results_copa/${instance%.*}_${method}.log" | grep "Custo.*VND:" | awk '{print $4}')
        if [ -z "$custo" ]; then
            custo=$(tail -10 "results_copa/${instance%.*}_${method}.log" | grep "Custo final:" | awk '{print $3}')
        fi
        echo "    Custo final: $custo"
    done
done

echo ""
echo "=== Concluído! ==="
echo "Resultados em: results_copa/"
echo "Soluções (.out) e logs (.log) salvos."