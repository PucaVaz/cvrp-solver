#include "Construction.h"
#include <algorithm>
#include <limits>
#include <iostream>

double RouteCost(const Data& data, const Route& route) {
    double cost = 0.0;

    if (route.nodes.size() < 2) {
        return cost; // Rota vazia tem custo 0
    }

    // Soma os custos de viagem entre nós consecutivos
    for (size_t i = 0; i < route.nodes.size() - 1; ++i) {
        int from = route.nodes[i];
        int to = route.nodes[i + 1];
        cost += data.getTravelCost(from, to);
    }

    return cost;
}

double SolutionCost(const Data& data, const Solution& solution) {
    double total_cost = 0.0;

    for (const auto& route : solution.routes) {
        total_cost += RouteCost(data, route);
    }

    return total_cost;
}

Solution GreedyNearestFeasible(const Data& data, std::mt19937& rng) {
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

Solution GreedyBestInsertion(const Data& data, std::mt19937& rng) {
    Solution solution;
    const int n_stations = data.getNumStations();
    const int max_vehicles = data.getNumVehicles();

    // Inicializa pool de estações não visitadas (1 a n)
    std::vector<bool> unvisited(n_stations + 1, true);
    unvisited[0] = false; // Depósito não precisa ser visitado como estação
    int remaining_stations = n_stations;

    // Cria primeira rota vazia
    Route initial_route;
    initial_route.nodes = {0, 0}; // Rota vazia: depósito -> depósito
    solution.routes.push_back(initial_route);

    // Insere estações até que todas sejam visitadas
    while (remaining_stations > 0) {
        int best_station = -1;
        size_t best_route_idx = 0;
        size_t best_position = 0;
        double best_delta_cost = std::numeric_limits<double>::max();
        bool found_feasible = false;

        // Para cada estação não visitada
        for (int station = 1; station <= n_stations; ++station) {
            if (!unvisited[station]) {
                continue; // Estação já visitada
            }

            // Testa inserção em todas as rotas existentes
            for (size_t route_idx = 0; route_idx < solution.routes.size(); ++route_idx) {
                const Route& current_route = solution.routes[route_idx];

                // Testa todas as posições de inserção possíveis na rota
                // (exceto primeira e última que são sempre depósito)
                for (size_t pos = 1; pos < current_route.nodes.size(); ++pos) {
                    // Cria rota temporária com a estação inserida
                    Route test_route = current_route;
                    test_route.nodes.insert(test_route.nodes.begin() + pos, station);

                    // Verifica viabilidade
                    RouteFeasInfo feas_info = CheckRouteFeasible(data, test_route);
                    if (!feas_info.ok) {
                        continue; // Inserção inviável
                    }

                    // Calcula delta de custo da inserção
                    double original_cost = RouteCost(data, current_route);
                    double new_cost = RouteCost(data, test_route);
                    double delta_cost = new_cost - original_cost;

                    // Seleciona melhor inserção
                    if (delta_cost < best_delta_cost) {
                        best_delta_cost = delta_cost;
                        best_station = station;
                        best_route_idx = route_idx;
                        best_position = pos;
                        found_feasible = true;
                    }
                }
            }
        }

        // Se encontrou inserção viável, aplica
        if (found_feasible) {
            solution.routes[best_route_idx].nodes.insert(
                solution.routes[best_route_idx].nodes.begin() + best_position,
                best_station
            );
            unvisited[best_station] = false;
            remaining_stations--;
        }
        // Se não encontrou inserção viável em rotas existentes, cria nova rota
        else {
            // Verifica se ainda há veículos disponíveis
            if (static_cast<int>(solution.routes.size()) >= max_vehicles) {
                std::cout << "Aviso: Não há veículos suficientes para atender todas as estações." << std::endl;
                std::cout << "Estações restantes: " << remaining_stations << std::endl;
                break;
            }

            // Encontra estação mais próxima do depósito para iniciar nova rota
            int nearest_station = -1;
            double nearest_cost = std::numeric_limits<double>::infinity();

            for (int station = 1; station <= n_stations; ++station) {
                if (!unvisited[station]) {
                    continue;
                }

                // Testa rota simples: 0 -> station -> 0
                Route test_route;
                test_route.nodes = {0, station, 0};

                RouteFeasInfo feas_info = CheckRouteFeasible(data, test_route);
                if (feas_info.ok) {
                    double cost = data.getTravelCost(0, station);
                    if (cost < nearest_cost) {
                        nearest_cost = cost;
                        nearest_station = station;
                    }
                }
            }

            // Se encontrou estação viável para nova rota
            if (nearest_station != -1) {
                Route new_route;
                new_route.nodes = {0, nearest_station, 0};
                solution.routes.push_back(new_route);
                unvisited[nearest_station] = false;
                remaining_stations--;
            } else {
                std::cout << "Erro: Não foi possível criar rota viável para estações restantes." << std::endl;
                break;
            }
        }
    }

    // Calcula o custo total da solução
    solution.total_cost = SolutionCost(data, solution);

    return solution;
}