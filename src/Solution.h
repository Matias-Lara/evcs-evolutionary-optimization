#ifndef SOLUTION_H
#define SOLUTION_H

#include "Instance.h"
#include <vector>
#include <random>

using namespace std;

class Solution {
public:
    // El cromosoma: 1 = construir estación, 0 = no construir
    vector<int> chromosome; 
    
    // Puntero a la instancia para poder leer los datos (costos, distancias)
    const Instance* instance;
    
    double cost;
    double penalty;
    double fitness;

    Solution(const Instance* inst);
    
    // Funcion que calcula el costo y las penalizaciones
    void evaluate();
    
    // Genera una solucion inicial 100% aleatoria
    void randomize(mt19937& rng);
};

#endif // SOLUTION_H
