#include "GeneticAlgorithm.h"
#include <iostream>
#include <algorithm>
#include <chrono>

using namespace std;

GeneticAlgorithm::GeneticAlgorithm(const Instance* inst, int p_size, double c_rate, double m_rate, int max_gen, long seed)
    : instance(inst), pop_size(p_size), crossover_rate(c_rate), mutation_rate(m_rate), max_generations(max_gen),
      best_solution(inst) {

    // Semilla: si se pasa una >= 0 se usa fija (reproducible); si no, se basa en el reloj.
    seed_used = (seed >= 0) ? (unsigned)seed
                            : (unsigned)std::chrono::system_clock::now().time_since_epoch().count();
    rng = mt19937(seed_used);

    // Se inicializa la mejor solución con un fitness gigantesco (peor caso)
    best_solution.fitness = 1e18;
}

void GeneticAlgorithm::initializePopulation() {
    population.clear();
    for (int i = 0; i < pop_size; ++i) {
        Solution sol(instance);
        sol.randomize(rng);
        sol.evaluate();
        population.push_back(sol);
        
        // Guardamos si es la mejor hasta ahora
        if (sol.fitness < best_solution.fitness) {
            best_solution = sol;
        }
    }
}

Solution GeneticAlgorithm::rouletteWheelSelection() {
    // Al ser un problema de MINIMIZACIÓN, la ruleta debe favorecer a los que tienen MENOR fitness.
    // Una forma clásica es invertir el fitness.
    
    double max_fitness = 0;
    for (const auto& sol : population) {
        if (sol.fitness > max_fitness) {
            max_fitness = sol.fitness;
        }
    }
    
    double sum_inverted_fitness = 0;
    vector<double> inverted_fitness(pop_size);
    for (int i = 0; i < pop_size; ++i) {
        // Distancia hacia el máximo fitness. Le sumamos 1 para asegurar que nunca haya probabilidad 0.
        inverted_fitness[i] = (max_fitness - population[i].fitness) + 1.0; 
        sum_inverted_fitness += inverted_fitness[i];
    }
    
    uniform_real_distribution<double> dist(0.0, sum_inverted_fitness);
    double rand_val = dist(rng);
    
    double partial_sum = 0;
    for (int i = 0; i < pop_size; ++i) {
        partial_sum += inverted_fitness[i];
        if (partial_sum >= rand_val) {
            return population[i];
        }
    }
    
    // Fallback por posible pérdida de precisión en doubles
    return population.back();
}

void GeneticAlgorithm::uniformCrossover(const Solution& parent1, const Solution& parent2, Solution& child1, Solution& child2) {
    uniform_real_distribution<double> dist(0.0, 1.0);
    
    // Determinamos si habrá cruce mediante la tasa de cruce
    if (dist(rng) < crossover_rate) {
        for (size_t i = 0; i < parent1.chromosome.size(); ++i) {
            // El cruce uniforme decide gen por gen de qué padre hereda cada hijo
            if (dist(rng) < 0.5) {
                child1.chromosome[i] = parent2.chromosome[i];
                child2.chromosome[i] = parent1.chromosome[i];
            } else {
                child1.chromosome[i] = parent1.chromosome[i];
                child2.chromosome[i] = parent2.chromosome[i];
            }
        }
    } else {
        // Si no hay cruce, los hijos son copias exactas de los padres
        child1 = parent1;
        child2 = parent2;
    }
}

void GeneticAlgorithm::bitFlipMutation(Solution& sol) {
    uniform_real_distribution<double> dist(0.0, 1.0);
    
    for (size_t i = 0; i < sol.chromosome.size(); ++i) {
        // La mutación se evalúa de manera independiente para cada bit del cromosoma
        if (dist(rng) < mutation_rate) {
            sol.chromosome[i] = 1 - sol.chromosome[i]; // Cambia 0 a 1, o 1 a 0
        }
    }
}

void GeneticAlgorithm::run() {
    cout << "--- Iniciando Algoritmo Genetico ---" << endl;
    initializePopulation();
    
    for (int gen = 0; gen < max_generations; ++gen) {
        vector<Solution> new_population;
        
        // ----- ELITISMO -----
        // 1. Encontrar al mejor individuo de la población actual
        Solution best_current = population[0];
        for (const auto& sol : population) {
            if (sol.fitness < best_current.fitness) {
                best_current = sol;
            }
        }
        
        // 2. Insertarlo directamente a la nueva generación (sobrevive intacto)
        new_population.push_back(best_current);
        
        // Actualizamos la mejor solución histórica si superamos nuestro record
        if (best_current.fitness < best_solution.fitness) {
            best_solution = best_current;
            cout << "Generacion " << gen << " | Nuevo mejor fitness: " << best_solution.fitness 
                 << " (Costo: " << best_solution.cost << ", Penalidad: " << best_solution.penalty << ")" << endl;
        }

        // ----- REPRODUCCIÓN -----
        // Rellenar el resto de la población de 2 en 2
        while (new_population.size() < (size_t)pop_size) {
            // Selección
            Solution parent1 = rouletteWheelSelection();
            Solution parent2 = rouletteWheelSelection();
            
            Solution child1(instance);
            Solution child2(instance);
            
            // Cruce (Intensificación)
            uniformCrossover(parent1, parent2, child1, child2);
            
            // Mutación (Exploración)
            bitFlipMutation(child1);
            bitFlipMutation(child2);
            
            // Evaluamos a los hijos luego de ser alterados
            child1.evaluate();
            child2.evaluate();
            
            // Los añadimos a la nueva población
            new_population.push_back(child1);
            if (new_population.size() < (size_t)pop_size) {
                new_population.push_back(child2);
            }
        }
        
        // Reemplazo generacional
        population = new_population;
    }
    
    cout << "--- Fin del Algoritmo Genetico ---" << endl;
    cout << "Mejor fitness global encontrado: " << best_solution.fitness << endl;
}
