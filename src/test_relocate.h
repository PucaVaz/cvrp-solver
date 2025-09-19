#ifndef TEST_RELOCATE_H
#define TEST_RELOCATE_H

#include "Data.h"

/**
 * @brief Executa uma suíte completa de testes unitários para a função TryRelocate.
 * * Esta função testa vários cenários, incluindo movimentos viáveis e inviáveis,
 * movimentos dentro da mesma rota e entre rotas diferentes, casos de borda
 * como mover o depósito, e a precisão do cálculo do delta_cost.
 * * @param data Os dados da instância do problema, necessários para os cálculos de custo e viabilidade.
 */
void run_all_relocate_tests(Data& data);

#endif 