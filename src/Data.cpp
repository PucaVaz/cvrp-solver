#include "Data.h"

Data::Data(int argc, char *argv[]) :
    cost_matrix(nullptr) {
    
    if (argc < 2) {
        cout << "Parâmetros insuficientes para o JP-Bike!\n";
        cout << " ./jp_bike_solver [Instancia] [--feastest]" << endl;
        exit(1);
    }
    
    if (argc > 3) {
        cout << "Muitos parâmetros para o JP-Bike!\n";
        cout << " ./jp_bike_solver [Instancia] [--feastest]" << endl;
        exit(1);
    }
    
    instance_name = argv[1];
    nb_of_par = argc;
    n_stations = -1;
}

Data::~Data() {
    if (cost_matrix != nullptr) {
        for (int i = 0; i <= n_stations; i++) {  // +1 para incluir o depósito
            delete[] cost_matrix[i];
        }
        delete[] cost_matrix;
    }
}

void Data::read() {
    ifstream inFile(instance_name, ios::in);
    
    if (!inFile) {
        cout << "Arquivo de instância JP-Bike não encontrado: " << instance_name << endl;
        exit(1);
    }
    
    // Lê linha 1: n (número de estações)
    inFile >> n_stations;
    
    // Lê linha 2: m (número de veículos disponíveis)
    inFile >> m_vehicles;
    
    // Lê linha 3: Q (capacidade de cada veículo)
    inFile >> vehicle_capacity;
    
    // Pula linha vazia (linha 4)
    string empty_line;
    getline(inFile, empty_line); // Consome o \n após Q
    getline(inFile, empty_line); // Linha vazia
    
    // Lê linha 5: demandas das estações (qi)
    // qi > 0: estação de coleta (bicicletas devem ser removidas)
    // qi < 0: estação de entrega (bicicletas devem ser entregues)
    string demands_line;
    getline(inFile, demands_line);
    
    // Faz parsing das demandas
    istringstream demands_stream(demands_line);
    long long demand;
    while (demands_stream >> demand) {
        station_demands.push_back(demand);
    }
    
    // Verifica se o número de demandas confere com n
    if (static_cast<int>(station_demands.size()) != n_stations) {
        cout << "Erro: número de demandas (" << station_demands.size() 
             << ") não confere com número de estações (" << n_stations << ")" << endl;
        exit(1);
    }
    
    // Pula linha vazia (linha 6)
    getline(inFile, empty_line);
    
    // Aloca matriz de custos de viagem cij
    // Inclui depósito (índice 0) + n estações (índices 1 a n)
    int matrix_size = n_stations + 1;
    cost_matrix = new double*[matrix_size];
    for (int i = 0; i < matrix_size; i++) {
        cost_matrix[i] = new double[matrix_size];
    }
    
    // Lê a matriz de custos de viagem (linhas 7+)
    // Matriz é (n+1) x (n+1) incluindo depósito
    for (int i = 0; i < matrix_size; i++) {
        for (int j = 0; j < matrix_size; j++) {
            inFile >> cost_matrix[i][j];
        }
    }
    
    inFile.close();
}

void Data::printDistanceMatrix() {
    cout << "Matriz de Custos de Viagem JP-Bike (cij):" << endl;
    cout << "Depósito = vértice 0, Estações = vértices 1 a " << n_stations << endl;
    for (int i = 0; i <= n_stations; i++) {
        for (int j = 0; j <= n_stations; j++) {
            cout << cost_matrix[i][j] << " ";
        }
        cout << endl;
    }
}

void Data::printJPBikeInstanceData() {
    cout << "=== Instância do Sistema JP-Bike ===" << endl;
    cout << "Arquivo: " << getInstanceName() << endl;
    cout << "Parâmetros do Problema de Rebalanceamento:" << endl;
    cout << "  Número de estações (n): " << n_stations << endl;
    cout << "  Número de veículos (m): " << m_vehicles << endl;
    cout << "  Capacidade dos veículos (Q): " << vehicle_capacity << endl;
    cout << "Demandas das Estações:" << endl;
    for (int i = 0; i < station_demands.size(); i++) {
        cout << "  Estação " << (i+1) << ": " << station_demands[i];
        if (station_demands[i] > 0) {
            cout << " (coleta: " << station_demands[i] << " bicicletas a remover)";
        } else if (station_demands[i] < 0) {
            cout << " (entrega: " << llabs(station_demands[i]) << " bicicletas a entregar)";
        } else {
            cout << " (balanceada)";
        }
        cout << endl;
    }
}

string Data::getInstanceName() {
    string::size_type loc = instance_name.find_last_of(".", instance_name.size());
    string::size_type loc2 = instance_name.find_last_of("/", instance_name.size());
    
    string instance;
    
    if (loc != string::npos) {
        instance.append(instance_name, loc2 + 1, loc - loc2 - 1);
    } else {
        instance.append(instance_name, loc2 + 1, instance.size());
    }
    
    return instance;
}
