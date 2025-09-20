#include "CLI.h"
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

    return opts;
}