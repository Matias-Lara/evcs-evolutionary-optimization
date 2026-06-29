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

## Diseño del barrido

Justificación de qué se varía, qué se fija y por qué (metodología experimental).

**Dos ejes barridos:**

- **alpha** (`0, 0.05, 0.10, 0.15, 0.20`): no es un hiperparámetro del GA sino un parámetro
  del *problema*; regula la dureza de la restricción de demanda (el anillo `[alpha*R, R]`). Es
  el eje central, porque muestra el trade-off costo/factibilidad a medida que la instancia se
  endurece (a partir de `alpha=0.15` se vuelve infactible por construcción).
- **Tamaño de población** (`10, 20, 50, 100, 200`): el único hiperparámetro del GA que se
  barre, porque no tiene un valor óptimo universal. El hallazgo es que una población pequeña
  iguala o supera a una grande gastando mucho menos esfuerzo.

**Semillas como réplica:** cada celda `(alpha, pop)` se corre con 5 semillas (no es un eje,
es metodología obligatoria en un algoritmo estocástico). Los resultados se agregan como media
y desviación estándar, y la tasa de factibilidad (k/5) se reporta por separado.

**Criterio de parada:** estancamiento (`patience` generaciones sin mejora), pero solo si la
mejor solución ya es factible; si sigue infactible, corre hasta el tope duro `max_gen`. Como
`fitness = costo + penalización`, un único contador cubre las dos fases (primero bajar la
penalización hasta ser factible, luego bajar el costo).

**Esfuerzo = evaluaciones = `pop x generaciones`.** Es la moneda honesta de comparación:
independiente de la máquina y reproducible, a diferencia del tiempo de reloj. Con parada
temprana cada corrida termina en una generación distinta, así que comparar solo el costo final
ocultaría que una configuración pudo gastar mucho más cómputo para llegar al mismo punto.

**Factibles vs. infactibles:** no se mezclan al promediar. El costo y las evaluaciones se
promedian solo sobre las corridas factibles (una corrida infactible agota `pop x max_gen` sin
resolver, y ese número no mide convergencia); la confiabilidad se mide aparte con la tasa de
factibilidad.

**Lo que NO se barre (se fija con respaldo en la literatura):**

- **Mutación = `1/n`**: tasa cuasi-óptima para cromosomas binarios (Bäck 1993; Mühlenbein
  1992), no un valor arbitrario.
- **Crossover = `0.9`**: práctica estándar (rango habitual entre 0.6 y 0.9); se fija por
  presupuesto experimental, no por una fórmula cerrada.
- **Generaciones**: ya no es un parámetro a elegir, lo decide el criterio de parada
  (`patience` + `max_gen`).

Los parámetros del GA son configurables por línea de comandos (no están hardcodeados): así no
hay que recompilar por cada combinación, cualquier corrida se reproduce con su semilla, y el
solver (genérico) queda separado del diseño del experimento (en `scripts/run_sweep.py`).
También se evaluó y descartó la inicialización *greedy* (compra velocidad de convergencia, no
calidad final confiablemente mejor), por lo que se mantiene la inicialización aleatoria.

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
.
├── src/                          # Código fuente C++ (solver)
│   ├── GeneticAlgorithm.cpp
│   ├── GeneticAlgorithm.h
│   ├── Instance.cpp
│   ├── Instance.h
│   ├── main.cpp
│   ├── Solution.cpp
│   └── Solution.h
├── EVCS_Instancias/              # Instancias del problema
│   ├── Instancia_real/           #   reales, N=100 (las usadas en el barrido)
│   ├── Instancias_alpha/         #   variantes con distinto alpha
│   ├── Instancias_grandes/
│   └── Instancias_pequenas/
├── scripts/
│   ├── run_sweep.py              # Barrido experimental  -> results/*.csv
│   └── graficos.py               # Figuras desde el CSV  -> figuras/*.png
├── results/
│   └── sweep_reales.csv          # Salida del barrido (se regenera)
├── figuras/                      # Figuras generadas (5 PNG)
├── docs_entregables/             # Entregables del curso
│   ├── e1_informe/               #   informe E1 (PDF)
│   ├── e2_informe/               #   informe E2 (fuente LaTeX + figuras)
│   ├── e2_instrucciones/         #   material provisto E2 (no versionado)
│   └── presentaciones/           #   p1 y p2 (beamer) + rubrica
├── instrucciones_generales/      # Reglas y paper de referencia (no versionado)
├── Makefile
└── README.md
```
