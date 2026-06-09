#ifndef INSTANCE_H
#define INSTANCE_H

#include <string>
#include <vector>

using namespace std;

// Estructura para almacenar todos los datos de un nodo (candidato a estación)
struct Node {
    int id;
    double cost;
    double demand;
    double capacity;      // Corresponde a f_i 
    double service_range; // Corresponde a s_i 
    int is_remote;        // 1 si es remoto (conjunto T), 0 si no
    double x;
    double y;
};

class Instance {
public:
    // Parametros globales del problema
    int n_nodes;
    double R;
    double alpha;
    double city_area;
    
    // Listas para almacenar la informacion
    vector<Node> nodes;
    vector<vector<double>> dist_matrix;

    // Constructor que carga el archivo automaticamente
    Instance(const string& filename);
    
    // Funcion de ayuda matematica: verifica si un par de nodos (i, j) 
    // pertenece al conjunto de aristas validas E segun el paper
    bool isValidEdge(int i, int j) const;
};

#endif // INSTANCE_H
