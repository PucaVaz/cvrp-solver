#include "Construction.h"
#include <algorithm>
#include <limits>
#include <iostream>

double RouteCost(const Data& data, const Route& route) {
    double cost = 0.0;

    if (route.nodes.size() < 2) {
        return cost;
    }
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

    std::vector<bool> unvisited(n_stations + 1, true);
    unvisited[0] = false;
    int remaining_stations = n_stations;
    while (remaining_stations > 0 && static_cast<int>(solution.routes.size()) < max_vehicles) {
        Route current_route;
        current_route.nodes.push_back(0);

        int current_station = 0;
        while (true) {
            int best_station = -1;
            double best_cost = std::numeric_limits<double>::infinity();

            for (int station = 1; station <= n_stations; ++station) {
                if (!unvisited[station]) {
                    continue;
                }

                Route test_route = current_route;
                test_route.nodes.push_back(station);
                test_route.nodes.push_back(0);

                RouteFeasInfo feas_info = CheckRouteFeasible(data, test_route);
                if (!feas_info.ok) {
                    continue;
                }

                double travel_cost = data.getTravelCost(current_station, station);

                if (travel_cost < best_cost) {
                    best_cost = travel_cost;
                    best_station = station;
                }
            }

            if (best_station == -1) {
                break;
            }
            current_route.nodes.push_back(best_station);
            unvisited[best_station] = false;
            remaining_stations--;
            current_station = best_station;
        }

        current_route.nodes.push_back(0);

        if (current_route.nodes.size() > 2) {
            solution.routes.push_back(current_route);
        }

        if (current_route.nodes.size() <= 2) {
            std::cout << "Aviso: Não foi possível atribuir todas as estações aos veículos disponíveis." << std::endl;
            std::cout << "Estações restantes: " << remaining_stations << std::endl;
            break;
        }
    }

    solution.total_cost = SolutionCost(data, solution);

    return solution;
}

Solution GreedyBestInsertion(const Data& data, std::mt19937& rng) {
    Solution solution;
    const int n_stations = data.getNumStations();
    const int max_vehicles = data.getNumVehicles();

    std::vector<bool> unvisited(n_stations + 1, true);
    unvisited[0] = false;
    int remaining_stations = n_stations;

    Route initial_route;
    initial_route.nodes = {0, 0};
    solution.routes.push_back(initial_route);
    while (remaining_stations > 0) {
        int best_station = -1;
        size_t best_route_idx = 0;
        size_t best_position = 0;
        double best_delta_cost = std::numeric_limits<double>::max();
        bool found_feasible = false;

        for (int station = 1; station <= n_stations; ++station) {
            if (!unvisited[station]) {
                continue;
            }

            for (size_t route_idx = 0; route_idx < solution.routes.size(); ++route_idx) {
                const Route& current_route = solution.routes[route_idx];

                // (exceto primeira e última que são sempre depósito)
                for (size_t pos = 1; pos < current_route.nodes.size(); ++pos) {
                    Route test_route = current_route;
                    test_route.nodes.insert(test_route.nodes.begin() + pos, station);

                    RouteFeasInfo feas_info = CheckRouteFeasible(data, test_route);
                    if (!feas_info.ok) {
                        continue;
                    }

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
        else {
            if (static_cast<int>(solution.routes.size()) >= max_vehicles) {
                std::cout << "Aviso: Não há veículos suficientes para atender todas as estações." << std::endl;
                std::cout << "Estações restantes: " << remaining_stations << std::endl;
                break;
            }

            int nearest_station = -1;
            double nearest_cost = std::numeric_limits<double>::infinity();

            for (int station = 1; station <= n_stations; ++station) {
                if (!unvisited[station]) {
                    continue;
                }

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

    solution.total_cost = SolutionCost(data, solution);

    return solution;
}

Solution BuildInitial_GRASP(const Data& data, std::mt19937& rng, double alpha_in) {
    Solution solution;
    const int n_stations = data.getNumStations();
    const int max_vehicles = data.getNumVehicles();

    std::vector<bool> unvisited(n_stations + 1, true);
    unvisited[0] = false;
    int remaining_stations = n_stations;

    Route initial_route;
    initial_route.nodes = {0, 0};
    solution.routes.push_back(initial_route);

    struct InsertionCandidate {
        int station;
        size_t route_idx;
        size_t position;
        double delta_cost;
    };

    while (remaining_stations > 0) {
        std::vector<InsertionCandidate> candidates;

        for (int station = 1; station <= n_stations; ++station) {
            if (!unvisited[station]) {
                continue;
            }

            for (size_t route_idx = 0; route_idx < solution.routes.size(); ++route_idx) {
                const Route& current_route = solution.routes[route_idx];

                for (size_t pos = 1; pos < current_route.nodes.size(); ++pos) {
                    Route test_route = current_route;
                    test_route.nodes.insert(test_route.nodes.begin() + pos, station);

                    RouteFeasInfo feas_info = CheckRouteFeasible(data, test_route);
                    if (!feas_info.ok) {
                        continue;
                    }

                    double original_cost = RouteCost(data, current_route);
                    double new_cost = RouteCost(data, test_route);
                    double delta_cost = new_cost - original_cost;

                    candidates.push_back({station, route_idx, pos, delta_cost});
                }
            }
        }

        if (!candidates.empty()) {
            std::sort(candidates.begin(), candidates.end(),
                     [](const InsertionCandidate& a, const InsertionCandidate& b) {
                         return a.delta_cost < b.delta_cost;
                     });

            double min_cost = candidates[0].delta_cost;
            double max_cost = candidates.back().delta_cost;
            double threshold = min_cost + alpha_in * (max_cost - min_cost);

            std::vector<InsertionCandidate> rcl;
            for (const auto& candidate : candidates) {
                if (candidate.delta_cost <= threshold) {
                    rcl.push_back(candidate);
                }
            }

            std::uniform_int_distribution<size_t> dist(0, rcl.size() - 1);
            const InsertionCandidate& selected = rcl[dist(rng)];

            solution.routes[selected.route_idx].nodes.insert(
                solution.routes[selected.route_idx].nodes.begin() + selected.position,
                selected.station
            );
            unvisited[selected.station] = false;
            remaining_stations--;
        }
        else {
            if (static_cast<int>(solution.routes.size()) >= max_vehicles) {
                std::cout << "Aviso: Não há veículos suficientes para atender todas as estações." << std::endl;
                std::cout << "Estações restantes: " << remaining_stations << std::endl;
                break;
            }

            int nearest_station = -1;
            double nearest_cost = std::numeric_limits<double>::infinity();

            for (int station = 1; station <= n_stations; ++station) {
                if (!unvisited[station]) {
                    continue;
                }

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

    solution.total_cost = SolutionCost(data, solution);

    return solution;
}