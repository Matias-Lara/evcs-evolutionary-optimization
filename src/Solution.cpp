#include "Solution.h"
#include <cstdlib>

using namespace std;

Solution::Solution(const Instance* inst) : instance(inst) {
    chromosome.resize(instance->n_nodes, 0);
    cost = 0;
    penalty = 0;
    fitness = 0;
}

void Solution::randomize() {
    for (int i = 0; i < instance->n_nodes; ++i) {
        chromosome[i] = rand() % 2; // Arroja 0 o 1 aleatoriamente
    }
    evaluate();
}

void Solution::evaluate() {
    cost = 0;
    penalty = 0;
    int n = instance->n_nodes;

    // 1. Calcular Costo Total de Construccion (Funcion Objetivo Original)
    for (int i = 0; i < n; ++i) {
        if (chromosome[i] == 1) {
            cost += instance->nodes[i].cost;
        }
    }

    // Calcular Penalizaciones
    // Un peso muy grande para forzar al algoritmo a preferir siempre soluciones factibles
    double penalty_weight = 1e9; 

    // RESTRICCION 1: Accesibilidad (Segun el Informe/Paper)
    // x_i + x_j >= 1 para todo (i,j) en \bar{E}
    
    // NOTA: AQUI SE PODRIA PROGRAMAR LA RESTRICCIÓN RELAJADA DEL AYUDANTE DE ESTA MANERA:
    /*
    for (int i = 0; i < n; ++i) {
        if (chromosome[i] == 1) { // Si decido construir...
            bool connected = false;
            for (int j = 0; j < n; ++j) {
                // Busco si hay al menos otra estacion construida en el rango R
                if (i != j && instance->dist_matrix[i][j] <= instance->R && chromosome[j] == 1) {
                    connected = true; 
                    break;
                }
            }
            if (!connected) {
                penalty += penalty_weight; // Penalizo porque quedo aislada
            }
        }
    }
    */
    
    // Pero lo programamos tal cual esta en tu informe (modo estricto):
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (instance->isValidEdge(i, j)) { // Distancia entre alpha*R y R
                if (chromosome[i] == 0 && chromosome[j] == 0) {
                    penalty += penalty_weight; // Penalizo si NINGUNO de los dos se construye
                }
            }
        }
    }

    // RESTRICCION 2: Satisfaccion de Demanda
    // Suma de fi*xi >= Dj para cada zona j
    for (int j = 0; j < n; ++j) {
        double provided_capacity = 0;
        for (int i = 0; i < n; ++i) {
            // Evaluamos si la estacion 'i' puede proveer a la zona 'j'
            if (i == j || instance->isValidEdge(i, j)) { 
                if (chromosome[i] == 1) {
                    provided_capacity += instance->nodes[i].capacity; // Sumamos f_i
                }
            }
        }
        if (provided_capacity < instance->nodes[j].demand) {
            // Penalizacion proporcional a cuanta demanda falto por cubrir
            penalty += penalty_weight * (instance->nodes[j].demand - provided_capacity); 
        }
    }

    // RESTRICCION 3: Cobertura Urbana
    // Suma de sj*xj >= CityArea
    double total_coverage = 0;
    for (int j = 0; j < n; ++j) {
        if (chromosome[j] == 1) {
            total_coverage += instance->nodes[j].service_range; // Sumamos s_j
        }
    }
    if (total_coverage < instance->city_area) {
        penalty += penalty_weight * (instance->city_area - total_coverage);
    }

    // RESTRICCION 4: Zonas Remotas
    // Al menos 1 estacion construida en el conjunto T
    int remote_count = 0;
    for (int j = 0; j < n; ++j) {
        if (instance->nodes[j].is_remote == 1 && chromosome[j] == 1) {
            remote_count++;
        }
    }
    if (remote_count < 1) {
        penalty += penalty_weight;
    }

    // El fitness final que el algoritmo intentara minimizar
    // Un fitness menor siempre sera mejor.
    fitness = cost + penalty;
}
