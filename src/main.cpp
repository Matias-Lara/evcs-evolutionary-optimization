#include <iostream>
#include <string>
#include <cstdlib>
#include "Instance.h"
#include "GeneticAlgorithm.h"

using namespace std;

int main(int argc, char* argv[]) {
    // Verificar si se pasó la ruta del archivo por argumento
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " <ruta_al_archivo_de_instancia>" << endl;
        return 1;
    }

    string file_path = argv[1];
    
    // 1. Leer y parsear la instancia
    Instance instance(file_path);

    cout << "========================================" << endl;
    cout << "Instancia cargada: " << file_path << endl;
    cout << "Nodos: " << instance.n_nodes << " | Area Ciudad: " << instance.city_area << endl;
    cout << "========================================" << endl;

    // 2. Parámetros del Algoritmo Genético
    // Se pueden pasar por linea de comandos para experimentar:
    //   ./evcs_solver <instancia> [pop_size] [crossover_rate] [mutation_rate] [max_gen] [seed] [patience]
    // Defaults: cross=0.9, mut=1/n, max_gen=2000, patience=200. seed < 0 (o ausente)
    // => semilla por reloj. max_gen es el tope duro; el GA corta antes si lleva
    // 'patience' generaciones sin mejora Y ya tiene una solucion factible.
    int pop_size = (argc > 2) ? atoi(argv[2]) : 50;
    double crossover_rate = (argc > 3) ? atof(argv[3]) : 0.9;
    double mutation_rate = (argc > 4) ? atof(argv[4]) : 1.0 / instance.n_nodes;
    int max_generations = (argc > 5) ? atoi(argv[5]) : 2000;
    long seed = (argc > 6) ? atol(argv[6]) : -1;
    int patience = (argc > 7) ? atoi(argv[7]) : 200;

    // Validacion basica de parametros: atoi/atof devuelven 0 ante texto no
    // numerico. Un pop_size < 1 dejaria la poblacion vacia y el GA accederia a
    // population[0] fuera de rango (crash). Clampeamos a valores sensatos.
    if (pop_size < 1) pop_size = 1;
    if (max_generations < 0) max_generations = 0;
    if (patience < 1) patience = 1;
    if (crossover_rate < 0.0) crossover_rate = 0.0;
    if (crossover_rate > 1.0) crossover_rate = 1.0;
    if (mutation_rate < 0.0) mutation_rate = 0.0;
    if (mutation_rate > 1.0) mutation_rate = 1.0;

    // 3. Crear el motor del Algoritmo Genético
    GeneticAlgorithm ga(&instance, pop_size, crossover_rate, mutation_rate, max_generations, seed, patience);
    cout << "Parametros GA -> pop:" << pop_size << " cross:" << crossover_rate
         << " mut:" << mutation_rate << " max_gen:" << max_generations
         << " patience:" << patience << " seed:" << ga.seed_used << endl;

    // 4. Ejecutar el Algoritmo
    ga.run();

    // 5. Mostrar la mejor solución encontrada
    cout << "========================================" << endl;
    cout << "--- MEJOR SOLUCION ENCONTRADA ---" << endl;
    cout << "Fitness Total: " << ga.best_solution.fitness << endl;
    cout << "Costo Base: " << ga.best_solution.cost << endl;
    cout << "Penalizacion: " << ga.best_solution.penalty << endl;
    cout << "----------------------------------------" << endl;
    cout << "Estaciones construidas en los nodos: ";
    bool first = true;
    for (size_t i = 0; i < ga.best_solution.chromosome.size(); ++i) {
        if (ga.best_solution.chromosome[i] == 1) {
            if (!first) cout << ", ";
            cout << i;
            first = false;
        }
    }
    cout << endl;
    int built = 0;
    for (int x : ga.best_solution.chromosome) built += x;
    cout << "Nodos construidos: " << built << "/" << instance.n_nodes << endl;
    cout << "========================================" << endl;

    return 0;
}
