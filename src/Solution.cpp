#include "Solution.h"
#include <cstdlib>

using namespace std;

Solution::Solution(const Instance* inst) : instance(inst) {
    chromosome.resize(instance->n_nodes, 0);
    cost = 0;
    penalty = 0;
    fitness = 0;
}

void Solution::randomize(mt19937& rng) {
    uniform_int_distribution<int> bit(0, 1);
    for (int i = 0; i < instance->n_nodes; ++i) {
        chromosome[i] = bit(rng); // Arroja 0 o 1 aleatoriamente
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

    // Penalizaciones con un peso grande para descartar soluciones infactibles
    double penalty_weight = 1e9;

    // RESTRICCION 1: Accesibilidad
    // x_i + x_j >= 1 para todo par (i,j) de aristas validas
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
            // La estacion i aporta a la zona j si esta a distancia entre alpha*R
            // y R (isValidEdge ya excluye i == j, no se cuenta la propia zona)
            if (instance->isValidEdge(i, j)) {
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

    // Fitness a minimizar: costo mas penalizaciones
    fitness = cost + penalty;
}
