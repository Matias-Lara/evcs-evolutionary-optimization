# evcs-evolutionary-optimization

Algoritmo genético en C++ para el **Electric Vehicle Charging Station Placement
Problem (EVCSPP)**: elegir qué nodos candidatos alojan estaciones de carga
minimizando el costo total de construcción, sujeto a 4 restricciones
(accesibilidad, demanda, cobertura urbana y zonas remotas).

Modelo basado en Ou et al. (2025), *"Solving Optimal EV Charging Station
Placement Problem Using Digital Quantum Annealing"*.

## Requisitos

- `g++` con soporte C++11
- `make`
- Python 3 (solo para el barrido experimental; usa únicamente la stdlib)

En Windows: MSYS2 (shell **UCRT64**) con `g++` y `make` instalados vía `pacman`.

## Compilar

```
make          # genera el binario evcs_solver (evcs_solver.exe en Windows)
make clean    # limpia build/ y el binario
```

## Ejecutar

```
./evcs_solver <instancia> [pop] [crossover] [mutation] [max_gen] [seed] [patience]
```

Ejemplo:

```
./evcs_solver EVCS_Instancias/Instancias_pequenas/evcsSmall_N10_A0_S51.txt
```

Defaults: `pop=50`, `crossover=0.9`, `mutation=1/n`, `max_gen=2000`, `seed`=reloj,
`patience=200`. El GA corta antes de `max_gen` si lleva `patience` generaciones sin
mejora **y** ya encontró una solución factible (penalización 0); si sigue
infactible, corre hasta `max_gen`. La semilla efectiva se imprime para reproducir.

## Barrido experimental

```
python run_sweep.py    # grilla por defecto: alpha x pop x 5 seeds -> results/sweep_reales.csv
```

Ver [docs/decisiones_experimentales.md](docs/decisiones_experimentales.md) para la
justificación del diseño (qué se barre, qué se fija y por qué).

## Estructura

```
src/                 Código fuente C++ (GA, instancia, solución, main)
EVCS_Instancias/     Instancias (pequeñas, grandes, reales y variantes de alpha)
docs/                Decisiones experimentales
presentaciones/      Presentaciones (presentacion1, presentacion2)
run_sweep.py         Barrido experimental -> results/*.csv
Makefile
```
