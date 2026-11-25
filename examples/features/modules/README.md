# modules

Shows a project with two user modules resolved through `z.yaml`. The `lib` alias points at `./lib`,
so `main.z` imports `base` from `@lib/config` and `add` from `@lib/math`. Both modules are assembled
into one generated C file with canonical names; the function and variable are emitted once.

The program prints `42`.
