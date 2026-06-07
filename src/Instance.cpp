#include "Instance.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>

using namespace std;

Instance::Instance(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("No se pudo abrir el archivo de instancia: " + filename);
    }

    string line;
    
    // 1. Leer parametros globales
    getline(file, line); // Saltar linea de texto: "n_nodos R alpha CityArea"
    file >> n_nodes >> R >> alpha >> city_area;
    nodes.resize(n_nodes);
    
    // 2. Leer atributos de cada nodo
    getline(file, line); // Consumir el salto de linea restante
    getline(file, line); // Saltar linea de texto: "ID_Nodo Costo_C Demanda_D..."
    
    for (int i = 0; i < n_nodes; ++i) {
        file >> nodes[i].id >> nodes[i].cost >> nodes[i].demand 
             >> nodes[i].capacity >> nodes[i].service_range >> nodes[i].is_remote;
    }
    
    // 3. Leer la matriz de distancias
    getline(file, line); // Consumir salto de linea
    getline(file, line); // Saltar linea de texto: "Matriz_Distancias"
    
    dist_matrix.assign(n_nodes, vector<double>(n_nodes, 0.0));
    for (int i = 0; i < n_nodes; ++i) {
        for (int j = 0; j < n_nodes; ++j) {
            file >> dist_matrix[i][j];
        }
    }
    
    // 4. Leer coordenadas X e Y
    getline(file, line); // Consumir salto de linea
    getline(file, line); // Saltar linea de texto: "Coordenadas"
    
    for (int i = 0; i < n_nodes; ++i) {
        file >> nodes[i].x >> nodes[i].y;
    }
    
    file.close();
}

// Implementación de la funcion para chequear si el arco pertenece a \bar{E}
bool Instance::isValidEdge(int i, int j) const {
    if (i == j) return false;
    double d = dist_matrix[i][j];
    // Retorna True si la distancia esta entre (alpha * R) y R
    return (d >= alpha * R) && (d <= R);
}
