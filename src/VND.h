#ifndef VND_H
#define VND_H

#include <vector>
#include <functional>
#include <random>
#include "Constructive.h"
#include "Data.h"

// Função de vizinhança: retorna true se encontrou melhoria e aplicou
using NeighborhoodFunction = std::function<bool(const Data&, Solution&, std::mt19937&)>;

// Implementações das vizinhanças

// Vizinhança intra-rota: inverte segmento (2-opt)
bool ImproveTwoOpt(const Data& data, Solution& solution, std::mt19937& rng);

// Vizinhança inter/intra-rota: move um cliente
bool ImproveRelocate(const Data& data, Solution& solution, std::mt19937& rng);

// Vizinhança inter-rota: troca clientes entre rotas
bool ImproveSwap(const Data& data, Solution& solution, std::mt19937& rng);

// Vizinhança intra-rota: move cadeia de 2 clientes (Or-opt)
bool ImproveOrOpt2(const Data& data, Solution& solution, std::mt19937& rng);

// Framework VND principal
Solution VND(const Data& data, Solution start, std::mt19937& rng,
             std::vector<NeighborhoodFunction> neighborhoods);

// Ordem padrão das vizinhanças para VND
std::vector<NeighborhoodFunction> GetDefaultNeighborhoods();

#endif