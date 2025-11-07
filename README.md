# SFL for robotics — intuition → code mapping (focused on how this repo works)
---

## 1) Intuition: groups of frogs looking for “food”
![Memeplex intuition](../docs/memeplex_food.png)

- Each **frog** = one candidate solution (a real vector `x ∈ ℝ^R`).
- The **food** bubble = the **objective minimum** (in this repo we start with **Sphere**).
- Frogs are split into small **memeplexes** (dashed areas). Inside each memeplex the **worst** frog is pushed toward better ones so the whole group “learns”.
- In the code:  
  `ordenar_por_fitness(...)` → best first → `dividir_en_memeplexes(...)` (round-robin) → local updates in `procesar_memeplex(...)`.

---

## 2) From sorted population to memeplexes (and multi-objective note)
![Sorted → fronts → memeplexes](../docs/sfl_pareto_partition.png)

- We **sort** the population by fitness (best at the top).  
- Then we assign frogs **round-robin** into `M` memeplexes so each group gets a mix of qualities.  
- In multi-objective robotics (e.g., time **and** energy), you’d form **fronts** (non-dominated sorting) before splitting. **This repo is single-objective** (Sphere), so we use simple fitness sorting; the hook to change is the sort criterion.

Code hooks:
- Sorting: `ordenar_por_fitness(x, fit, P)`
- Split: `dividir_en_memeplexes(mem_idx, mem_sz)`

---

## 3) What actually moves a frog (sub-memeplex update)
![Local update of worst frog](../docs/sfl_submemeplex_update.png)

Inside each memeplex we run `J_MAX` **local steps**. Each step tries to **repair the worst frog**:

1. **Jump toward memeplex best:**  
   \[
   \mathbf{x}' = \mathbf{x}_w + r\cdot(\mathbf{x}_b-\mathbf{x}_w),\quad r \sim U(0,1)
   \]
   and we **cap each component** by `D_MAX` and **clamp** to `[X_MIN, X_MAX]`.  
2. If that didn’t help, **jump toward the global best** \(\mathbf{x}_g\).  
3. If still worse, **random re-init** (diversity refresh).

Code path:
- `procesar_memeplex(...)` → finds **best/worst** → calls `intentar_mejorar_peor(best, worst)`
- Step capping: `D_MAX`
- Bounds: `clamp_en_rango(...)`
- Global best tracking: `actualizar_mejor_global_posicion(...)`

---

## 4) Reading the run like a roboticist

Your screenshot (Sphere, `R=3`, `P=20`, `M=5`, `J_MAX=5`, `MAX_ITERS=100`) shows:

