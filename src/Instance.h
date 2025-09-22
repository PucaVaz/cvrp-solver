#ifndef INSTANCE_H
#define INSTANCE_H

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>

class Data {
public:
    Data(int argc, char *argv[]);
    ~Data();

    void read();
    void printDistanceMatrix();
    void printJPBikeInstanceData();
    
    // Métodos de acesso aos dados da instância JP-Bike
    [[nodiscard]] inline int getNumStations() const noexcept { return n_stations; }
    [[nodiscard]] inline int getNumVehicles() const noexcept { return m_vehicles; }
    [[nodiscard]] inline int getVehicleCapacity() const noexcept { return vehicle_capacity; }
    [[nodiscard]] inline double getTravelCost(int i, int j) const { return cost_matrix[i][j]; }
    [[nodiscard]] inline double **getCostMatrix() { return cost_matrix; }
    [[nodiscard]] inline long long getStationDemand(int i) const { return i < station_demands.size() ? station_demands[i] : 0; }
    [[nodiscard]] inline std::vector<long long> getAllDemands() const { return station_demands; }
    
    [[nodiscard]] std::string getInstanceName() const;

private:
    std::string instance_name;
    int nb_of_par;
    
    // Parâmetros do problema JP-Bike conforme especificação
    int n_stations = 0;        // Número de estações (linha 1)
    int m_vehicles = 0;        // Número de veículos disponíveis (linha 2) 
    int vehicle_capacity = 0;  // Capacidade Q de cada veículo (linha 3)
    
    // Demandas das estações: qi > 0 (coleta), qi < 0 (entrega)
    std::vector<long long> station_demands;
    
    // Matriz de custos de viagem cij entre depósito/estações
    double **cost_matrix;
};

#endif
