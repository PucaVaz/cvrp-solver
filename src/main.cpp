#include "Data.h"
#include "Feasibility.h"
#include <string>
#include "test_relocate.h"
#include <iostream>

int main(int argc, char *argv[]) {
    Data *data = new Data(argc, argv);
    
    std::cout << "Carregando instancia do sistema JP-Bike..." << std::endl;
    data->read();
    
    cout << "\n";
    data->printJPBikeInstanceData();
    
    cout << "\n=== Matriz de Custos de Viagem ===" << endl;
    data->printDistanceMatrix();
    
    cout << "\n=== Informações para Algoritmo de Rebalanceamento ===" << endl;
    cout << "Total de localidades (depósito + estações): " << (data->getNumStations() + 1) << endl;
    cout << "Soma total das demandas: ";
    long long total_demand = 0;
    for (int i = 0; i < data->getNumStations(); i++) {
        total_demand += data->getStationDemand(i);
    }
    cout << total_demand << endl;
    if (total_demand == 0) {
        cout << "Sistema balanceado: soma das demandas = 0" << endl;
    } else {
        cout << "Sistema desbalanceado: diferença = " << total_demand << endl;
    }
    
    // Verifica flag --feastest
    bool run_feas_test = false;
    for (int i = 0; i < argc; ++i) {
        if (std::string(argv[i]) == "--feastest") {
            run_feas_test = true;
            break;
        }
    }
    
    if (run_feas_test) {
        cout << "\n=== Teste de Viabilidade (--feastest) ===" << endl;
        
        // Teste 1: Rota r1 = {0, 0} - deve ser viável (rota vazia)
        Route r1;
        r1.nodes = {0, 0};
        RouteFeasInfo info1 = CheckRouteFeasible(*data, r1);
        
        cout << "Teste 1 - Rota vazia {0, 0}:" << endl;
        cout << "  Viável: " << (info1.ok ? "SIM" : "NÃO") << endl;
        cout << "  Violação de capacidade: " << info1.cap_violation << endl;
        cout << "  Max prefix: " << info1.max_prefix << endl;
        cout << "  Min prefix: " << info1.min_prefix << endl;
        cout << "  L0_min: " << info1.L0_min << endl;
        cout << "  L0_max: " << info1.L0_max << endl;
        cout << "  L0 sugerido: " << info1.suggested_L0 << endl;
        
        // Teste 2: Rota r2 = {0, 12, 5, 7, 9, 6, 2, 4, 11, 0} - deve ser inviável para Q=20
        // Demandas: 12=+5, 5=+1, 7=-5, 9=-5, 6=-4, 2=-3, 4=-3, 11=-2
        // Prefixes: 5, 6, 1, -4, -8, -11, -14, -16
        // Range: 6-(-16) = 22 > Q=20 → inviável
        Route r2;
        r2.nodes = {0, 12, 5, 7, 9, 6, 2, 4, 11, 0};
        RouteFeasInfo info2 = CheckRouteFeasible(*data, r2);
        
        cout << "\nTeste 2 - Rota {0, 12, 5, 7, 9, 6, 2, 4, 11, 0}:" << endl;
        cout << "  Demandas críticas: 12=" << NodeDemand(*data, 12) 
             << ", 5=" << NodeDemand(*data, 5) << " (coletas); "
             << "7=" << NodeDemand(*data, 7) << ", 9=" << NodeDemand(*data, 9) << " (entregas)" << endl;
        cout << "  Viável: " << (info2.ok ? "SIM" : "NÃO") << endl;
        cout << "  Violação de capacidade: " << info2.cap_violation << endl;
        cout << "  Max prefix: " << info2.max_prefix << endl;
        cout << "  Min prefix: " << info2.min_prefix << endl;
        cout << "  L0_min: " << info2.L0_min << endl;
        cout << "  L0_max: " << info2.L0_max << endl;
        cout << "  L0 sugerido: " << info2.suggested_L0 << endl;
        cout << "  Capacidade do veículo (Q): " << data->getVehicleCapacity() << endl;
        
        cout << "\n=== Resultado dos Testes ===" << endl;
        cout << "Teste 1 (rota vazia): " << (info1.ok ? "PASSOU" : "FALHOU") << endl;
        cout << "Teste 2 (contra-exemplo): " << (!info2.ok ? "PASSOU (inviável como esperado)" : "FALHOU (deveria ser inviável)") << endl;
    }

    run_all_relocate_tests(*data);
    
    delete data;
    
    return 0;
}
