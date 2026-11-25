# La primera visita

La persona abre el vínculo. Si es la primera vez en ese navegador, la pantalla completa se convierte en una presentación. No hay página debajo esperando: hay una secuencia de diapositivas que pide atención.

Un `localStorage` recuerda si la persona ya la vio. Si ya la vio, entra directo al workspace. Si quiere verla otra vez, aprieta **Replay presentation**.

La presentación termina con **Continue to Zlang** o **Skip**. Ambos cierran el deck y guardan la marca. Después aparece otra homepage: no la de la presentación, sino la de alguien que ya sabe qué es Zlang y quiere entrar a trabajar.

## Diapositivas

### 01 · Intro

Aparece el logo del sitio, quieto, sobre el color del tema activo. Debajo:

> An expressive language that can be transpiled into C.

Y una frase mínima:

> Let me show you the whole path. It takes less than a minute.

No hay contador de carga. La diapositiva espera a que la persona aprete **Continue**.

### 02 · Source

La pantalla se llena con Fibonacci real del repositorio. Los colores hacen que la estructura se vea antes de leerse: `array` e `integer` en coral, `for` y `foreach` en ámbar, `fib`, `index` y `value` en crema, números en rojo, strings en verde.

Arriba:

> Does this look familiar?

Abajo:

> Ten slots. One loop. Nothing grows.

### 03 · Build

La diapositiva muestra la transformación como una terminal guiada:

```console
$ zc fibonacci.z
  read fibonacci.z
  checked ten slots
  wrote fibonacci.c
  built fibonacci

$ ./fibonacci
0 1 1 2 3 5 8 13 21 34
```

Las líneas entran de a una. No se finge una sesión remota; se cuenta el viaje con la salida real del ejemplo.

Título:

> It turns into C you can compile.

### 04 · Scale

Un número ocupa la pantalla:

```text
200,000
```

Debajo:

> steps in `pi.z`. Same fixed plan.

Y el resultado real:

```text
3.14167
```

La escala se muestra con magnitudes del repositorio. No se inventan segundos de compilación.

### 05 · Run

Cierre de la conversación:

> Where does this run?

> On your computer. Any system with a C compiler.

Botón final:

> Continue to Zlang

## Después del deck

La homepage cambia. Ya no necesita explicar el lenguaje ni repetir el ejemplo que la persona acaba de ver. Ahora ofrece puertas:

- Quick start — rebuild the environment and run one program.
- Language guide — syntax, declarations, loops, functions and errors.
- Full specification — the normative contract for emitted C.
- Standard library — domains, imports and naming rules.
- Compiler — storage plan, pipeline and diagnostics.
- Doxygen API — headers and functions generated from source comments.
- Get the source — clone the repository and build from source.

Encabezado:

> Back to Zlang.

Frase:

> You already saw the presentation. Pick the thing you came for.

Botón discreto:

> Replay presentation

## Controles del deck

- **Continue** avanza. En la última diapositiva cierra la presentación.
- **Skip** cierra en cualquier momento.
- **Back** retrocede.
- Flechas del teclado y `Escape` hacen lo mismo.
- Indicador de progreso de cinco segmentos.
- `prefers-reduced-motion` desactiva las animaciones de entrada.

## Reglas

- El deck es una escena, no un loader.
- Después del deck, la home es un panel de trabajo, no un pitch.
- Los detalles técnicos viven en los docs.
- Las cifras salen de los ejemplos o de benchmarks medidos.
- Si una frase necesita glosario, no pertenece al deck ni a la home.
