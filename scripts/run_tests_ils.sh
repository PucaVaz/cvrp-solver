
SOLVER="../bin/apa_jpbike"
INSTANCES_DIR="../instances/instancias_teste"
OUTPUT_DIR="../results_teste_ils"
SEED=24112004

# Parâmetros ILS
MAX_ITER=50
MAX_ITER_ILS=150
ALPHA_MIN=0.1
ALPHA_MAX=0.5
PERTURB_STRENGTH=2

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

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

get_value_type() {
    case "$1" in
        "n12_q20"|"n12_q30"|"n13_q30"|"n14_q12"|"n14_q30"|"n17_q10"|"n17_q20"|"n20_q20"|"n20_q30"|"n26_q20"|"n26_q30"|"n40_q20"|"n40_q30"|"n58_q30") echo "opt" ;;
        "n54_q30"|"n74_q20"|"n79_q30"|"n81_q10"|"n115_q20") echo "LB" ;;
        *) echo "" ;;
    esac
}

echo -e "${BLUE}=== Executando ILS em todas as instâncias de teste ===${NC}"
echo "Diretório de instâncias: $INSTANCES_DIR"
echo "Diretório de resultados: $OUTPUT_DIR"
echo "Semente: $SEED"
echo "Parâmetros ILS: max_iter=$MAX_ITER, max_iter_ils=$MAX_ITER_ILS"
echo "Alpha range: [$ALPHA_MIN, $ALPHA_MAX], perturbation: $PERTURB_STRENGTH"
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
SUMMARY_FILE="$OUTPUT_DIR/resumo_execucoes_ils.txt"
echo "# Resumo das Execuções ILS - Instâncias de Teste" > "$SUMMARY_FILE"
echo "# Instância | Método | Custo_Final | Veículos | Tempo_Total | Referência | Tipo_Ref | GAP%" >> "$SUMMARY_FILE"

total_instances=${#instances[@]}
current=0

# Processa cada instância com ILS
for instance in "${instances[@]}"; do
    instance_name=$(basename "$instance" .txt)
    ref_value=$(get_reference_value "$instance_name")
    value_type=$(get_value_type "$instance_name")

    current=$((current + 1))
    echo -e "${YELLOW}[$current/$total_instances] Processando: $instance_name${NC}"
    if [ -n "$ref_value" ]; then
        echo -e "  Valor de referência: $ref_value [$value_type]"
    else
        echo -e "  ${RED}Sem valor de referência definido${NC}"
    fi

    # Executa o solver ILS e captura a saída
    output_file="$OUTPUT_DIR/${instance_name}_ils.out"
    log_file="$OUTPUT_DIR/${instance_name}_ils.log"

    start_time=$(date +%s.%3N)

    "$SOLVER" --instance "$instance" \
              --seed "$SEED" \
              --ils \
              --max-iter "$MAX_ITER" \
              --max-iter-ils "$MAX_ITER_ILS" \
              --rcl-alpha-min "$ALPHA_MIN" \
              --rcl-alpha-max "$ALPHA_MAX" \
              --perturb-strength "$PERTURB_STRENGTH" \
              --out "$OUTPUT_DIR/" > "$log_file" 2>&1

    exit_code=$?
    end_time=$(date +%s.%3N)

    if [ $exit_code -eq 0 ]; then
        # Extrai informações do log
        custo_final=$(grep "Custo final:" "$log_file" | awk '{print $3}' | head -1)
        veiculos=$(grep "Veículos utilizados:" "$log_file" | awk -F'/' '{print $1}' | awk '{print $3}' | head -1)

        tempo_total=$(echo "$end_time - $start_time" | bc)

        # Calcula GAP
        gap=$(calculate_gap "$custo_final" "$ref_value")

        # Adiciona ao resumo
        ref_display=${ref_value:-"N/A"}
        type_display=${value_type:-"N/A"}
        echo "$instance_name | ILS | $custo_final | $veiculos | ${tempo_total}s | $ref_display | $type_display | $gap" >> "$SUMMARY_FILE"

        if [ "$gap" != "N/A" ]; then
            echo -e "    ${GREEN}✓${NC} Custo ILS: $custo_final (GAP: ${gap}%)"
        else
            echo -e "    ${GREEN}✓${NC} Custo ILS: $custo_final"
        fi
    else
        echo -e "    ${RED}✗ Erro na execução${NC}"
        ref_display=${ref_value:-"N/A"}
        type_display=${value_type:-"N/A"}
        echo "$instance_name | ILS | ERRO | ERRO | ERRO | $ref_display | $type_display | ERRO" >> "$SUMMARY_FILE"
    fi
    echo ""
done

echo -e "${GREEN}=== Execução ILS Concluída ===${NC}"
echo "Resultados salvos em: $OUTPUT_DIR/"
echo "Resumo disponível em: $SUMMARY_FILE"
echo ""

# Gera estatísticas finais
echo -e "${BLUE}=== Estatísticas Finais ILS ===${NC}"
successful_runs=$(grep -v "ERRO" "$SUMMARY_FILE" | grep -v "^#" | wc -l)
total_runs=$(grep -v "^#" "$SUMMARY_FILE" | wc -l)

echo "Execuções bem-sucedidas: $successful_runs/$total_runs"

if [ $successful_runs -gt 0 ]; then
    echo ""
    best_cost=$(grep "| ILS |" "$SUMMARY_FILE" | grep -v "ERRO" | awk -F'|' '{print $3}' | sort -n | head -1 | tr -d ' ')
    best_instance=$(grep "| ILS |" "$SUMMARY_FILE" | grep -v "ERRO" | grep "| $best_cost |" | awk -F'|' '{print $1}' | tr -d ' ' | head -1)
    echo "Melhor custo ILS: $best_cost (instância: $best_instance)"

    echo ""
    echo "GAP médio ILS (apenas instâncias com referência):"
    avg_gap=$(grep "| ILS |" "$SUMMARY_FILE" | grep -v "ERRO" | grep -v "N/A" | awk -F'|' '{if($8 != " N/A") {sum+=$8; count++}} END {if(count>0) printf "%.2f", sum/count; else print "N/A"}')
    count_with_ref=$(grep "| ILS |" "$SUMMARY_FILE" | grep -v "ERRO" | grep -v "N/A" | awk -F'|' '{if($8 != " N/A") count++} END {print count+0}')
    echo "  ILS: ${avg_gap}% ($count_with_ref instâncias)"

    echo ""
    echo "Tempo médio ILS (todas as instâncias):"
    avg_time_ils=$(grep "| ILS |" "$SUMMARY_FILE" | grep -v "ERRO" | awk -F'|' '{gsub(/s/,"",$5); sum+=$5+0; count++} END {if(count>0) printf "%.3f", sum/count; else print "N/A"}')
    echo "  ILS: ${avg_time_ils}s"

    echo ""
    echo "Melhor GAP ILS:"
    best_gap_line=$(grep "| ILS |" "$SUMMARY_FILE" | grep -v "ERRO" | grep -v "N/A" | awk -F'|' '{if($8 != " N/A") print $0}' | sort -t'|' -k8 -n | head -1)
    if [ -n "$best_gap_line" ]; then
        best_gap=$(echo "$best_gap_line" | awk -F'|' '{print $8}' | tr -d ' ')
        best_gap_instance=$(echo "$best_gap_line" | awk -F'|' '{print $1}' | tr -d ' ')
        echo "  ILS: ${best_gap}% (instância: $best_gap_instance)"
    else
        echo "  ILS: N/A"
    fi
fi

echo ""
echo -e "${GREEN}Script ILS finalizado!${NC}"
