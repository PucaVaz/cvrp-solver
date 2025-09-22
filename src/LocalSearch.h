#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include <vector>
#include <functional>
#include <random>
#include "Construction.h"
#include "Instance.h"

// Função de vizinhança: retorna true se encontrou melhoria e aplicou
using NeighborhoodFunction = std::function<bool(const Data&, Solution&, std::mt19937&)>;

// Implementações das vizinhanças

/// Attempts one improving move of this type; applies it if found and returns true.
bool TwoOptStep(const Data& data, Solution& solution, std::mt19937& rng);

/// Attempts one improving move of this type; applies it if found and returns true.
bool RelocateStep(const Data& data, Solution& solution, std::mt19937& rng);

/// Attempts one improving move of this type; applies it if found and returns true.
bool SwapStep(const Data& data, Solution& solution, std::mt19937& rng);

/// Attempts one improving move of this type; applies it if found and returns true.
bool OrOpt2Step(const Data& data, Solution& solution, std::mt19937& rng);

// Framework VND principal
Solution VND(const Data& data, Solution start, std::mt19937& rng,
             std::vector<NeighborhoodFunction> neighborhoods);

// Ordem padrão das vizinhanças para VND
std::vector<NeighborhoodFunction> GetDefaultNeighborhoods();

#endif