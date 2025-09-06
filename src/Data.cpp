#include "Data.h"

Data::Data(int argc, char *argv[]) :
    cost_matrix(nullptr) {
    
    if (argc < 2) {
        cout << "Parâmetros insuficientes para o JP-Bike!\n";
        cout << " ./jp_bike_solver [Instancia]" << endl;
        exit(1);
    }
    
    if (argc > 2) {
        cout << "Muitos parâmetros para o JP-Bike!\n";
        cout << " ./jp_bike_solver [Instancia]" << endl;
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
    
    // Lê linha 1: "1 n" (número de estações)
    int line_id;
    inFile >> line_id >> n_stations;
    
    // Lê linha 2: "2 m" (número de veículos disponíveis)
    inFile >> line_id >> m_vehicles;
    
    // Lê linha 3: "3 Q" (capacidade de cada veículo)
    inFile >> line_id >> vehicle_capacity;
    
    // Lê linha 4: "4" (parâmetro adicional)
    inFile >> param4;
    
    // Lê linha 5: "5 d1 d2 ... dn" (demandas das estações)
    inFile >> line_id;
    
    // Lê demandas das estações até chegar na linha 6
    // qi > 0: estação de coleta (bicicletas devem ser removidas)
    // qi < 0: estação de entrega (bicicletas devem ser entregues)

    vector<double> temp_demands;
    
    // Lê todos os valores da linha 5 (demandas das estações)
    string line;
    getline(inFile, line); // Obtém o resto da linha 5
    
    // Faz o parsing das demandas da linha 5
    size_t pos = 0;
    while ((pos = line.find(" ")) != string::npos || !line.empty()) {
        if (pos != string::npos) {
            string token = line.substr(0, pos);
            if (!token.empty()) {
                station_demands.push_back(stod(token));
            }
            line.erase(0, pos + 1);
        } else {
            if (!line.empty()) {
                station_demands.push_back(stod(line));
            }
            break;
        }
    }
    
    // Lê linha 6: "6" (parâmetro adicional)
    inFile >> param6;
    
    // Determina a dimensão contando linhas da matriz de custos
    // A matriz inicia na linha 7 e inclui depósito + estações
    int matrix_dimension = 0;
    int first_matrix_id;
    inFile >> first_matrix_id; // Deve ser 7
    
    // Conta quantos valores há na primeira linha para determinar dimensão
    string matrix_line;
    getline(inFile, matrix_line);
    
    // Conta espaços para determinar dimensão da matriz
    matrix_dimension = 1; // Começa com 1 para o valor após o ID
    for (char c : matrix_line) {
        if (c == ' ') matrix_dimension++;
    }
    matrix_dimension--; // Ajusta para espaços extras
    
    // Reinicia posição do arquivo para ler matriz corretamente
    inFile.clear();
    inFile.seekg(0, ios::beg);
    
    // Pula para a seção da matriz (linhas 1-6)
    for (int i = 0; i < 6; i++) {
        getline(inFile, line);
    }
    
    // Aloca matriz de custos de viagem cij
    // Inclui depósito (índice 0) + n estações (índices 1 a n)
    cost_matrix = new double*[matrix_dimension];
    for (int i = 0; i < matrix_dimension; i++) {
        cost_matrix[i] = new double[matrix_dimension];
    }
    
    // Lê a matriz de custos de viagem
    for (int i = 0; i < matrix_dimension; i++) {
        int row_id;
        inFile >> row_id; // Lê identificador da linha (7, 8, 9, ...)
        
        for (int j = 0; j < matrix_dimension; j++) {
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
    cout << "  Parâmetro adicional 4: " << param4 << endl;
    cout << "  Parâmetro adicional 6: " << param6 << endl;
    cout << "Demandas das Estações:" << endl;
    for (int i = 0; i < station_demands.size(); i++) {
        cout << "  Estação " << (i+1) << ": " << station_demands[i];
        if (station_demands[i] > 0) {
            cout << " (coleta: " << station_demands[i] << " bicicletas a remover)";
        } else if (station_demands[i] < 0) {
            cout << " (entrega: " << abs(station_demands[i]) << " bicicletas a entregar)";
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
