
SOLVER="../bin/apa_jpbike"
INSTANCES_DIR="../instances/instancias_teste"
OUTPUT_DIR="../results_teste_final"
NUM_RUNS=10
BASE_SEED=100
CONSTRUCTIVE_METHODS=("nearest" "insertion")

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

echo -e "${BLUE}=== Executando Solver JP-Bike em todas as instâncias de teste ===${NC}"
echo "Diretório de instâncias: $INSTANCES_DIR"
echo "Diretório de resultados: $OUTPUT_DIR"
echo "Número de execuções por instância: $NUM_RUNS"
echo "Sementes: $BASE_SEED até $((BASE_SEED + NUM_RUNS - 1))"
echo ""

if [ ! -f "$SOLVER" ]; then
    echo -e "${RED}Erro: Solver não encontrado em $SOLVER${NC}"
    echo "Execute 'make' no diretório src/ primeiro."
    exit 1
fi

mkdir -p "$OUTPUT_DIR"

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

SUMMARY_FILE="$OUTPUT_DIR/resumo_execucoes.txt"
DETAILED_FILE="$OUTPUT_DIR/resultados_detalhados.txt"
AVERAGES_FILE="$OUTPUT_DIR/medias_finais.txt"

echo "# Resumo das Execuções - Instâncias de Teste ($NUM_RUNS execuções por combinação)" > "$SUMMARY_FILE"
echo "# Instância | Método | Run | Seed | Custo_Construtivo | Custo_VND | Melhoria | Veículos | Tempo_Total | Referência | Tipo_Ref | GAP%" >> "$SUMMARY_FILE"

echo "# Resultados Detalhados - Todas as Execuções" > "$DETAILED_FILE"
echo "# Instância | Método | Run | Seed | Custo_Construtivo | Custo_VND | Melhoria | Veículos | Tempo_Total | Referência | Tipo_Ref | GAP%" >> "$DETAILED_FILE"

echo "# Médias Finais - $NUM_RUNS execuções por combinação" > "$AVERAGES_FILE"
echo "# Instância | Método | Média_Custo_Construtivo | Média_Custo_VND | Média_Melhoria | Média_Veículos | Média_Tempo | Referência | Tipo_Ref | Média_GAP% | Desvio_Padrão_GAP" >> "$AVERAGES_FILE"

total_instances=$((${#instances[@]} * ${#CONSTRUCTIVE_METHODS[@]} * NUM_RUNS))
current=0

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
        echo -e "  Método: $method"

        # Execuções serão agregadas posteriormente a partir do SUMMARY_FILE (compatível com bash 3)

        for run in $(seq 1 $NUM_RUNS); do
            current=$((current + 1))
            current_seed=$((BASE_SEED + run - 1))
            echo -e "    [$current/$total_instances] Run $run/$NUM_RUNS (seed: $current_seed)"

            output_file="$OUTPUT_DIR/${instance_name}_${method}_run${run}.out"
            log_file="$OUTPUT_DIR/${instance_name}_${method}_run${run}.log"

            start_time=$(date +%s.%3N)

            "$SOLVER" --instance "$instance" \
                      --seed "$current_seed" \
                      --constructive "$method" \
                      --out "$OUTPUT_DIR/" > "$log_file" 2>&1

            exit_code=$?
            end_time=$(date +%s.%3N)

            if [ $exit_code -eq 0 ]; then
                custo_construtivo=$(grep "Custo construtivo:" "$log_file" | awk '{print $3}' | head -1)
                custo_vnd=$(grep "Custo após VND:" "$log_file" | awk '{print $4}' | head -1)
                melhoria=$(grep "Melhoria total:" "$log_file" | awk '{print $3}' | head -1)
                veiculos=$(grep "Veículos utilizados:" "$log_file" | awk -F'/' '{print $1}' | awk '{print $3}' | head -1)

                if [ -z "$custo_construtivo" ] || [ -z "$custo_vnd" ]; then
                    custo_final=$(grep "Custo final:" "$log_file" | awk '{print $3}' | head -1)
                    custo_construtivo="$custo_final"
                    custo_vnd="$custo_final"
                    melhoria="0"
                fi

                tempo_total=$(echo "$end_time - $start_time" | bc)

                gap=$(calculate_gap "$custo_vnd" "$ref_value")

                # dados por execução gravados em $SUMMARY_FILE; médias serão calculadas depois via awk

                ref_display=${ref_value:-"N/A"}
                type_display=${value_type:-"N/A"}
                echo "$instance_name | $method | $run | $current_seed | $custo_construtivo | $custo_vnd | $melhoria | $veiculos | ${tempo_total}s | $ref_display | $type_display | $gap" >> "$SUMMARY_FILE"
                echo "$instance_name | $method | $run | $current_seed | $custo_construtivo | $custo_vnd | $melhoria | $veiculos | ${tempo_total}s | $ref_display | $type_display | $gap" >> "$DETAILED_FILE"

                if [ "$gap" != "N/A" ]; then
                    echo -e "      ${GREEN}✓${NC} Custo: $custo_construtivo → $custo_vnd (melhoria: $melhoria, GAP: ${gap}%)"
                else
                    echo -e "      ${GREEN}✓${NC} Custo: $custo_construtivo → $custo_vnd (melhoria: $melhoria)"
                fi
            else
                echo -e "      ${RED}✗ Erro na execução${NC}"
                ref_display=${ref_value:-"N/A"}
                type_display=${value_type:-"N/A"}
                echo "$instance_name | $method | $run | $current_seed | ERRO | ERRO | ERRO | ERRO | ERRO | $ref_display | $type_display | ERRO" >> "$SUMMARY_FILE"
                echo "$instance_name | $method | $run | $current_seed | ERRO | ERRO | ERRO | ERRO | ERRO | $ref_display | $type_display | ERRO" >> "$DETAILED_FILE"
            fi
        done
    done
    echo ""
done

echo -e "${BLUE}=== Calculando Médias ===${NC}"
for instance in "${instances[@]}"; do
    instance_name=$(basename "$instance" .txt)
    ref_value=$(get_reference_value "$instance_name")
    value_type=$(get_value_type "$instance_name")
    ref_display=${ref_value:-"N/A"}
    type_display=${value_type:-"N/A"}

    for method in "${CONSTRUCTIVE_METHODS[@]}"; do
        awk -F'|' -v inst="$instance_name" -v mtd="$method" -v refd="$ref_display" -v typed="$type_display" '
            function trim(s){ gsub(/^ +| +$/, "", s); return s }
            !/^#/ {
                i1=trim($1); i2=trim($2);
                if(i1==inst && i2==mtd){
                    c++;
                    v5=trim($5); v6=trim($6); v7=trim($7); v8=trim($8); v9=trim($9); v12=trim($12);
                    gsub(/s/, "", v9);
                    sum5+=v5+0; sum6+=v6+0; sum7+=v7+0; sum8+=v8+0; sum9+=v9+0;
                    if(v12!="N/A"){ gaps[++ng]=v12+0; sumg+=v12+0; cg++ }
                }
            }
            END{
                if(c>0){
                    avg5=sum5/c; avg6=sum6/c; avg7=sum7/c; avg8=sum8/c; avg9=sum9/c;
                    if(cg>0){
                        avgg=sumg/cg; for(i=1;i<=ng;i++){ dev+=(gaps[i]-avgg)^2 };
                        std=sqrt(dev/(cg-1));
                        printf("%s | %s | %.2f | %.2f | %.2f | %.1f | %.3fs | %s | %s | %.2f | %.2f\n", inst, mtd, avg5, avg6, avg7, avg8, avg9, refd, typed, avgg, std);
                    } else {
                        printf("%s | %s | %.2f | %.2f | %.2f | %.1f | %.3fs | %s | %s | %s | %s\n", inst, mtd, avg5, avg6, avg7, avg8, avg9, refd, typed, "N/A", "N/A");
                    }
                }
            }' "$SUMMARY_FILE" >> "$AVERAGES_FILE"
    done
done

echo -e "${GREEN}=== Execução Concluída ===${NC}"
echo "Resultados salvos em: $OUTPUT_DIR/"
echo "Resumo detalhado: $SUMMARY_FILE"
echo "Todos os resultados: $DETAILED_FILE"
echo "Médias finais: $AVERAGES_FILE"
echo ""

echo -e "${BLUE}=== Estatísticas Finais ===${NC}"
successful_runs=$(grep -v "ERRO" "$SUMMARY_FILE" | grep -v "^#" | wc -l)
total_runs=$(grep -v "^#" "$SUMMARY_FILE" | wc -l)

echo "Execuções bem-sucedidas: $successful_runs/$total_runs"

if [ $successful_runs -gt 0 ]; then
    echo ""
    echo "Melhor custo VND por método (entre todas as execuções):"
    for method in "${CONSTRUCTIVE_METHODS[@]}"; do
        best_cost=$(grep "| $method |" "$SUMMARY_FILE" | grep -v "ERRO" | awk -F'|' '{print $6}' | sort -n | head -1 | tr -d ' ')
        best_instance=$(grep "| $method |" "$SUMMARY_FILE" | grep -v "ERRO" | grep "| $best_cost |" | awk -F'|' '{print $1}' | tr -d ' ' | head -1)
        echo "  $method: $best_cost (instância: $best_instance)"
    done

    echo ""
    echo "Melhoria média do VND (todas as execuções):"
    for method in "${CONSTRUCTIVE_METHODS[@]}"; do
        avg_improvement=$(grep "| $method |" "$SUMMARY_FILE" | grep -v "ERRO" | awk -F'|' '{sum+=$7; count++} END {if(count>0) print sum/count; else print 0}')
        echo "  $method: $(printf "%.1f" "$avg_improvement")"
    done

    echo ""
    echo "GAP médio por método (todas as execuções com referência):"
    for method in "${CONSTRUCTIVE_METHODS[@]}"; do
        avg_gap=$(grep "| $method |" "$SUMMARY_FILE" | grep -v "ERRO" | grep -v "N/A" | awk -F'|' '{if($12 != " N/A") {sum+=$12; count++}} END {if(count>0) printf "%.2f", sum/count; else print "N/A"}')
        count_with_ref=$(grep "| $method |" "$SUMMARY_FILE" | grep -v "ERRO" | grep -v "N/A" | awk -F'|' '{if($12 != " N/A") count++} END {print count+0}')
        echo "  $method: ${avg_gap}% ($count_with_ref execuções)"
    done

    echo ""
    echo "Tempo médio por método (todas as execuções):"
    for method in "${CONSTRUCTIVE_METHODS[@]}"; do
        avg_time=$(grep "| $method |" "$SUMMARY_FILE" | grep -v "ERRO" | awk -F'|' '{gsub(/s/,"",$9); sum+=$9+0; count++} END {if(count>0) printf "%.3f", sum/count; else print "N/A"}')
        echo "  $method: ${avg_time}s"
    done

    echo ""
    echo "Melhores GAPs por método:"
    for method in "${CONSTRUCTIVE_METHODS[@]}"; do
        best_gap_line=$(grep "| $method |" "$SUMMARY_FILE" | grep -v "ERRO" | grep -v "N/A" | awk -F'|' '{if($12 != " N/A") print $0}' | sort -t'|' -k12 -n | head -1)
        if [ -n "$best_gap_line" ]; then
            best_gap=$(echo "$best_gap_line" | awk -F'|' '{print $12}' | tr -d ' ')
            best_gap_instance=$(echo "$best_gap_line" | awk -F'|' '{print $1}' | tr -d ' ')
            best_run=$(echo "$best_gap_line" | awk -F'|' '{print $3}' | tr -d ' ')
            echo "  $method: ${best_gap}% (instância: $best_gap_instance, run: $best_run)"
        else
            echo "  $method: N/A"
        fi
    done

    echo ""
    echo "Estatísticas baseadas nas médias (arquivo: medias_finais.txt):"
    if [ -f "$AVERAGES_FILE" ] && [ $(grep -v "^#" "$AVERAGES_FILE" | wc -l) -gt 0 ]; then
        echo "  Melhor GAP médio por método:"
        for method in "${CONSTRUCTIVE_METHODS[@]}"; do
            best_avg_gap=$(grep "| $method |" "$AVERAGES_FILE" | grep -v "N/A" | awk -F'|' '{print $10}' | sort -n | head -1 | tr -d ' ')
            if [ -n "$best_avg_gap" ] && [ "$best_avg_gap" != "N/A" ]; then
                best_avg_instance=$(grep "| $method |" "$AVERAGES_FILE" | grep "| $best_avg_gap |" | awk -F'|' '{print $1}' | tr -d ' ' | head -1)
                echo "    $method: ${best_avg_gap}% (instância: $best_avg_instance)"
            else
                echo "    $method: N/A"
            fi
        done
    fi
fi

echo ""
echo -e "${GREEN}Script finalizado!${NC}"
