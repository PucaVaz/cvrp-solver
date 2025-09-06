#ifndef DATA_H
#define DATA_H

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <math.h>

using namespace std;

class Data {
public:
    Data(int argc, char *argv[]);
    ~Data();

    void read();
    void printDistanceMatrix();
    void printJPBikeInstanceData();
    
    // Métodos de acesso aos dados da instância JP-Bike
    inline int getNumStations() { return n_stations; }
    inline int getNumVehicles() { return m_vehicles; }
    inline int getVehicleCapacity() { return vehicle_capacity; }
    inline double getTravelCost(int i, int j) { return cost_matrix[i][j]; }
    inline double **getCostMatrix() { return cost_matrix; }
    inline double getStationDemand(int i) { return i < station_demands.size() ? station_demands[i] : 0; }
    inline vector<double> getAllDemands() { return station_demands; }
    
    string getInstanceName();

private:
    string instance_name;
    int nb_of_par;
    
    // Parâmetros do problema JP-Bike conforme especificação
    int n_stations = 0;        // Número de estações (linha "1 n")
    int m_vehicles = 0;        // Número de veículos disponíveis (linha "2 m") 
    int vehicle_capacity = 0;  // Capacidade Q de cada veículo (linha "3 Q")
    int param4 = 0;           // Parâmetro linha "4"
    int param6 = 0;           // Parâmetro linha "6"
    
    // Demandas das estações: qi > 0 (coleta), qi < 0 (entrega)
    vector<double> station_demands;
    
    // Matriz de custos de viagem cij entre depósito/estações
    double **cost_matrix;
};

#endif
