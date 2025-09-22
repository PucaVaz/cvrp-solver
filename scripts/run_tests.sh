#!/bin/bash

# Script para executar todas as instâncias de teste com cálculo de GAP
# Uso: ./run_teste_batch.sh

# Configurações
SOLVER="../bin/apa_jpbike"
INSTANCES_DIR="../instances/instancias_teste"
OUTPUT_DIR="../results_teste"
SEED=42
CONSTRUCTIVE_METHODS=("nearest" "insertion")

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Função para obter valor de referência
get_reference_value() {
    case "$1" in
        "n12_q20") echo "15700" ;;
        "n12_q30") echo "14600" ;;
        "n13_q30") echo "16900" ;;
        "n14_q12") echo "13500" ;;
        "n14_q30") echo "12600" ;;
        "n17_q10") echo "31443" ;;
        "n17_q20") echo "29259" ;;
        "n20_q20") echo "91619" ;;
        "n20_q30") echo "76999" ;;
        "n26_q20") echo "31100" ;;
        "n26_q30") echo "30300" ;;
        "n40_q20") echo "59493" ;;
        "n40_q30") echo "57476" ;;
        "n54_q30") echo "120277" ;;
        "n58_q30") echo "65669" ;;
        "n74_q20") echo "48829" ;;
        "n79_q30") echo "39979" ;;
        "n81_q10") echo "388680" ;;
        "n115_q20") echo "157115" ;;
        *) echo "" ;;
    esac
}

# Função para obter tipo de valor
get_value_type() {
    case "$1" in
        "n12_q20"|"n12_q30"|"n13_q30"|"n14_q12"|"n14_q30"|"n17_q10"|"n17_q20"|"n20_q20"|"n20_q30"|"n26_q20"|"n26_q30"|"n40_q20"|"n40_q30"|"n58_q30") echo "opt" ;;
        "n54_q30"|"n74_q20"|"n79_q30"|"n81_q10"|"n115_q20") echo "LB" ;;
        *) echo "" ;;
    esac
}

echo -e "${BLUE}=== Executando Solver JP-Bike em todas as instâncias de teste ===${NC}"
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
    instance_name=$(basename "$inst" .txt)
    ref_value=$(get_reference_value "$instance_name")
    value_type=$(get_value_type "$instance_name")
    if [ -n "$ref_value" ]; then
        echo "  - $instance_name (referência: $ref_value [$value_type])"
    else
        echo "  - $instance_name (sem valor de referência)"
    fi
done
echo ""

# Função para calcular GAP
calculate_gap() {
    local obtained=$1
    local reference=$2
    if [ -n "$obtained" ] && [ -n "$reference" ] && [ "$obtained" != "0" ] && [ "$reference" != "0" ]; then
        gap=$(echo "scale=2; ($obtained - $reference) / $reference * 100" | bc)
        echo "$gap"
    else
        echo "N/A"
    fi
}

# Arquivo de resumo
SUMMARY_FILE="$OUTPUT_DIR/resumo_execucoes.txt"
echo "# Resumo das Execuções - Instâncias de Teste" > "$SUMMARY_FILE"
echo "# Instância | Método | Custo_Construtivo | Custo_VND | Melhoria | Veículos | Tempo_Total | Referência | Tipo_Ref | GAP%" >> "$SUMMARY_FILE"

total_instances=$((${#instances[@]} * ${#CONSTRUCTIVE_METHODS[@]}))
current=0

# Processa cada instância com cada método
for instance in "${instances[@]}"; do
    instance_name=$(basename "$instance" .txt)
    ref_value=$(get_reference_value "$instance_name")
    value_type=$(get_value_type "$instance_name")

    echo -e "${YELLOW}Processando: $instance_name${NC}"
    if [ -n "$ref_value" ]; then
        echo -e "  Valor de referência: $ref_value [$value_type]"
    else
        echo -e "  ${RED}Sem valor de referência definido${NC}"
    fi

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

            # Calcula GAP
            gap=$(calculate_gap "$custo_vnd" "$ref_value")

            # Adiciona ao resumo
            ref_display=${ref_value:-"N/A"}
            type_display=${value_type:-"N/A"}
            echo "$instance_name | $method | $custo_construtivo | $custo_vnd | $melhoria | $veiculos | ${tempo_total}s | $ref_display | $type_display | $gap" >> "$SUMMARY_FILE"

            if [ "$gap" != "N/A" ]; then
                echo -e "    ${GREEN}✓${NC} Custo: $custo_construtivo → $custo_vnd (melhoria: $melhoria, GAP: ${gap}%)"
            else
                echo -e "    ${GREEN}✓${NC} Custo: $custo_construtivo → $custo_vnd (melhoria: $melhoria)"
            fi
        else
            echo -e "    ${RED}✗ Erro na execução${NC}"
            ref_display=${ref_value:-"N/A"}
            type_display=${value_type:-"N/A"}
            echo "$instance_name | $method | ERRO | ERRO | ERRO | ERRO | ERRO | $ref_display | $type_display | ERRO" >> "$SUMMARY_FILE"
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

    echo ""
    echo "GAP médio por método (apenas instâncias com referência):"
    for method in "${CONSTRUCTIVE_METHODS[@]}"; do
        avg_gap=$(grep "| $method |" "$SUMMARY_FILE" | grep -v "ERRO" | grep -v "N/A" | awk -F'|' '{if($10 != " N/A") {sum+=$10; count++}} END {if(count>0) printf "%.2f", sum/count; else print "N/A"}')
        count_with_ref=$(grep "| $method |" "$SUMMARY_FILE" | grep -v "ERRO" | grep -v "N/A" | awk -F'|' '{if($10 != " N/A") count++} END {print count+0}')
        echo "  $method: ${avg_gap}% ($count_with_ref instâncias)"
    done

    echo ""
    echo "Melhores GAPs por método:"
    for method in "${CONSTRUCTIVE_METHODS[@]}"; do
        best_gap_line=$(grep "| $method |" "$SUMMARY_FILE" | grep -v "ERRO" | grep -v "N/A" | awk -F'|' '{if($10 != " N/A") print $0}' | sort -t'|' -k10 -n | head -1)
        if [ -n "$best_gap_line" ]; then
            best_gap=$(echo "$best_gap_line" | awk -F'|' '{print $10}' | tr -d ' ')
            best_gap_instance=$(echo "$best_gap_line" | awk -F'|' '{print $1}' | tr -d ' ')
            echo "  $method: ${best_gap}% (instância: $best_gap_instance)"
        else
            echo "  $method: N/A"
        fi
    done
fi

echo ""
echo -e "${GREEN}Script finalizado!${NC}"
