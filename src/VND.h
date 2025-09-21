#ifndef VND_H
#define VND_H

#include <vector>
#include <functional>
#include <random>
#include <set>
#include "Constructive.h"
#include "Data.h"

// Resultado das operações de vizinhança
struct NeighborhoodResult {
    bool improved;                    // Se houve melhoria
    std::set<size_t> modified_routes; // Índices das rotas modificadas

    NeighborhoodResult() : improved(false) {}
    NeighborhoodResult(bool imp) : improved(imp) {}
    NeighborhoodResult(bool imp, std::set<size_t> routes) : improved(imp), modified_routes(routes) {}
};

// Função de vizinhança: retorna resultado com melhoria e rotas modificadas
using NeighborhoodFunction = std::function<NeighborhoodResult(const Data&, Solution&, std::mt19937&)>;
// Implementações das vizinhanças

// Vizinhança intra-rota: inverte segmento (2-opt)
NeighborhoodResult ImproveTwoOpt(const Data& data, Solution& solution, std::mt19937& rng);

// Vizinhança inter/intra-rota: move um cliente
NeighborhoodResult ImproveRelocate(const Data& data, Solution& solution, std::mt19937& rng);

// Vizinhança inter-rota: troca clientes entre rotas
NeighborhoodResult ImproveSwap(const Data& data, Solution& solution, std::mt19937& rng);

// Vizinhança intra-rota: move cadeia de 2 clientes (Or-opt)
NeighborhoodResult ImproveOrOpt2(const Data& data, Solution& solution, std::mt19937& rng);

// Versões direcionadas das vizinhanças intra-rota para rotas específicas
NeighborhoodResult TwoOptTargeted(const Data& data, Solution& solution, std::mt19937& rng,
                                  const std::set<size_t>& target_routes);
NeighborhoodResult OrOpt2Targeted(const Data& data, Solution& solution, std::mt19937& rng,
                                  const std::set<size_t>& target_routes);

// Mini-RVND para intensificação em rotas específicas
bool IntraRouteRVND(const Data& data, Solution& solution, std::mt19937& rng,
                    const std::set<size_t>& target_routes);

// Framework VND principal
Solution VND(const Data& data, Solution start, std::mt19937& rng,
             std::vector<NeighborhoodFunction> neighborhoods);

// Vizinhanças separadas por tipo
std::vector<NeighborhoodFunction> GetInterRouteNeighborhoods();
std::vector<NeighborhoodFunction> GetIntraRouteNeighborhoods();
std::vector<NeighborhoodFunction> GetAllNeighborhoods();
#endif