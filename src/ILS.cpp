#include "ILS.h"
#include "Feasibility.h"
#include <iostream>
#include <algorithm>
#include <limits>

static double SampleAlpha(std::mt19937& rng, double alpha_min, double alpha_max) {
    std::uniform_real_distribution<double> dist(alpha_min, alpha_max);
    return dist(rng);
}

static void ApplyRandomPerturbation(const Data& data, Solution& solution,
                                    std::mt19937& rng, int strength) {
    if (solution.routes.empty()) return;
    
    std::uniform_int_distribution<int> strategy_dist(0, 2);
    int strategy = strategy_dist(rng);
    
    switch (strategy) {
        case 0: {
            if (solution.routes.size() < 2) return;
            
            std::uniform_int_distribution<size_t> route_dist(0, solution.routes.size() - 1);
            size_t from_route_idx, to_route_idx;
            do {
                from_route_idx = route_dist(rng);
                to_route_idx = route_dist(rng);
            } while (from_route_idx == to_route_idx);
            
            Route& from_route = solution.routes[from_route_idx];
            Route& to_route = solution.routes[to_route_idx];
            
            if (from_route.nodes.size() <= 2) return;
            
            int available_stations = static_cast<int>(from_route.nodes.size()) - 2;
            int block_size = std::min(strength, std::max(1, available_stations / 2));
            std::uniform_int_distribution<int> size_dist(1, std::min(block_size, available_stations));
            int selected_size = size_dist(rng);
            
            std::uniform_int_distribution<size_t> start_dist(1, from_route.nodes.size() - 1 - selected_size);
            size_t start_pos = start_dist(rng);
            
            std::vector<int> block(from_route.nodes.begin() + start_pos,
                                  from_route.nodes.begin() + start_pos + selected_size);
            std::vector<int> original_from_nodes = from_route.nodes;
            from_route.nodes.erase(from_route.nodes.begin() + start_pos,
                                  from_route.nodes.begin() + start_pos + selected_size);

            if (!CheckRouteFeasible(data, from_route).ok) {
                from_route.nodes = std::move(original_from_nodes);
                break;
            }
            
            bool inserted = false;
            for (size_t pos = 1; pos <= to_route.nodes.size() - 1 && !inserted; ++pos) {
                Route test_route = to_route;
                test_route.nodes.insert(test_route.nodes.begin() + pos, block.begin(), block.end());
                
                RouteFeasInfo feas_info = CheckRouteFeasible(data, test_route);
                if (feas_info.ok) {
                    to_route = test_route;
                    inserted = true;
                }
            }
            
            if (!inserted) {
                from_route.nodes = std::move(original_from_nodes);
            }
            break;
        }
        
        case 1: {
            if (solution.routes.size() < 2) return;
            
            std::vector<size_t> valid_routes;
            for (size_t i = 0; i < solution.routes.size(); ++i) {
                if (solution.routes[i].nodes.size() > 2) {
                    valid_routes.push_back(i);
                }
            }
            
            if (valid_routes.size() < 2) return;
            
            std::uniform_int_distribution<size_t> valid_dist(0, valid_routes.size() - 1);
            size_t idx1 = valid_dist(rng);
            size_t idx2;
            do {
                idx2 = valid_dist(rng);
            } while (idx1 == idx2);
            
            Route& route1 = solution.routes[valid_routes[idx1]];
            Route& route2 = solution.routes[valid_routes[idx2]];
            
            std::uniform_int_distribution<size_t> pos1_dist(1, route1.nodes.size() - 2);
            std::uniform_int_distribution<size_t> pos2_dist(1, route2.nodes.size() - 2);
            
            size_t pos1 = pos1_dist(rng);
            size_t pos2 = pos2_dist(rng);
            
            int station1 = route1.nodes[pos1];
            int station2 = route2.nodes[pos2];
            
            route1.nodes[pos1] = station2;
            route2.nodes[pos2] = station1;
            
            bool route1_feasible = CheckRouteFeasible(data, route1).ok;
            bool route2_feasible = CheckRouteFeasible(data, route2).ok;
            
            if (!route1_feasible || !route2_feasible) {
                route1.nodes[pos1] = station1;
                route2.nodes[pos2] = station2;
            }
            break;
        }
        
        case 2: {
            if (solution.routes.empty()) return;
            
            std::uniform_int_distribution<size_t> route_dist(0, solution.routes.size() - 1);
            size_t route_idx = route_dist(rng);
            Route& route = solution.routes[route_idx];
            
            if (route.nodes.size() < 5) return; // Need at least 0-a-b-c-0
            
            std::uniform_int_distribution<size_t> pos_dist(1, route.nodes.size() - 3);
            size_t pos1 = pos_dist(rng);
            size_t pos2 = pos1 + 1 + (rng() % std::min(strength, static_cast<int>(route.nodes.size() - pos1 - 2)));
            
            if (pos2 >= route.nodes.size() - 1) return;
            
            std::reverse(route.nodes.begin() + pos1, route.nodes.begin() + pos2 + 1);
            
            RouteFeasInfo feas_info = CheckRouteFeasible(data, route);
            if (!feas_info.ok) {
                std::reverse(route.nodes.begin() + pos1, route.nodes.begin() + pos2 + 1);
            }
            break;
        }
    }
    
    solution.total_cost = SolutionCost(data, solution);
}

Solution ILS(const Data& data, std::mt19937& rng, const ILSParams& p, bool verbose) {
    Solution best_solution;
    best_solution.total_cost = std::numeric_limits<double>::infinity();
    
    if (verbose) {
        std::cout << "\n=== ILS (Iterated Local Search) ===" << std::endl;
        std::cout << "Parâmetros: max_iter=" << p.max_iter 
                  << ", max_iter_ils=" << p.max_iter_ils
                  << ", alpha=[" << p.alpha_min << "," << p.alpha_max << "]"
                  << ", perturb_strength=" << p.perturb_strength << std::endl;
    }
    
    for (int t = 1; t <= p.max_iter; ++t) {
        double alpha = SampleAlpha(rng, p.alpha_min, p.alpha_max);
        
        Solution current_solution = BuildInitial_GRASP(data, rng, alpha);
        
        current_solution = RVND(data, current_solution, rng);
        
        if (current_solution.total_cost < best_solution.total_cost) {
            best_solution = current_solution;
            if (verbose) {
                std::cout << "Iter " << t << ": Nova melhor solução = " << best_solution.total_cost << std::endl;
            }
        }
        
        int stall = 0;
        Solution current_best = best_solution;
        
        while (stall < p.max_iter_ils) {
            Solution perturbed_solution = current_best;
            ApplyRandomPerturbation(data, perturbed_solution, rng, p.perturb_strength);
            
                perturbed_solution = RVND(data, perturbed_solution, rng);
            
            if (perturbed_solution.total_cost < current_best.total_cost) {
                current_best = perturbed_solution;
                stall = 0;
                
                if (current_best.total_cost < best_solution.total_cost) {
                    best_solution = current_best;
                    if (verbose) {
                        std::cout << "Iter " << t << " (ILS): Nova melhor solução = " << best_solution.total_cost << std::endl;
                    }
                }
            } else {
                stall++;
            }
        }
        
        if (verbose && t % 10 == 0) {
            std::cout << "Progresso: " << t << "/" << p.max_iter << " iterações completadas" << std::endl;
        }
    }
    
    if (verbose) {
        std::cout << "ILS finalizado. Melhor custo: " << best_solution.total_cost << std::endl;
    }
    
    return best_solution;
}
