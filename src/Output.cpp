#include "Output.h"
#include <fstream>
#include <iostream>
#include <filesystem>

void WriteSolution(const std::string& path, const Data& data, const Solution& solution) {
    // Cria diretório de saída se não existir
    std::filesystem::path filepath(path);
    std::filesystem::create_directories(filepath.parent_path());

    std::ofstream outFile(path);
    if (!outFile) {
        std::cout << "Erro: Não foi possível criar arquivo de saída: " << path << std::endl;
        return;
    }

    // Linha 1: Custo total
    outFile << solution.total_cost << std::endl;

    // Linha 2: Número de veículos utilizados
    outFile << solution.routes.size() << std::endl;

    // Linhas seguintes: Rotas (cada uma iniciando e terminando com 0)
    for (const auto& route : solution.routes) {
        for (size_t i = 0; i < route.nodes.size(); ++i) {
            outFile << route.nodes[i];
            if (i < route.nodes.size() - 1) {
                outFile << " ";
            }
        }
        outFile << std::endl;
    }

    outFile.close();
    std::cout << "Solução salva em: " << path << std::endl;
}