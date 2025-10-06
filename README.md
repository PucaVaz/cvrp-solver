# JP-Bike Rebalancing Solver

Fast, reproducible C++ solver for the static bicycle rebalancing problem, a Capacitated Vehicle Routing Problem. Includes constructive heuristics, local search (VND/RVND), and an ILS metaheuristic with GRASP construction.

## Highlights

- Clean CLI with sensible defaults and full verbosity mode
- Constructive methods: Nearest‑Feasible, Best‑Insertion
- Local search: VND (Relocate, Swap, 2‑Opt, Or‑Opt‑2) and RVND
- ILS: GRASP α in [αmin, αmax] + RVND + feasible perturbations
- Standardized input/output formats and ready‑to‑run scripts

## Quick Start

Requirements
- g++ with C++20, make
- Optional: `just`, `clang-format`, `clang-tidy`

Build
```bash
# using make
cd src && make -s

# or using just (optional)
just build
```

Run
```bash
./bin/apa_jpbike --instance instances/instancias_teste/n14_q30.txt --verbose
```

## CLI Reference

Constructive + optional VND (default)
- `--instance PATH`  path to instance file (required)
- `--seed N`  RNG seed (default: 42)
- `--constructive {nearest|insertion}`  constructive method (default: nearest)
- `--no-vnd`  disable VND (keep constructive only)
- `--out DIR`  output directory (default: `outputs/`)
- `--feastest`  run built‑in feasibility tests
- `--verbose`  detailed output (costs, routes, L0 suggested)
- `--help`  usage

ILS options
- `--ils`  run Iterated Local Search
- `--max-iter N`  outer ILS iterations (default: 50)
- `--max-iter-ils N`  no‑improve iterations before restart (default: 150)
- `--rcl-alpha-min F`  GRASP α lower bound (default: 0.1)
- `--rcl-alpha-max F`  GRASP α upper bound (default: 0.5)
- `--perturb-strength K`  base perturbation intensity (default: 2)

Examples
```bash
# simple run (nearest)
./bin/apa_jpbike --instance instances/instancias_teste/n14_q30.txt

# best-insertion with fixed seed
./bin/apa_jpbike --instance instances/copa/instancia1.txt --seed 42 --constructive insertion

# constructive only (no VND)
./bin/apa_jpbike --instance instances/copa/instancia2.txt --no-vnd --constructive nearest

# ILS with custom parameters
./bin/apa_jpbike \
  --instance instances/instancias_teste/n40_q20.txt \
  --ils --max-iter 60 --max-iter-ils 200 \
  --rcl-alpha-min 0.1 --rcl-alpha-max 0.4 \
  --perturb-strength 3 --verbose

# custom output directory
./bin/apa_jpbike --instance instances/copa/instancia3.txt --out resultados/
```

## Instance Format (JP‑Bike)

Plain‑text, no labels:
```
n                             # number of stations
m                             # number of vehicles
Q                             # vehicle capacity
                              # blank line
d1 d2 ... dn                  # station demands
                              # blank line
c0,0 c0,1 c0,2 ... c0,n       # cost matrix row 0 (depot)
c1,0 c1,1 c1,2 ... c1,n       # cost matrix row 1
...                           # (n+1) x (n+1)
```
Demand meaning
- `qi > 0`: collect `qi` bikes from station i
- `qi < 0`: deliver `|qi|` bikes to station i
- `qi = 0`: station already balanced

## Output Format

Each `.out` file contains:
```
<total_cost>
<vehicles_used>
0 v1 v2 ... vk 0
0 u1 u2 ... uj 0
...
```

## Algorithms

- Constructive
  - Nearest‑Feasible: greedily visits the closest feasible next station
  - Best‑Insertion: inserts each station in the position with minimal cost increase
- Local Search
  - VND order: Relocate → Swap → 2‑Opt → Or‑Opt‑2
  - RVND: shuffle neighborhood order on every improvement
- ILS
  - Initial solution via GRASP (α sampled in `[αmin, αmax]`)
  - RVND as the local improvement procedure
  - Feasible perturbations: intra/inter‑route moves, swaps, partial 2‑Opt

## Feasibility Model (L0)

For a route `0 → … → 0`, let prefix sums accumulate node demands (depot = 0). Define
`L0_min = max(0, -min_prefix)`, `L0_max = Q - max_prefix`. The route is feasible iff
`L0_min ≤ L0 ≤ L0_max` for some initial load `L0`. The solver computes a suggested `L0`
and validates all routes and the full solution.

## Project Layout

```
src/
  Argparse.*        # CLI parsing
  Construction.*    # Greedy + GRASP builders, Solution type
  Feasibility.*     # L0 interval checks, route/solution validation
  ILS.*             # Iterated Local Search (GRASP + RVND + perturb)
  Instance.*        # Instance reader and data model
  LocalSearch.*     # VND/RVND neighborhoods
  Output.*          # .out writer
  main.cpp          # entry point
  makefile          # build script (bin/ and obj/)
instances/
  copa/             # Copa APA instances
  instancias_teste/ # test instances
scripts/
  run_copa_simple.sh, run_copa.sh, run_tests.sh, run_tests_ils.sh
bin/ obj/ outputs/  # build artifacts and default outputs
```

## Scripts

- `scripts/run_copa_simple.sh`  run Copa instances with nearest/insertion
- `scripts/run_copa.sh`  extended scenarios
- `scripts/run_tests.sh`  constructive + VND tests
- `scripts/run_tests_ils.sh`  ILS tests

## Troubleshooting

- “file not found”: check `--instance` path and run from repo root
- “instance required”: pass `--instance <path>`
- Use `--verbose` to inspect route feasibility and costs
- To reproduce results, fix `--seed`

## Acknowledgements

Specification by professors Teobaldo Bulhões and Gilberto Farias (UFPB)