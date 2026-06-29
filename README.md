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
python scripts/run_sweep.py    # grilla por defecto: alpha x pop x 5 seeds -> results/sweep_reales.csv
```

Ver [docs/decisiones_experimentales.md](docs/decisiones_experimentales.md) para la
justificación del diseño (qué se barre, qué se fija y por qué).

## Gráficos

Las figuras del informe se generan desde el CSV del barrido. A diferencia del
barrido, esto sí necesita dependencias externas (`pandas`, `matplotlib`), así que
se usa un entorno virtual:

```
python3 -m venv .venv
source .venv/bin/activate
pip install pandas matplotlib
python scripts/graficos.py        # genera figuras/*.png a partir de results/sweep_reales.csv
```

Produce cinco figuras en `figuras/`:

- `costo_vs_evaluaciones.png`: costo vs esfuerzo (evaluaciones) por tamaño de población.
- `mapa_solucion.png`: mapa de la mejor solución factible (estaciones construidas,
  candidatos no construidos y nodos remotos).
- `factibilidad_vs_alpha.png`: trade-off costo/factibilidad y la frontera de
  infactibilidad por construcción a partir de alpha=0.15.
- `costo_vs_pop.png`: afinamiento de pop_size, costo relativo según la población.
- `costo_y_esfuerzo_vs_pop.png`: costo y esfuerzo según la población para alpha=0.10.

## Estructura

```
src/                 Código fuente C++ (GA, instancia, solución, main)
EVCS_Instancias/     Instancias (pequeñas, grandes, reales y variantes de alpha)
docs/                Decisiones experimentales
presentaciones/      Presentaciones (presentacion1, presentacion2; son similares,
                     falta actualizar presentacion2 a futuro)
scripts/             Scripts de Python
  run_sweep.py       Barrido experimental -> results/*.csv
  graficos.py        Figuras del informe (lee el CSV) -> figuras/*.png
results/             Salida del barrido (sweep_reales.csv)
figuras/             Figuras generadas (5 PNG)
Makefile
```
