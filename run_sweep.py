#!/usr/bin/env python3
"""
Barrido experimental EVCSPP.

Ejes barridos : alpha (via archivos de instancia) x pop_size x max_generations x seeds
Fijos         : crossover_rate (robusto en la literatura),
                mutation_rate = 1/n  (principista, Muhlenbein 1992 / Back 1993)
Salida        : results/sweep_reales.csv  (una fila por corrida)

Solo usa la biblioteca estandar (no requiere pip install). El ploteo posterior
si usara pandas/matplotlib, pero la generacion del CSV corre tal cual.

Uso:
    make                       # compilar el solver primero
    python run_sweep.py        # grilla por defecto
    python run_sweep.py --jobs 6 --pops 10,20,50,100,200 --gens 500,1000,2000,4000 --seeds 1,2,3,4,5

En Windows (MSYS2 UCRT64 o consola normal con Python): mismo comando.
El barrido es vergonzosamente paralelo: --jobs = nro de nucleos (p.ej. 6 en un i5-10400F).
"""
import argparse
import csv
import os
import re
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

ROOT = Path(__file__).resolve().parent

# Instancias: 2 reales x 5 alphas. alpha=0 usa el original; el resto, las variantes.
INSTANCES = [
    "EVCS_Instancias/Instancia_real/evcsReal_N100_A0_S71.txt",
    "EVCS_Instancias/Instancias_alpha/real/evcsReal_N100_A0.05_S71.txt",
    "EVCS_Instancias/Instancias_alpha/real/evcsReal_N100_A0.10_S71.txt",
    "EVCS_Instancias/Instancias_alpha/real/evcsReal_N100_A0.15_S71.txt",
    "EVCS_Instancias/Instancias_alpha/real/evcsReal_N100_A0.20_S71.txt",
    "EVCS_Instancias/Instancia_real/evcsReal_N100_A0_S72.txt",
    "EVCS_Instancias/Instancias_alpha/real/evcsReal_N100_A0.05_S72.txt",
    "EVCS_Instancias/Instancias_alpha/real/evcsReal_N100_A0.10_S72.txt",
    "EVCS_Instancias/Instancias_alpha/real/evcsReal_N100_A0.15_S72.txt",
    "EVCS_Instancias/Instancias_alpha/real/evcsReal_N100_A0.20_S72.txt",
]

CSV_FIELDS = [
    "instancia", "n", "alpha", "pop", "crossover", "mutation", "gen", "seed",
    "fitness", "costo", "penalizacion", "nodos_construidos", "nodos_total",
    "factible", "wall_s",
]

RE_FIT = re.compile(r"Fitness Total:\s*(\S+)")
RE_COST = re.compile(r"Costo Base:\s*(\S+)")
RE_PEN = re.compile(r"Penalizacion:\s*(\S+)")
RE_NODES = re.compile(r"Nodos construidos:\s*(\d+)/(\d+)")


def find_solver():
    for name in ("evcs_solver", "evcs_solver.exe"):
        p = ROOT / name
        if p.exists():
            return str(p)
    sys.exit("ERROR: no se encontro el binario (evcs_solver). Corre 'make' primero.")


def read_header(fp):
    """Devuelve (n, alpha) leyendo la linea 2 del .txt de instancia."""
    with open(fp) as f:
        f.readline()                 # linea 1: titulos de columnas
        parts = f.readline().split() # linea 2: n R alpha CityArea
    return int(parts[0]), parts[2]


def run_one(solver, inst, pop, gen, seed, cross):
    fp = ROOT / inst
    n, alpha = read_header(fp)
    mut = 1.0 / n                     # mutacion = 1/n (principista)
    cmd = [solver, str(fp), str(pop), str(cross), f"{mut:.6f}", str(gen), str(seed)]

    t0 = time.perf_counter()
    out = subprocess.run(cmd, capture_output=True, text=True).stdout
    wall = time.perf_counter() - t0

    def grab(rx):
        m = rx.search(out)
        return m.group(1) if m else ""

    fit, cost, pen = grab(RE_FIT), grab(RE_COST), grab(RE_PEN)
    mn = RE_NODES.search(out)
    built, total = (mn.group(1), mn.group(2)) if mn else ("", "")

    try:
        feas = "si" if float(pen) == 0.0 else "no"
    except (ValueError, TypeError):
        feas = "ERROR"

    return {
        "instancia": os.path.basename(inst), "n": n, "alpha": alpha, "pop": pop,
        "crossover": cross, "mutation": f"{mut:.6f}", "gen": gen, "seed": seed,
        "fitness": fit, "costo": cost, "penalizacion": pen,
        "nodos_construidos": built, "nodos_total": total,
        "factible": feas, "wall_s": f"{wall:.2f}",
    }


def parse_list(s, conv):
    return [conv(x) for x in s.replace(",", " ").split()]


def main():
    ap = argparse.ArgumentParser(description="Barrido experimental EVCSPP -> CSV")
    ap.add_argument("--jobs", type=int, default=6, help="corridas en paralelo (= nucleos)")
    ap.add_argument("--pops", default="10,20,50,100,200")
    ap.add_argument("--gens", default="500,1000,2000,4000")
    ap.add_argument("--seeds", default="1,2,3,4,5")
    ap.add_argument("--cross", type=float, default=0.9)
    ap.add_argument("--out", default="results/sweep_reales.csv")
    args = ap.parse_args()

    solver = find_solver()
    pops = parse_list(args.pops, int)
    gens = parse_list(args.gens, int)
    seeds = parse_list(args.seeds, int)

    jobs = [(inst, pop, gen, seed)
            for inst in INSTANCES
            for pop in pops
            for gen in gens
            for seed in seeds]

    out_path = ROOT / args.out
    out_path.parent.mkdir(parents=True, exist_ok=True)

    print(f">> Barrido: {len(jobs)} corridas | {args.jobs} en paralelo | "
          f"crossover={args.cross} | mut=1/n")
    print(f">> pops={pops} gens={gens} seeds={seeds}")

    start = time.time()
    done = feas = 0
    # Escritura INCREMENTAL: abrimos el CSV antes del barrido y volcamos cada fila
    # apenas termina, con flush a disco. Asi, si el proceso se corta a mitad
    # (Ctrl-C, cierre, error externo) NO se pierde el trabajo ya hecho.
    with open(out_path, "w", newline="") as f, \
         ThreadPoolExecutor(max_workers=args.jobs) as ex:
        w = csv.DictWriter(f, fieldnames=CSV_FIELDS)
        w.writeheader()
        f.flush()

        futs = [ex.submit(run_one, solver, *job, args.cross) for job in jobs]
        for fut in as_completed(futs):
            row = fut.result()
            w.writerow(row)
            f.flush()
            done += 1
            feas += 1 if row["factible"] == "si" else 0
            if done % max(1, len(jobs) // 20) == 0 or done == len(jobs):
                print(f"   {done}/{len(jobs)} corridas listas...", flush=True)

    print(f">> Listo en {time.time() - start:.0f}s. {done} filas -> {out_path}")
    print(f">> Factibles: {feas} / {done}")


if __name__ == "__main__":
    main()