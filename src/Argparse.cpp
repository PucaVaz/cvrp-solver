#include "Argparse.h"
#include <iostream>
#include <cstring>

void PrintUsage(const char* program_name) {
    std::cout << "Uso: " << program_name << " [opções]" << std::endl;
    std::cout << "Opções:" << std::endl;
    std::cout << "  --instance PATH       Caminho para arquivo da instância (obrigatório)" << std::endl;
    std::cout << "  --seed N             Semente para números aleatórios (padrão: 42)" << std::endl;
    std::cout << "  --constructive TIPO  Método construtivo: 'nearest' ou 'insertion' (padrão: nearest)" << std::endl;
    std::cout << "  --no-vnd             Desabilita VND, usa apenas heurística construtiva" << std::endl;
    std::cout << "  --out DIR            Diretório de saída (padrão: outputs/)" << std::endl;
    std::cout << "  --feastest           Executa testes de viabilidade" << std::endl;
    std::cout << "  --verbose            Exibe saída detalhada" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "Opções ILS:" << std::endl;
    std::cout << "  --ils                Executa metaheurística ILS (Iterated Local Search)" << std::endl;
    std::cout << "  --max-iter N         Número de iterações externas do ILS (padrão: 50)" << std::endl;
    std::cout << "  --max-iter-ils N     Iterações sem melhoria antes de re-iniciar (padrão: 150)" << std::endl;
    std::cout << "  --rcl-alpha-min F    Limite inferior para GRASP α (padrão: 0.1)" << std::endl;
    std::cout << "  --rcl-alpha-max F    Limite superior para GRASP α (padrão: 0.5)" << std::endl;
    std::cout << "  --perturb-strength K Intensidade base para perturbação (padrão: 2)" << std::endl;
    std::cout << "  --help               Exibe esta ajuda" << std::endl;
}

CLIOptions ParseCLI(int argc, char* argv[]) {
    CLIOptions opts;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--instance") == 0) {
            if (i + 1 < argc) {
                opts.instance_path = argv[++i];
            } else {
                std::cout << "Erro: --instance requer um caminho" << std::endl;
                PrintUsage(argv[0]);
                exit(1);
            }
        }
        else if (strcmp(argv[i], "--seed") == 0) {
            if (i + 1 < argc) {
                opts.seed = static_cast<unsigned int>(std::stoul(argv[++i]));
            } else {
                std::cout << "Erro: --seed requer um número" << std::endl;
                PrintUsage(argv[0]);
                exit(1);
            }
        }
        else if (strcmp(argv[i], "--constructive") == 0) {
            if (i + 1 < argc) {
                opts.constructive_method = argv[++i];
                if (opts.constructive_method != "nearest" && opts.constructive_method != "insertion") {
                    std::cout << "Erro: --constructive deve ser 'nearest' ou 'insertion'" << std::endl;
                    exit(1);
                }
            } else {
                std::cout << "Erro: --constructive requer um método" << std::endl;
                PrintUsage(argv[0]);
                exit(1);
            }
        }
        else if (strcmp(argv[i], "--no-vnd") == 0) {
            opts.use_vnd = false;
        }
        else if (strcmp(argv[i], "--out") == 0) {
            if (i + 1 < argc) {
                opts.output_dir = argv[++i];
                // Garante que termina com /
                if (!opts.output_dir.empty() && opts.output_dir.back() != '/') {
                    opts.output_dir += '/';
                }
            } else {
                std::cout << "Erro: --out requer um diretório" << std::endl;
                PrintUsage(argv[0]);
                exit(1);
            }
        }
        else if (strcmp(argv[i], "--feastest") == 0) {
            opts.run_feastest = true;
        }
        else if (strcmp(argv[i], "--verbose") == 0) {
            opts.verbose = true;
        }
        else if (strcmp(argv[i], "--ils") == 0) {
            opts.use_ils = true;
        }
        else if (strcmp(argv[i], "--max-iter") == 0) {
            if (i + 1 < argc) {
                opts.max_iter = std::stoi(argv[++i]);
                if (opts.max_iter <= 0) {
                    std::cout << "Erro: --max-iter deve ser maior que 0" << std::endl;
                    exit(1);
                }
            } else {
                std::cout << "Erro: --max-iter requer um número" << std::endl;
                PrintUsage(argv[0]);
                exit(1);
            }
        }
        else if (strcmp(argv[i], "--max-iter-ils") == 0) {
            if (i + 1 < argc) {
                opts.max_iter_ils = std::stoi(argv[++i]);
                if (opts.max_iter_ils <= 0) {
                    std::cout << "Erro: --max-iter-ils deve ser maior que 0" << std::endl;
                    exit(1);
                }
            } else {
                std::cout << "Erro: --max-iter-ils requer um número" << std::endl;
                PrintUsage(argv[0]);
                exit(1);
            }
        }
        else if (strcmp(argv[i], "--rcl-alpha-min") == 0) {
            if (i + 1 < argc) {
                opts.rcl_alpha_min = std::stod(argv[++i]);
                if (opts.rcl_alpha_min < 0.0 || opts.rcl_alpha_min > 1.0) {
                    std::cout << "Erro: --rcl-alpha-min deve estar entre 0.0 e 1.0" << std::endl;
                    exit(1);
                }
            } else {
                std::cout << "Erro: --rcl-alpha-min requer um número" << std::endl;
                PrintUsage(argv[0]);
                exit(1);
            }
        }
        else if (strcmp(argv[i], "--rcl-alpha-max") == 0) {
            if (i + 1 < argc) {
                opts.rcl_alpha_max = std::stod(argv[++i]);
                if (opts.rcl_alpha_max < 0.0 || opts.rcl_alpha_max > 1.0) {
                    std::cout << "Erro: --rcl-alpha-max deve estar entre 0.0 e 1.0" << std::endl;
                    exit(1);
                }
            } else {
                std::cout << "Erro: --rcl-alpha-max requer um número" << std::endl;
                PrintUsage(argv[0]);
                exit(1);
            }
        }
        else if (strcmp(argv[i], "--perturb-strength") == 0) {
            if (i + 1 < argc) {
                opts.perturb_strength = std::stoi(argv[++i]);
                if (opts.perturb_strength <= 0) {
                    std::cout << "Erro: --perturb-strength deve ser maior que 0" << std::endl;
                    exit(1);
                }
            } else {
                std::cout << "Erro: --perturb-strength requer um número" << std::endl;
                PrintUsage(argv[0]);
                exit(1);
            }
        }
        else if (strcmp(argv[i], "--help") == 0) {
            PrintUsage(argv[0]);
            exit(0);
        }
    }

    if (opts.instance_path.empty()) {
        std::cout << "Erro: Instância é obrigatória (--instance PATH)" << std::endl;
        PrintUsage(argv[0]);
        exit(1);
    }

    // Validação de parâmetros ILS
    if (opts.rcl_alpha_min > opts.rcl_alpha_max) {
        std::cout << "Erro: --rcl-alpha-min não pode ser maior que --rcl-alpha-max" << std::endl;
        exit(1);
    }

    return opts;
}