# Sistema JP-Bike - Parser de Instâncias de Rebalanceamento

Este parser lê instâncias do problema de rebalanceamento de bicicletas do sistema JP-Bike conforme especificação dos professores Teobaldo Bulhões e Gilberto Farias da UFPB.

## Estrutura dos Arquivos

- `src/Data.h` - Arquivo de cabeçalho com definição da classe Data
- `src/Data.cpp` - Implementação do parser JP-Bike
- `src/main.cpp` - Programa de teste para demonstrar o parser
- `src/makefile` - Configuração de compilação

## Formato da Instância JP-Bike

O formato do arquivo de instância segue a especificação:
```
1 n                           # Número de estações
2 m                           # Número de veículos disponíveis  
3 Q                           # Capacidade de cada veículo
<param4>                      # Parâmetro adicional
5 <d1> <d2> ... <dn>         # Demandas das estações
<param6>                      # Parâmetro adicional
<id1> <c1,1> <c1,2> ...      # Matriz de custos linha 1
<id2> <c2,1> <c2,2> ...      # Matriz de custos linha 2
...                          # Linhas adicionais da matriz
```

## Interpretação das Demandas

- **qi > 0**: Estação de coleta (qi bicicletas devem ser removidas)
- **qi < 0**: Estação de entrega (|qi| bicicletas devem ser entregues)
- **qi = 0**: Estação balanceada

## Compilação

```bash
cd src
make
```

## Uso

```bash
./jp_bike_solver <arquivo_instancia>
```

Exemplo:
```bash
make test
# ou
../jp_bike_solver ../instances/instancia_pdf.txt
```

## Saída do Parser

O parser exibe:
- Nome da instância JP-Bike
- Parâmetros do problema de rebalanceamento (n, m, Q)
- Interpretação das demandas de cada estação
- Matriz de custos de viagem entre depósito e estações
- Verificação de balanceamento do sistema

## Exemplo de Instância

Para a instância do exemplo dos professores (n=7, m=4, Q=10):
```
1 7
2 4  
3 10
4
5 3 -6 4 -2 3 -6 -4
6
7 0 6 10 25 21 20 20 20
8 4 0 21 5 20 30 20 27
...
```

## Estrutura do Problema

- **Depósito**: Vértice 0, ponto de partida/chegada dos veículos
- **Estações**: Vértices 1 a n, locais com demandas de rebalanceamento
- **Objetivo**: Minimizar custo total das rotas respeitando capacidades
- **Restrições**: Capacidade dos veículos, atendimento completo das demandas

## Implementação

- Segue diretrizes de codificação C++ do projeto ILS-SP-CVRP
- Usa header guards tradicionais (`#ifndef`)
- Convenções de nomenclatura snake_case para variáveis
- Gerenciamento seguro de memória com RAII
- Interface consistente com outros parsers do projeto

Este parser serve como base para implementação de algoritmos de otimização para o problema de rebalanceamento do sistema JP-Bike.