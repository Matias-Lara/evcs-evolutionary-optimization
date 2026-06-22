#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H

#include "Instance.h"
#include "Solution.h"
#include <vector>
#include <random>

using namespace std;

class GeneticAlgorithm {
public:
    const Instance* instance;
    int pop_size;
    double crossover_rate;
    double mutation_rate;
    int max_generations;   // tope duro de generaciones (red de seguridad)
    int patience;          // criterio de parada: generaciones sin mejora antes de cortar
    unsigned seed_used;    // semilla efectivamente usada por el RNG (reproducibilidad)
    int generations_run;   // generaciones realmente ejecutadas (<= max_generations)

    vector<Solution> population;
    Solution best_solution;

    // Constructor que recibe la instancia y los parámetros del algoritmo.
    // seed >= 0 -> semilla fija (reproducible); seed < 0 -> semilla basada en el reloj.
    // patience -> generaciones consecutivas sin mejora tras las cuales se corta
    // (solo si ya hay solucion factible); max_gen es el tope duro de seguridad.
    GeneticAlgorithm(const Instance* inst, int p_size, double c_rate, double m_rate, int max_gen, long seed = -1, int patience = 200);

    // Método principal que ejecuta el bucle evolutivo
    void run();

private:
    // Generador de números aleatorios
    mt19937 rng;

    // Métodos internos
    void initializePopulation();
    
    // Selección mediante Ruleta (Roulette Wheel Selection)
    Solution rouletteWheelSelection();
    
    // Cruce Uniforme (Uniform Crossover)
    void uniformCrossover(const Solution& parent1, const Solution& parent2, Solution& child1, Solution& child2);
    
    // Mutación de inversión de bits (Bit-flip Mutation)
    void bitFlipMutation(Solution& sol);
};

#endif // GENETIC_ALGORITHM_H
