# Decisiones experimentales del barrido (scripts/run_sweep.py)

Justificación de qué se barre, qué se fija, y por qué — para usar al escribir
la sección de metodología/diseño experimental del informe.

> **Estado actual:** el barrido se SIMPLIFICÓ a 2 ejes (**alpha × pop**). La
> versión previa que barría también `gen` y `seed` como ejes (1000 corridas)
> quedó archivada en el tag `barrido-completo-v1` (recuperable con
> `git checkout barrido-completo-v1 -- run_sweep.py`).

## Qué se barre y por qué

**Alpha (0, .05, .10, .15, .20)** — no es un hiperparámetro del GA, es un
parámetro del *problema*: controla qué tan dura es la restricción R2 (anillo
de demanda [αR, R]). Es el eje experimental central del informe: mostrar el
trade-off costo↔factibilidad a medida que la instancia se endurece. No hay
"número correcto"; es la variable que define el experimento, no algo que se
optimiza.

**Tamaño de población (10, 20, 50, 100, 200)** — el único hiperparámetro del
GA que se barre. No hay tamaño óptimo universal para este problema (Goldberg,
Reeves). Hallazgo propio que lo justifica: a esfuerzo comparable, pop chico
(20) puede igualar o superar a pop grande (200) — solo se descubre barriendo,
no asumiendo un valor de la literatura.

Esos son los **dos únicos ejes**. Todo lo demás se fija o se trata como réplica.

## Seeds: réplica estadística, NO un eje

**Seeds (1-5)** — no es tuning ni un eje de barrido, es metodología estadística
obligatoria en metaheurísticas estocásticas (Eiben & Smith, *Introduction to
Evolutionary Computing*). El GA usa RNG en init, ruleta, cruce y mutación → la
misma config da resultados distintos por corrida. Por eso cada celda
(alpha, pop) se corre con 5 semillas y se **agrega** (media ± std, tasa de
factibilidad k/5). Sin réplica, reportar un solo número es anecdótico.

## Criterio de parada (reemplaza al barrido de generaciones)

Antes se barría `gen ∈ {500,1000,2000,4000}` como un eje. Se eliminó: en su
lugar el solver usa un **criterio de parada por estancamiento** (*stall /
stagnation*), práctica estándar en GAs (Eiben & Smith; en herramientas reales,
p.ej. `MaxStallGenerations` de MATLAB).

**Cómo funciona:** con elitismo el mejor fitness es monótono (nunca empeora),
así que basta un contador de generaciones consecutivas sin mejora estricta. Se
corta cuando el contador llega a `patience` (def. 200)... **pero solo si la
mejor solución ya es factible** (penalización = 0). Si todavía es infactible,
sigue hasta el **tope duro `max_gen`** (4000 en el barrido, 2000 en el CLI; red de seguridad). O sea:
para por estancamiento O por tope, lo que ocurra primero, y nunca abandona
mientras no haya encontrado al menos una solución válida.

**Por qué el fitness penalizado unifica las dos fases:** como
fitness = costo + penalización (λ=1e9), mientras es infactible "mejorar" = bajar
penalización (avanzar hacia factibilidad); una vez factible, "mejorar" = bajar
costo. Un solo contador cubre ambas fases sin lógica extra.

**Ventaja metodológica (clave para el informe).** Con parada temprana cada
corrida termina en una generación distinta → el "esfuerzo" varía por corrida.
Lejos de ser un problema, eso permite **medir la velocidad de convergencia**.
Pero obliga a comparar con cuidado: NO se compara solo el costo final, porque
una config puede llegar al mismo costo gastando mucho más. La moneda honesta
del esfuerzo computacional es:

> **evaluaciones de fitness = pop × generaciones**
>
> Es independiente del equipo y reproducible: dos PCs distintos dan el MISMO
> número para la misma corrida. `wall_s` (tiempo de reloj) también se registra,
> pero solo como dato secundario — depende de la máquina, el compilador y de
> cuántas corridas vayan en paralelo, así que NO sirve para comparar de forma
> justa. La comparación honesta se hace por evaluaciones, no por tiempo.

Ejemplo: pop=20 que converge en 900 gens = 18.000 evaluaciones; pop=200 en 1600
gens = 320.000 evaluaciones. Mismo costo final, ¡pero pop=200 trabajó ~18× más!
Un número de "costo final" suelto ocultaba eso. Por eso el CSV registra
`gen_alcanzado` y `evaluaciones`: la figura **costo final vs evaluaciones** (una
línea por pop) muestra quién converge más rápido y más barato — que es
justamente el hallazgo de "el esfuerzo, no la población, es la palanca".

### Corridas factibles vs. infactibles: NO mezclarlas al promediar

Una corrida puede terminar de dos formas muy distintas, y eso cambia qué
significa su número de evaluaciones:

1. **Encontró solución factible** (penalización 0) y cortó por estancamiento.
   Aquí `evaluaciones` = el esfuerzo real que le costó LLEGAR a la solución.
   Este número sí es comparable: mide velocidad de convergencia.

2. **Nunca encontró factible** y agotó el tope `max_gen`. Aquí `evaluaciones`
   siempre vale lo mismo (`pop × max_gen`), porque gastó TODO el presupuesto.
   Ese número NO mide "lo que costó resolver"; mide "se rindió tras gastarlo todo
   sin resolver". Es un techo fijo, no un esfuerzo de convergencia.

**Por qué importa:** si promedias `evaluaciones` (o `costo`) mezclando los dos
casos, el promedio no significa nada — estás mezclando "lo resolví en 18.000"
con "me rendí a las 320.000". Ejemplo concreto: en una celda (α, pop) con 5
seeds, si 3 fueron factibles y 2 no, promediar las 5 ensucia el resultado con
dos corridas que ni siquiera resolvieron el problema.

**Regla práctica para los gráficos** — para cada celda (α, pop) reportar DOS
cosas por separado:

- **Tasa de factibilidad**: cuántas de las 5 seeds llegaron a factible (p.ej.
  3/5). Responde "¿qué tan confiable es esta configuración?". Se calcula con la
  columna `factible`.
- **Costo y evaluaciones**: promediados SOLO sobre las corridas factibles de esa
  celda (filtrar `factible == "si"` antes de promediar). Responde "cuando
  resuelve, ¿qué tan bueno y qué tan rápido es?".

Así cada métrica dice una cosa limpia: la tasa de factibilidad habla de "qué tan
seguido resuelve"; el costo/evaluaciones habla de "qué tan bien y rápido resuelve
cuando resuelve". Nunca se contaminan entre sí.

## Qué NO se barre y por qué

**Mutación = 1/n (fijo)** — respaldo directo en literatura: Bäck (1993,
*Optimal Mutation Rates in Genetic Search*) y Mühlenbein (1992) muestran que
1/n (n = largo del cromosoma) es una tasa cuasi-óptima derivada
analíticamente, no arbitraria. Se fija "principista" porque ya está resuelto
por la teoría; barrerlo no aporta información nueva.

**Crossover = 0.9 (fijo)** — respaldo más débil que el de mutación: es
práctica estándar (rango típico 0.6–0.9 desde Goldberg/De Jong), no una
fórmula cerrada como 1/n. Se deja fijo por presupuesto experimental: barrer
todo explota combinatoriamente y el retorno marginal de afinar crossover es
bajo comparado con pop. Es una decisión práctica, no una afirmación científica
fuerte — vale la pena presentarla así en el informe en vez de sobre-venderla.

**Generaciones (gen)** — ya no es un parámetro a elegir: lo decide el criterio
de parada (ver arriba). Solo quedan dos constantes: `patience` (paciencia ante
estancamiento) y `max_gen` (tope duro de seguridad), ambas reportables en la
metodología.

## Por qué los parámetros del GA son configurables por CLI (y no hardcodeados)

1. **No recompilar por cada combinación.** El sweep cambia pop/seed en cada
   corrida; recompilar el binario por cada combinación sería absurdamente lento
   y arriesga que un error de compilación arruine el barrido a la mitad.
2. **Reproducibilidad real.** El seed usado se imprime en el output
   (`seed_used`) — cualquier corrida puntual se puede repetir exactamente con
   el mismo comando. Si estuviera hardcodeado en el código fuente, "reproducir"
   implicaría rastrear qué commit tenía qué valor.
3. **Separación de responsabilidades.** El binario C++ es un solver genérico;
   el diseño del experimento (qué barrer, en qué rango) vive en `scripts/run_sweep.py`.
   El solver no necesita saber nada del experimento, y el barrido se puede
   rediseñar sin tocar ni recompilar el C++.

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
