# SFL for robotics — intuition → code mapping
---
Inspired by the group-foraging behavior of frogs, SFL splits the population into memeplexes that repeatedly pull the worst frog toward better exemplars (or the global best), with periodic shuffling to mix information.

## 1) Intuition: groups of frogs looking for “food”
![Memeplex intuition](/docs/memeplex_food.png)

- Each **frog** = one candidate solution (a real vector `x ∈ ℝ^R`).
- The **food** bubble = the **objective minimum** (in this repo we start with **Sphere**).
- Frogs are split into small **memeplexes** (dashed areas). Inside each memeplex the **worst** frog is pushed toward better ones so the whole group “learns”.
- In the code:  
  `ordenar_por_fitness(...)` → best first → `dividir_en_memeplexes(...)` (round-robin) → local updates in `procesar_memeplex(...)`.

---

## 2) From sorted population to memeplexes (and multi-objective note)
![Sorted → fronts → memeplexes](/docs/sfl_pareto_partition.png)

- We **sort** the population by fitness (best at the top).  
- Then we assign frogs **round-robin** into `M` memeplexes so each group gets a mix of qualities.  
- In multi-objective robotics (e.g., time **and** energy), you’d form **fronts** (non-dominated sorting) before splitting. **This repo is single-objective** (Sphere), so we use simple fitness sorting; the hook to change is the sort criterion.

Code hooks:
- Sorting: `ordenar_por_fitness(x, fit, P)`
- Split: `dividir_en_memeplexes(mem_idx, mem_sz)`

---

## 3) What actually moves a frog (sub-memeplex update)
![Local update of worst frog](/docs/sfl_submemeplex_update.png)

Inside each memeplex we run `J_MAX` **local steps**. Each step tries to **repair the worst frog**:

1. **Jump toward memeplex best:**  
   $\mathbf{x}' = \mathbf{x}_w + r\cdot(\mathbf{x}_b-\mathbf{x}_w),\quad r \sim U(0,1)$
   and we **cap each component** by `D_MAX` and **clamp** to `[X_MIN, X_MAX]`.  
2. If that didn’t help, **jump toward the global best** $\mathbf{x}_g\$.  
3. If still worse, **random re-init** (diversity refresh).

Code path:
- `procesar_memeplex(...)` → finds **best/worst** → calls `intentar_mejorar_peor(best, worst)`
- Step capping: `D_MAX`
- Bounds: `clamp_en_rango(...)`
- Global best tracking: `actualizar_mejor_global_posicion(...)`

---

## 4) Reading the results obtained in sphere.md using sfl.c

You can take a look into sphere.md and see the results obtained. Here is a brief description and discussion about the solution acquired.

### What the results shown
- **Early phase (Iter 0 → ~20):** a big drop. Memeplexes drag their **worst** frogs toward strong exemplars, so fitness collapses quickly.
- **Middle phase (~20 → ~60):** steady tightening. The population concentrates near the origin as local repairs keep working.
- **Late phase (~60 → 100):** a small **plateau** around $\(4.72\times10^{-4}\)$. Jumps are capped by `D_MAX` and the population is already clustered; improvements are tiny and may be hidden by print precision.

### Sanity-check (does the position match the fitness?)
Take the final best position and square–sum its coordinates (Sphere = \(\sum x_i^2\)):

- \( (-0.00244627)^2 \approx 5.98\times10^{-6} \)
- \( (-0.00497473)^2 \approx 2.47\times10^{-5} \)
- \( (-0.02099504)^2 \approx 4.41\times10^{-4} \)

Adding them:
\[
5.98\times10^{-6} + 2.47\times10^{-5} + 4.41\times10^{-4}
\;\approx\; 4.72\times10^{-4},
\]
which is exactly the printed **Best fitness \(\approx 0.00047152\)**. That confirms the run is behaving correctly on Sphere (global minimum at \(\mathbf{x}=\mathbf{0}\)).

### If you want to squeeze a bit more at the end
- Increase **local effort**: `J_MAX` from 5 → 7–10.  
- Add a few individuals: `P` from 20 → ~30.  
- Reduce print noise: print every 10 iters, or add an early-stop like `if (best_fitness < 1e-5) break;`.

> Note: exact numbers vary with the RNG seed; the convergence shape (big early drop → gentle tail) should remain the same.


