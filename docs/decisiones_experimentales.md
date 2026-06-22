# Decisiones experimentales del barrido (run_sweep.py)

Justificación de qué se barre, qué se fija, y por qué — para usar al escribir
la sección de metodología/diseño experimental del informe.

## Qué se barre y por qué

**Alpha (0, .05, .10, .15, .20)** — no es un hiperparámetro del GA, es un
parámetro del *problema*: controla qué tan dura es la restricción R2 (anillo
de demanda [αR, R]). Es el eje experimental central del informe: mostrar el
trade-off costo↔factibilidad a medida que la instancia se endurece. No hay
"número correcto"; es la variable que define el experimento, no algo que se
optimiza.

**Tamaño de población (10, 20, 50, 100, 200)** — sí se barre porque no hay
tamaño óptimo universal para este problema (Goldberg, Reeves). Hallazgo
propio que justifica haberlo barrido: a presupuesto de generaciones fijo,
pop=20 superó a pop=300 en N=200 (contraintuitivo). Solo se descubre
barriendo, no asumiendo un valor de la literatura.

**Generaciones (500, 1000, 2000, 4000)** — se barre junto con pop porque
interactúan: dado un pop fijo, mide cuánto presupuesto de generaciones hace
falta para llegar a factible (velocidad de convergencia).

**Seeds (1-5)** — no es tuning, es metodología estadística obligatoria en
metaheurísticas estocásticas (Eiben & Smith, *Introduction to Evolutionary
Computing*). El GA usa RNG en init, ruleta, cruce y mutación → la misma
config da resultados distintos por corrida. Sin repetir seeds, reportar un
solo número es anecdótico. 5 réplicas es compromiso entre robustez
estadística y costo computacional (el barrido completo ya es ~1000
corridas).

## Qué NO se barre y por qué

**Mutación = 1/n (fijo)** — respaldo directo en literatura: Bäck (1993,
*Optimal Mutation Rates in Genetic Search*) y Mühlenbein (1992) muestran que
1/n (n = largo del cromosoma) es una tasa cuasi-óptima derivada
analíticamente, no arbitraria. Se fija "principista" porque ya está resuelto
por la teoría; barrerlo no aporta información nueva.

**Crossover = 0.9 (fijo)** — respaldo más débil que el de mutación: es
práctica estándar (rango típico 0.6–0.9 desde Goldberg/De Jong), no una
fórmula cerrada como 1/n. Se deja fijo no porque esté "demostrado óptimo",
sino por presupuesto experimental: barrer alpha×pop×gen×seed×cross explota
combinatoriamente y el retorno marginal de afinar crossover es bajo
comparado con pop/gen. Es una decisión práctica, no una afirmación
científica fuerte — vale la pena presentarla así en el informe en vez de
sobre-vender el respaldo.

## Por qué los parámetros del GA son configurables por CLI (y no hardcodeados)

1. **No recompilar por cada combinación.** El sweep cambia
   pop/cross/mut/gen/seed en cada corrida; recompilar el binario por cada
   combinación sería absurdamente lento y arriesga que un error de
   compilación arruine el barrido a la mitad.
2. **Reproducibilidad real.** El seed usado se imprime en el output
   (`seed_used`) — cualquier corrida puntual se puede repetir exactamente
   con el mismo comando. Si estuviera hardcodeado en el código fuente,
   "reproducir" implicaría rastrear qué commit tenía qué valor.
3. **Separación de responsabilidades.** El binario C++ es un solver
   genérico; el diseño del experimento (qué barrer, en qué rango) vive en
   `run_sweep.py`. El solver no necesita saber nada del experimento, y el
   barrido se puede rediseñar sin tocar ni recompilar el C++.

## Decisión relacionada: no usar inicialización greedy

Discutido y descartado (sesión 13-14 jun 2026). La literatura (Kazimipour-Li
-Qin 2014; Reeves 1993; Krasnogor-Smith 2005) indica que el seeding greedy
compra velocidad de convergencia, no calidad final confiablemente mejor
(se diluye con suficientes generaciones), con riesgo de convergencia
prematura si se siembra demasiado. Aunque este problema es de cobertura
(R1-R4 todas "≥") y los greedy de cobertura tienen garantía ln(n)
(Chvátal 1979), se decidió mantener init aleatoria: agrega una dimensión de
complejidad sin beneficio claro en calidad final. Si se retoma en el futuro,
diseñarlo como greedy de descarte desde all-ones con 1 solo individuo
sembrado (resto aleatorio), no como eje de barrido.
