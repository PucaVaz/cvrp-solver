#ifndef RELOCATE_H
#define RELOCATE_H

#include <vector>
#include "Data.h"
#include "feasibility.h"

// Calcula o custo total de uma rota
double RouteCost(const Data &data, const Route &route);

// Executa ou simula um movimento Relocate
// Remove um nó de sol[r_from] e insere antes da posição to_idx em sol[r_to]
// Retorna true se o movimento for viável (capacidade ok) 
// delta_cost recebe a variação de custo (novo - antigo)
bool TryRelocate(const Data &data,
                 std::vector<Route> &sol,
                 int r_from, //índice da rota de origem sol
                 int from_idx, // indice do nó dentro de sol[r_from]
                 int r_to, // índice da rota de destino sol
                 int to_idx, 
                 double &delta_cost,
                 bool apply = true); // se apply == false, simula o movimento

#endif
