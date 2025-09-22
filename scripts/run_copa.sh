#!/bin/bash

# Script para executar todas as instâncias da Copa APA
# Uso: ./run_copa_batch.sh

# Configurações
SOLVER="../bin/apa_jpbike"
INSTANCES_DIR="../instances/copa"
OUTPUT_DIR="../results_copa"
SEED=42
CONSTRUCTIVE_METHODS=("nearest" "insertion")

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Executando Solver JP-Bike em todas as instâncias Copa APA ===${NC}"
echo "Diretório de instâncias: $INSTANCES_DIR"
echo "Diretório de resultados: $OUTPUT_DIR"
echo "Semente: $SEED"
echo ""

# Verifica se o solver existe
if [ ! -f "$SOLVER" ]; then
    echo -e "${RED}Erro: Solver não encontrado em $SOLVER${NC}"
    echo "Execute 'make' no diretório src/ primeiro."
    exit 1
fi

# Cria diretório de resultados
mkdir -p "$OUTPUT_DIR"

# Encontra todas as instâncias
instances=($(find "$INSTANCES_DIR" -name "*.txt" | sort))

if [ ${#instances[@]} -eq 0 ]; then
    echo -e "${RED}Erro: Nenhuma instância encontrada em $INSTANCES_DIR${NC}"
    exit 1
fi

echo -e "${GREEN}Encontradas ${#instances[@]} instâncias:${NC}"
for inst in "${instances[@]}"; do
    echo "  - $(basename "$inst")"
done
echo ""

# Arquivo de resumo
SUMMARY_FILE="$OUTPUT_DIR/resumo_execucoes.txt"
echo "# Resumo das Execuções - Copa APA" > "$SUMMARY_FILE"
echo "# Instância | Método | Custo_Construtivo | Custo_VND | Melhoria | Veículos | Tempo_Total" >> "$SUMMARY_FILE"

total_instances=$((${#instances[@]} * ${#CONSTRUCTIVE_METHODS[@]}))
current=0

# Processa cada instância com cada método
for instance in "${instances[@]}"; do
    instance_name=$(basename "$instance" .txt)

    echo -e "${YELLOW}Processando: $instance_name${NC}"

    for method in "${CONSTRUCTIVE_METHODS[@]}"; do
        current=$((current + 1))
        echo -e "  [$current/$total_instances] Método: $method"

        # Executa o solver e captura a saída
        output_file="$OUTPUT_DIR/${instance_name}_${method}.out"
        log_file="$OUTPUT_DIR/${instance_name}_${method}.log"

        start_time=$(date +%s.%3N)

        "$SOLVER" --instance "$instance" \
                  --seed "$SEED" \
                  --constructive "$method" \
                  --out "$OUTPUT_DIR/" > "$log_file" 2>&1

        exit_code=$?
        end_time=$(date +%s.%3N)

        if [ $exit_code -eq 0 ]; then
            # Extrai informações do log
            custo_construtivo=$(grep "Custo construtivo:" "$log_file" | awk '{print $3}' | head -1)
            custo_vnd=$(grep "Custo após VND:" "$log_file" | awk '{print $4}' | head -1)
            melhoria=$(grep "Melhoria total:" "$log_file" | awk '{print $3}' | head -1)
            veiculos=$(grep "Veículos utilizados:" "$log_file" | awk -F'/' '{print $1}' | awk '{print $3}' | head -1)

            # Se não encontrou VND, usa custo final
            if [ -z "$custo_construtivo" ] || [ -z "$custo_vnd" ]; then
                custo_final=$(grep "Custo final:" "$log_file" | awk '{print $3}' | head -1)
                custo_construtivo="$custo_final"
                custo_vnd="$custo_final"
                melhoria="0"
            fi

            tempo_total=$(echo "$end_time - $start_time" | bc)

            # Adiciona ao resumo
            echo "$instance_name | $method | $custo_construtivo | $custo_vnd | $melhoria | $veiculos | ${tempo_total}s" >> "$SUMMARY_FILE"

            echo -e "    ${GREEN}✓${NC} Custo: $custo_construtivo → $custo_vnd (melhoria: $melhoria)"
        else
            echo -e "    ${RED}✗ Erro na execução${NC}"
            echo "$instance_name | $method | ERRO | ERRO | ERRO | ERRO | ERRO" >> "$SUMMARY_FILE"
        fi
    done
    echo ""
done

echo -e "${GREEN}=== Execução Concluída ===${NC}"
echo "Resultados salvos em: $OUTPUT_DIR/"
echo "Resumo disponível em: $SUMMARY_FILE"
echo ""

# Gera estatísticas finais
echo -e "${BLUE}=== Estatísticas Finais ===${NC}"
successful_runs=$(grep -v "ERRO" "$SUMMARY_FILE" | grep -v "^#" | wc -l)
total_runs=$(grep -v "^#" "$SUMMARY_FILE" | wc -l)

echo "Execuções bem-sucedidas: $successful_runs/$total_runs"

if [ $successful_runs -gt 0 ]; then
    echo ""
    echo "Melhor custo por método:"
    for method in "${CONSTRUCTIVE_METHODS[@]}"; do
        best_cost=$(grep "| $method |" "$SUMMARY_FILE" | grep -v "ERRO" | awk -F'|' '{print $4}' | sort -n | head -1 | tr -d ' ')
        best_instance=$(grep "| $method |" "$SUMMARY_FILE" | grep -v "ERRO" | grep "| $best_cost |" | awk -F'|' '{print $1}' | tr -d ' ' | head -1)
        echo "  $method: $best_cost (instância: $best_instance)"
    done

    echo ""
    echo "Melhoria média do VND:"
    for method in "${CONSTRUCTIVE_METHODS[@]}"; do
        avg_improvement=$(grep "| $method |" "$SUMMARY_FILE" | grep -v "ERRO" | awk -F'|' '{sum+=$5; count++} END {if(count>0) print sum/count; else print 0}')
        echo "  $method: $(printf "%.1f" "$avg_improvement")"
    done
fi

echo ""
echo -e "${GREEN}Script finalizado!${NC}"