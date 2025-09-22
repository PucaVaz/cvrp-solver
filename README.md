# Sistema JP-Bike - Solver de Rebalanceamento

Este solver implementa algoritmos de otimização para o problema de rebalanceamento de bicicletas do sistema JP-Bike, conforme especificação dos professores Teobaldo Bulhões e Gilberto Farias da UFPB.

## Funcionalidades Implementadas

- **Parser de Instâncias**: Leitura completa do formato JP-Bike
- **Heurísticas Construtivas**: Nearest Neighbor e Best Insertion
- **Busca Local**: VND com 4 vizinhanças (Relocate, Swap, 2-Opt, Or-Opt-2)
- **Interface CLI**: Opções flexíveis para diferentes configurações
- **Saída Padronizada**: Arquivos de solução no formato exigido

## Estrutura dos Arquivos

- `src/Data.h/cpp` - Parser e estruturas de dados da instância
- `src/Feasibility.h/cpp` - Verificação de viabilidade das rotas
- `src/Constructive.h/cpp` - Heurísticas construtivas
- `src/VND.h/cpp` - Variable Neighborhood Descent
- `src/CLI.h/cpp` - Interface de linha de comando
- `src/IO.h/cpp` - Escrita de arquivos de solução
- `src/main.cpp` - Programa principal

## Formato da Instância JP-Bike

O arquivo de instância segue este formato (apenas os números, sem rótulos):
```
n                             # Número de estações
m                             # Número de veículos disponíveis
Q                             # Capacidade de cada veículo
                              # Linha em branco
d1 d2 ... dn                  # Demandas das estações
                              # Linha em branco
c0,0 c0,1 c0,2 ... c0,n       # Matriz de custos linha 0 (depósito)
c1,0 c1,1 c1,2 ... c1,n       # Matriz de custos linha 1
...                           # Demais linhas da matriz (n+1)x(n+1)
```

## Interpretação das Demandas

- **qi > 0**: Estação de coleta (qi bicicletas devem ser removidas)
- **qi < 0**: Estação de entrega (|qi| bicicletas devem ser entregues)
- **qi = 0**: Estação balanceada

## Compilação

```bash
cd src
make clean && make
```

Isso criará o executável `bin/apa_jpbike`.

## Uso

### Comandos Básicos
```bash
# Execução simples com método nearest neighbor
./bin/apa_jpbike --instance instances/instancias_teste/n14_q30.txt

# Com método de inserção e semente específica
./bin/apa_jpbike --instance instances/copa/instancia1.txt --seed 42 --constructive insertion

# Apenas heurística construtiva (sem VND)
./bin/apa_jpbike --instance instances/copa/instancia2.txt --no-vnd --constructive nearest

# Com diretório de saída personalizado
./bin/apa_jpbike --instance instances/copa/instancia3.txt --out resultados/
```

### Opções da Linha de Comando
- `--instance PATH` - Caminho para arquivo da instância (obrigatório)
- `--seed N` - Semente para números aleatórios (padrão: 42)
- `--constructive TIPO` - Método construtivo: 'nearest' ou 'insertion' (padrão: nearest)
- `--no-vnd` - Desabilita VND, usa apenas heurística construtiva
- `--out DIR` - Diretório de saída (padrão: outputs/)
- `--feastest` - Executa testes de viabilidade
- `--help` - Exibe ajuda completa

### Executar Todas as Instâncias Copa APA
```bash
./run_copa_simple.sh
```

## Formato da Saída

O solver gera arquivos `.out` com o formato exigido:
```
<custo_total>
<número_veículos>
0 v1 v2 ... vk 0
0 u1 u2 ... uj 0
...
```

Esses arquivos podem ser diretamente copiados para o formulário de submissão.

## Algoritmos Implementados

### Heurísticas Construtivas
1. **Nearest Neighbor**: Sempre seleciona a estação não visitada mais próxima que mantém viabilidade
2. **Best Insertion**: Insere cada estação na posição que resulta no menor aumento de custo

### Variable Neighborhood Descent (VND)
Aplicado na ordem: Relocate → Swap → 2-Opt → Or-Opt-2
- **Relocate**: Move um cliente entre/dentro de rotas
- **Swap**: Troca dois clientes entre rotas diferentes
- **2-Opt**: Inverte um segmento dentro de uma rota
- **Or-Opt-2**: Move uma cadeia de 2 clientes dentro da rota

## Verificação de Viabilidade

O sistema implementa verificação rigorosa de viabilidade considerando:
- Capacidade dos veículos (0 ≤ carga ≤ Q)
- Carga inicial necessária para cada rota
- Atendimento completo de todas as estações
- Rotas iniciando e terminando no depósito

## Estrutura do Problema

- **Depósito**: Vértice 0, ponto de partida/chegada dos veículos
- **Estações**: Vértices 1 a n, locais com demandas de rebalanceamento
- **Objetivo**: Minimizar custo total das rotas respeitando capacidades
- **Restrições**: Capacidade dos veículos, atendimento completo das demandas

## Resultados Típicos

O VND consegue melhorias significativas sobre as soluções construtivas:
- Instâncias pequenas (14 estações): 10-16% de redução de custo
- Instâncias médias (40 estações): 8-12% de redução de custo
- Todas as soluções mantêm 100% de viabilidade

## Implementação

- Segue diretrizes de codificação C++ do projeto ILS-SP-CVRP
- Usa header guards tradicionais (`#ifndef`)
- Convenções de nomenclatura snake_case para variáveis
- Gerenciamento seguro de memória com RAII
- Código determinístico com controle de sementes

Este solver fornece uma solução completa e robusta para o problema de rebalanceamento do sistema JP-Bike, adequado tanto para pesquisa quanto para aplicações práticas.