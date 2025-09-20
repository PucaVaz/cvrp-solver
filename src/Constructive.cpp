#include "Constructive.h"
#include <algorithm>
#include <limits>
#include <iostream>

long long RoutesCost(const Data& data, const Route& route) {
    long long cost = 0;

    if (route.nodes.size() < 2) {
        return cost; // Rota vazia tem custo 0
    }

    // Soma os custos de viagem entre nós consecutivos
    for (size_t i = 0; i < route.nodes.size() - 1; ++i) {
        int from = route.nodes[i];
        int to = route.nodes[i + 1];
        cost += static_cast<long long>(data.getTravelCost(from, to));
    }

    return cost;
}

long long SolutionCost(const Data& data, const Solution& solution) {
    long long total_cost = 0;

    for (const auto& route : solution.routes) {
        total_cost += RoutesCost(data, route);
    }

    return total_cost;
}

Solution GreedyNearestFeasible(const Data& data) {
    Solution solution;
    const int n_stations = data.getNumStations();
    const int max_vehicles = data.getNumVehicles();

    // Inicializa pool de estações não visitadas (1 a n)
    std::vector<bool> unvisited(n_stations + 1, true);
    unvisited[0] = false; // Depósito não precisa ser visitado como estação
    int remaining_stations = n_stations;

    // Constrói rotas até que todas as estações sejam visitadas
    while (remaining_stations > 0 && static_cast<int>(solution.routes.size()) < max_vehicles) {
        Route current_route;
        current_route.nodes.push_back(0); // Inicia no depósito

        int current_station = 0; // Posição atual (inicialmente no depósito)

        // Constrói a rota adicionando estações viáveis mais próximas
        while (true) {
            int best_station = -1;
            double best_cost = std::numeric_limits<double>::infinity();

            // Procura pela estação não visitada mais próxima que mantém viabilidade
            for (int station = 1; station <= n_stations; ++station) {
                if (!unvisited[station]) {
                    continue; // Estação já visitada
                }

                // Testa inserir esta estação no final da rota atual
                Route test_route = current_route;
                test_route.nodes.push_back(station);
                test_route.nodes.push_back(0); // Fecha a rota temporariamente para teste

                // Verifica se a rota continua viável
                RouteFeasInfo feas_info = CheckRouteFeasible(data, test_route);
                if (!feas_info.ok) {
                    continue; // Esta estação torna a rota inviável
                }

                // Calcula custo de viagem da posição atual até esta estação
                double travel_cost = data.getTravelCost(current_station, station);

                // Seleciona estação com menor custo de viagem
                if (travel_cost < best_cost) {
                    best_cost = travel_cost;
                    best_station = station;
                }
            }

            // Se não encontrou estação viável, finaliza a rota atual
            if (best_station == -1) {
                break;
            }

            // Adiciona a melhor estação encontrada à rota
            current_route.nodes.push_back(best_station);
            unvisited[best_station] = false;
            remaining_stations--;
            current_station = best_station;
        }

        // Finaliza a rota retornando ao depósito
        current_route.nodes.push_back(0);

        // Adiciona a rota à solução se ela contém pelo menos uma estação
        if (current_route.nodes.size() > 2) { // Mais que {0, 0}
            solution.routes.push_back(current_route);
        }

        // Se nenhuma estação foi adicionada nesta iteração, não conseguimos continuar
        if (current_route.nodes.size() <= 2) {
            std::cout << "Aviso: Não foi possível atribuir todas as estações aos veículos disponíveis." << std::endl;
            std::cout << "Estações restantes: " << remaining_stations << std::endl;
            break;
        }
    }

    // Calcula o custo total da solução
    solution.total_cost = SolutionCost(data, solution);

    return solution;
}