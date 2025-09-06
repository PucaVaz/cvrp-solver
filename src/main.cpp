#include "Data.h"

int main(int argc, char *argv[]) {
    Data *data = new Data(argc, argv);
    
    cout << "Carregando instância do sistema JP-Bike..." << endl;
    data->read();
    
    cout << "\n";
    data->printJPBikeInstanceData();
    
    cout << "\n=== Matriz de Custos de Viagem ===" << endl;
    data->printDistanceMatrix();
    
    cout << "\n=== Informações para Algoritmo de Rebalanceamento ===" << endl;
    cout << "Total de localidades (depósito + estações): " << (data->getNumStations() + 1) << endl;
    cout << "Soma total das demandas: ";
    double total_demand = 0;
    for (int i = 0; i < data->getNumStations(); i++) {
        total_demand += data->getStationDemand(i);
    }
    cout << total_demand << endl;
    if (abs(total_demand) < 1e-6) {
        cout << "✓ Sistema balanceado: soma das demandas = 0" << endl;
    } else {
        cout << "⚠ Sistema desbalanceado: diferença = " << total_demand << endl;
    }
    
    delete data;
    
    return 0;
}
