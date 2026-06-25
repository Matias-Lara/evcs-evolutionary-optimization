#!/usr/bin/env python3
"""
Genera las 3 figuras del informe a partir de results/sweep_reales.csv.

    1. costo_vs_evaluaciones.png : esfuerzo (evaluaciones) vs costo, una curva por
       alpha, marcando la pop en cada punto -> "el esfuerzo, no la poblacion".
    2. mapa_solucion.png         : ubicacion de las estaciones de la MEJOR solucion
       factible (min costo global) sobre el plano de la ciudad.
    3. factibilidad_vs_alpha.png : costo medio (factibles) y tasa de factibilidad
       en funcion de alpha -> el trade-off y el acantilado de factibilidad.

Requiere el venv con pandas+matplotlib:
    source .venv/bin/activate
    python scripts/graficos.py
"""
import glob
import os
from pathlib import Path

import matplotlib
matplotlib.use("Agg")            # backend sin display: solo escribe archivos
import matplotlib.pyplot as plt
import pandas as pd

ROOT = Path(__file__).resolve().parent.parent   # el script vive en scripts/, la raiz del repo esta un nivel arriba
CSV = ROOT / "results" / "sweep_reales.csv"
OUTDIR = ROOT / "figuras"
ALPHAS_FACTIBLES = ["0", "0.05", "0.10"]   # las que tienen solucion (ver auditoria)


def cargar():
    # alpha se lee como TEXTO crudo: si pandas lo infiere como float, "0.10"->0.1
    # y "0"->0.0, y dejarian de matchear con ALPHAS_FACTIBLES (= strings del .txt).
    df = pd.read_csv(CSV, dtype={"alpha": str})
    df["costo"] = pd.to_numeric(df["costo"], errors="coerce")
    df["evaluaciones"] = pd.to_numeric(df["evaluaciones"], errors="coerce")
    df["pop"] = pd.to_numeric(df["pop"], errors="coerce")
    return df


def find_instancia(basename):
    """Devuelve la ruta completa de un .txt de instancia por su nombre de archivo."""
    hits = glob.glob(str(ROOT / "EVCS_Instancias" / "**" / basename), recursive=True)
    if not hits:
        raise FileNotFoundError(basename)
    return hits[0]


def load_coords(fp):
    """Lee coordenadas (x,y) y flag remoto de cada nodo desde el .txt de instancia."""
    with open(fp) as f:
        lines = f.read().split("\n")
    it = iter(lines)
    next(it)                                  # titulo globales
    n = int(next(it).split()[0])
    next(it)                                  # titulo nodos
    remote = []
    for _ in range(n):
        p = next(it).split()
        remote.append(int(p[5]))
    next(it)                                  # titulo matriz
    for _ in range(n):
        next(it)                              # saltar matriz de distancias
    next(it)                                  # titulo coordenadas
    xs, ys = [], []
    for _ in range(n):
        x, y = next(it).split()
        xs.append(float(x)); ys.append(float(y))
    return xs, ys, remote


# ---------------------------------------------------------------------------
# Figura 1: costo vs evaluaciones (una curva por alpha, anotando pop)
# ---------------------------------------------------------------------------
def fig_costo_vs_evals(df):
    fac = df[df["factible"] == "si"]
    fig, ax = plt.subplots(figsize=(7, 5))
    for a in ALPHAS_FACTIBLES:
        sub = fac[fac["alpha"] == a]
        g = sub.groupby("pop").agg(ev=("evaluaciones", "mean"),
                                   co=("costo", "mean")).reset_index().sort_values("pop")
        # Normalizamos el costo por alpha (1.0 = el mejor costo medio de ESE alpha):
        # cada alpha vive en una escala distinta de costo, asi se comparan en un
        # mismo grafico y se ve el mensaje "el esfuerzo no compra calidad".
        g["co_norm"] = g["co"] / g["co"].min()
        ax.plot(g["ev"], g["co_norm"], "-o", label=f"alpha={a}")
        for _, row in g.iterrows():
            ax.annotate(f"pop {int(row['pop'])}", (row["ev"], row["co_norm"]),
                        textcoords="offset points", xytext=(6, 4), fontsize=8)
    ax.axhline(1.0, color="gray", ls="--", lw=0.8, alpha=0.6)
    ax.set_xscale("log")
    ax.set_xlabel("Evaluaciones de fitness  (pop x generaciones, escala log)")
    ax.set_ylabel("Costo relativo  (1.0 = mejor costo de cada alpha)")
    ax.set_title("Costo vs esfuerzo: mas poblacion gasta mas, no mejora")
    ax.legend()
    ax.grid(True, which="both", ls=":", alpha=0.5)
    fig.tight_layout()
    out = OUTDIR / "costo_vs_evaluaciones.png"
    fig.savefig(out, dpi=150); plt.close(fig)
    return out


# ---------------------------------------------------------------------------
# Figura 2: mapa de la mejor solucion factible
# ---------------------------------------------------------------------------
def fig_mapa(df):
    fac = df[df["factible"] == "si"]
    best = fac.loc[fac["costo"].idxmin()]
    ids = {int(x) for x in str(best["nodos_ids"]).split(";") if x != ""}
    xs, ys, remote = load_coords(find_instancia(best["instancia"]))

    fig, ax = plt.subplots(figsize=(7, 6))
    for i in range(len(xs)):
        construido = i in ids
        es_remoto = remote[i] == 1
        ax.scatter(xs[i], ys[i],
                   s=90 if construido else 45,
                   c="#1f77b4" if construido else "none",
                   edgecolors="#1f77b4" if construido else "#bbbbbb",
                   marker="*" if es_remoto else "o",
                   linewidths=1.2, zorder=3 if construido else 1)
    # leyenda manual
    from matplotlib.lines import Line2D
    leg = [
        Line2D([0], [0], marker="o", color="none", markerfacecolor="#1f77b4",
               markeredgecolor="#1f77b4", markersize=10, label="Estacion construida"),
        Line2D([0], [0], marker="o", color="none", markerfacecolor="none",
               markeredgecolor="#bbbbbb", markersize=8, label="Nodo candidato (no construido)"),
        Line2D([0], [0], marker="*", color="none", markerfacecolor="#1f77b4",
               markeredgecolor="#1f77b4", markersize=13, label="Nodo remoto (conjunto T)"),
    ]
    ax.legend(handles=leg, loc="best", fontsize=8)
    ax.set_xlabel("Coordenada X"); ax.set_ylabel("Coordenada Y")
    ax.set_title(f"Mejor solucion (alpha={best['alpha']}, pop={int(best['pop'])}, "
                 f"seed={int(best['seed'])})\n"
                 f"{len(ids)}/{int(best['nodos_total'])} estaciones | costo={best['costo']:.3e}")
    ax.set_aspect("equal", adjustable="datalim")
    ax.grid(True, ls=":", alpha=0.4)
    fig.tight_layout()
    out = OUTDIR / "mapa_solucion.png"
    fig.savefig(out, dpi=150); plt.close(fig)
    return out


# ---------------------------------------------------------------------------
# Figura 3: factibilidad y costo vs alpha
# ---------------------------------------------------------------------------
def fig_factibilidad_vs_alpha(df):
    alphas = sorted(df["alpha"].unique(), key=float)
    tasa, costo_med = [], []
    for a in alphas:
        sub = df[df["alpha"] == a]
        tasa.append(100.0 * (sub["factible"] == "si").mean())
        fac = sub[sub["factible"] == "si"]
        costo_med.append(fac["costo"].mean() if len(fac) else float("nan"))

    x = [float(a) for a in alphas]
    fig, ax1 = plt.subplots(figsize=(7, 5))
    c1 = "#d62728"
    ax1.bar(x, tasa, width=0.025, color=c1, alpha=0.35, label="Tasa de factibilidad")
    # Etiqueta explicita donde la factibilidad es 0: que no se confunda con "falta dato".
    for xi, ti in zip(x, tasa):
        if ti == 0:
            ax1.annotate("infactible\npor construccion", (xi, 2), ha="center", va="bottom",
                         fontsize=8, color=c1, rotation=90)
    ax1.set_xlabel("alpha (dureza del anillo de demanda)")
    ax1.set_ylabel("Tasa de factibilidad (%)", color=c1)
    ax1.tick_params(axis="y", labelcolor=c1)
    ax1.set_ylim(0, 105)

    ax2 = ax1.twinx()
    c2 = "#1f77b4"
    ax2.plot(x, costo_med, "-o", color=c2, label="Costo medio (factibles)")
    ax2.set_ylabel("Costo medio de soluciones factibles", color=c2)
    ax2.tick_params(axis="y", labelcolor=c2)

    ax1.set_title("Trade-off costo-factibilidad y acantilado en alpha=0.15")
    ax1.set_xticks(x)
    fig.tight_layout()
    out = OUTDIR / "factibilidad_vs_alpha.png"
    fig.savefig(out, dpi=150); plt.close(fig)
    return out


def main():
    if not CSV.exists():
        raise SystemExit(f"No existe {CSV}. Corre primero: python scripts/run_sweep.py")
    OUTDIR.mkdir(exist_ok=True)
    df = cargar()
    for fn in (fig_costo_vs_evals, fig_mapa, fig_factibilidad_vs_alpha):
        out = fn(df)
        print(f">> {os.path.relpath(out, ROOT)}")
    print(">> Listo. 3 figuras en figuras/")


if __name__ == "__main__":
    main()
