#include "Data.h"
#include "Feasibility.h"
#include "Constructive.h"
#include "CLI.h"
#include "IO.h"
#include "VND.h"
#include <string>
#include <random>
#include <chrono>

void PrintSolution(const Data& data, const Solution& solution) {
    cout << "\n=== Solução Construída (Heurística Gulosa) ===" << endl;
    cout << "Custo total da solução: " << solution.total_cost << endl;
    cout << "Número de veículos utilizados: " << solution.routes.size() << endl;

    for (size_t i = 0; i < solution.routes.size(); ++i) {
        const Route& route = solution.routes[i];
        cout << "Rota " << (i + 1) << ": ";

        for (size_t j = 0; j < route.nodes.size(); ++j) {
            cout << route.nodes[j];
            if (j < route.nodes.size() - 1) {
                cout << " -> ";
            }
        }

        double route_cost = RoutesCost(data, route);
        cout << " (custo: " << route_cost << ")" << endl;

        // Verifica viabilidade da rota
        RouteFeasInfo feas_info = CheckRouteFeasible(data, route);
        cout << "  Viável: " << (feas_info.ok ? "SIM" : "NÃO");
        if (feas_info.ok) {
            cout << " (carga inicial sugerida: " << feas_info.suggested_L0 << ")";
        }
        cout << endl;
    }
}

int main(int argc, char *argv[]) {
    // Processa argumentos da linha de comando
    CLIOptions opts = ParseCLI(argc, argv);

    // Inicializa gerador de números aleatórios com seed
    std::mt19937 rng(opts.seed);

    cout << "=== Solver JP-Bike Rebalancing ===" << endl;
    cout << "Semente aleatória: " << opts.seed << endl;
    cout << "Método construtivo: " << opts.constructive_method << endl;
    cout << "VND habilitado: " << (opts.use_vnd ? "SIM" : "NÃO") << endl;
    cout << "Diretório de saída: " << opts.output_dir << endl;
    cout << endl;

    // Carrega dados da instância usando apenas o caminho do arquivo
    char* temp_argv[2] = {argv[0], const_cast<char*>(opts.instance_path.c_str())};
    Data *data = new Data(2, temp_argv);
    
    cout << "Carregando instância do sistema JP-Bike..." << endl;
    data->read();
    
    cout << "\n";
    data->printJPBikeInstanceData();
    
    cout << "\n=== Matriz de Custos de Viagem ===" << endl;
    data->printDistanceMatrix();
    
    cout << "\n=== Informações para Algoritmo de Rebalanceamento ===" << endl;
    cout << "Total de localidades (depósito + estações): " << (data->getNumStations() + 1) << endl; // +1 para o depósito
    cout << "Soma total das demandas: ";
    long long total_demand = 0;
    for (int i = 0; i < data->getNumStations(); i++) {
        total_demand += data->getStationDemand(i); // getStationDemand(i) retorna a demanda da estação i
    }
    cout << total_demand << endl;
    if (total_demand == 0) {
        cout << "Sistema balanceado: soma das demandas = 0" << endl;
    } else {
        cout << "Sistema desbalanceado: diferença = " << total_demand << endl;
    }
    
    if (opts.run_feastest) {
        cout << "\n=== Teste de Viabilidade (--feastest) ===" << endl;
        
        // Teste 1: Rota r1 = {0, 0} - deve ser viável (rota vazia)
        Route r1;
        r1.nodes = {0, 0};
        RouteFeasInfo info1 = CheckRouteFeasible(*data, r1);
        
        cout << "Teste 1 - Rota vazia {0, 0}:" << endl;
        cout << "  Viável: " << (info1.ok ? "SIM" : "NÃO") << endl;
        cout << "  Violação de capacidade: " << info1.cap_violation << endl;
        cout << "  Max prefix: " << info1.max_prefix << endl;
        cout << "  Min prefix: " << info1.min_prefix << endl;
        cout << "  L0_min: " << info1.L0_min << endl;
        cout << "  L0_max: " << info1.L0_max << endl;
        cout << "  L0 sugerido: " << info1.suggested_L0 << endl;
        
        // Teste 2: Apenas se há estações suficientes (n >= 12)
        if (data->getNumStations() >= 12) {
            // Rota r2 = {0, 12, 5, 7, 9, 6, 2, 4, 11, 0} - deve ser inviável para Q=20
            // Demandas: 12=+5, 5=+1, 7=-5, 9=-5, 6=-4, 2=-3, 4=-3, 11=-2
            // Prefixes: 5, 6, 1, -4, -8, -11, -14, -16
            // Range: 6-(-16) = 22 > Q=20 → inviável
            Route r2;
            r2.nodes = {0, 12, 5, 7, 9, 6, 2, 4, 11, 0};
            RouteFeasInfo info2 = CheckRouteFeasible(*data, r2);

            cout << "\nTeste 2 - Rota {0, 12, 5, 7, 9, 6, 2, 4, 11, 0}:" << endl;
            cout << "  Demandas críticas: 12=" << NodeDemand(*data, 12)
                 << ", 5=" << NodeDemand(*data, 5) << " (coletas); "
                 << "7=" << NodeDemand(*data, 7) << ", 9=" << NodeDemand(*data, 9) << " (entregas)" << endl;
            cout << "  Viável: " << (info2.ok ? "SIM" : "NÃO") << endl;
            cout << "  Violação de capacidade: " << info2.cap_violation << endl;
            cout << "  Max prefix: " << info2.max_prefix << endl;
            cout << "  Min prefix: " << info2.min_prefix << endl;
            cout << "  L0_min: " << info2.L0_min << endl;
            cout << "  L0_max: " << info2.L0_max << endl;
            cout << "  L0 sugerido: " << info2.suggested_L0 << endl;
            cout << "  Capacidade do veículo (Q): " << data->getVehicleCapacity() << endl;

            cout << "\n=== Resultado dos Testes ===" << endl;
            cout << "Teste 1 (rota vazia): " << (info1.ok ? "PASSOU" : "FALHOU") << endl;
            cout << "Teste 2 (contra-exemplo): " << (!info2.ok ? "PASSOU (inviável como esperado)" : "FALHOU (deveria ser inviável)") << endl;
        } else {
            cout << "\nTeste 2 - Pulado (instância com n=" << data->getNumStations() << " < 12 estações)" << endl;

            cout << "\n=== Resultado dos Testes ===" << endl;
            cout << "Teste 1 (rota vazia): " << (info1.ok ? "PASSOU" : "FALHOU") << endl;
            cout << "Teste 2 (contra-exemplo): PULADO (instância pequena)" << endl;
        }
    }

    cout << "\n=== Executando Heurística Construtiva ===" << endl;

    auto start_time = std::chrono::high_resolution_clock::now();
    Solution solution;

    if (opts.constructive_method == "nearest") {
        cout << "Aplicando algoritmo Greedy Nearest-Feasible..." << endl;
        solution = GreedyNearestFeasible(*data, rng);
    } else if (opts.constructive_method == "insertion") {
        cout << "Aplicando algoritmo Greedy Best-Insertion..." << endl;
        solution = GreedyBestInsertion(*data, rng);
    } else {
        cout << "Método construtivo desconhecido: " << opts.constructive_method << endl;
        delete data;
        return 1;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    cout << "Tempo de construção: " << duration.count() << " ms" << endl;

    // Exibe a solução construída
    PrintSolution(*data, solution);

    // Verifica a viabilidade completa da solução
    bool solution_feasible = CheckSolutionFeasible(*data, solution.routes);
    cout << "\n=== Verificação Final ===" << endl;
    cout << "Solução completa é viável: " << (solution_feasible ? "SIM" : "NÃO") << endl;

    if (solution_feasible) {
        cout << "Todas as " << data->getNumStations() << " estações foram atendidas com sucesso!" << endl;
    } else {
        cout << "ATENÇÃO: A solução construída possui problemas de viabilidade." << endl;
    }

    // Aplicar VND se habilitado
    Solution final_solution = solution;
    if (opts.use_vnd) {
        cout << "\n=== VND (Variable Neighborhood Descent) ===" << endl;
        cout << "Custo inicial: " << solution.total_cost << endl;

        auto vnd_start = std::chrono::high_resolution_clock::now();
        auto neighborhoods = GetAllNeighborhoods();
        final_solution = VND(*data, solution, rng, neighborhoods);
        auto vnd_end = std::chrono::high_resolution_clock::now();

        auto vnd_duration = std::chrono::duration_cast<std::chrono::milliseconds>(vnd_end - vnd_start);

        cout << "Custo final após VND: " << final_solution.total_cost << endl;
        cout << "Melhoria: " << (solution.total_cost - final_solution.total_cost) << endl;
        cout << "Tempo VND: " << vnd_duration.count() << " ms" << endl;

        // Verifica se VND manteve viabilidade
        bool vnd_feasible = CheckSolutionFeasible(*data, final_solution.routes);
        if (!vnd_feasible) {
            cout << "ATENÇÃO: VND produziu solução inviável! Usando apenas a solução construtiva." << endl;
            final_solution = solution;
        }
    }

    // Verifica viabilidade final
    bool final_feasible = CheckSolutionFeasible(*data, final_solution.routes);

    // Salva a solução final no arquivo de saída apenas se viável
    if (final_feasible) {
        std::string output_filename = opts.output_dir + data->getInstanceName() + ".out";
        WriteSolution(output_filename, *data, final_solution);
    }

    cout << "\n=== Resumo Final ===" << endl;
    cout << "Instância: " << data->getInstanceName() << endl;
    cout << "Método: " << opts.constructive_method << endl;
    if (opts.use_vnd) {
        cout << "Custo construtivo: " << solution.total_cost << endl;
        cout << "Custo após VND: " << final_solution.total_cost << endl;
        cout << "Melhoria total: " << (solution.total_cost - final_solution.total_cost) << endl;
    } else {
        cout << "Custo final: " << final_solution.total_cost << endl;
    }
    cout << "Veículos utilizados: " << final_solution.routes.size() << "/" << data->getNumVehicles() << endl;
    cout << "Viável: " << (final_feasible ? "SIM" : "NÃO") << endl;

    if (!final_feasible) {
        cout << "\nERRO: Solução final é inviável - arquivo .out não foi gerado!" << endl;
    }

    delete data;

    return final_feasible ? 0 : 1;
}
