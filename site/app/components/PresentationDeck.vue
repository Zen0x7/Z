<script setup lang="ts">
const emit = defineEmits<{ done: [] }>()

const slide = ref(0)
const total = 4

const presentationCode = String.raw`import { write } from "@std/io";

function main(): int32 {
    array[10] int32 fib = {0, 1};

    for (int32 index = 2; index < 10; index += 1) {
        fib[index] = fib[index - 1] + fib[index - 2];
    }

    foreach (value in fib) {
        write(value);
        write(" ");
    }
    write("\n");

    return 0;
}`

const buildSource = String.raw`array[10] int32 fib = {0, 1};

for (int32 index = 2; index < 10; index += 1) {
    fib[index] = fib[index - 1] + fib[index - 2];
}

foreach (value in fib) {
    write(value);
}`

const buildGenerated = String.raw`int32_t app_fn_main(void) {
    app_array_fib_t app_local_fib = { { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } };

    for (int32_t app_local_index = 2; (app_local_index < 10); app_local_index = zlang_add_i32(app_local_index, 1)) {
        int32_t zlang_gen_read_3 = app_local_fib.data_[(uint32_t)zlang_sub_i32(app_local_index, 1)];
        int32_t zlang_gen_read_4 = app_local_fib.data_[(uint32_t)zlang_sub_i32(app_local_index, 2)];
        app_local_fib.data_[(uint32_t)app_local_index] = zlang_add_i32(zlang_gen_read_3, zlang_gen_read_4);
    }

    for (uint32_t zlang_gen_foreach_5 = ZLANG_ZERO; zlang_gen_foreach_5 < 10; ++zlang_gen_foreach_5) {
        int32_t app_local_value = app_local_fib.data_[zlang_gen_foreach_5];
        zlang_std_io_write_i32(app_local_value);
        zlang_std_io_write_string(zlang_gen_literal_0);
    }

    zlang_std_io_write_string(zlang_gen_literal_1);

    return 0;
}`

const buildStage = ref(0)
let buildTimers: ReturnType<typeof setTimeout>[] = []

function playBuild() {
  for (const timer of buildTimers) clearTimeout(timer)
  buildTimers = []
  buildStage.value = 0
  buildTimers.push(
    setTimeout(() => { buildStage.value = 1 }, 2000),
    setTimeout(() => { buildStage.value = 2 }, 4800),
    setTimeout(() => { buildStage.value = 3 }, 6200),
    setTimeout(() => { buildStage.value = 4 }, 7000),
  )
}

watch(() => slide.value, (value) => {
  if (value === 2) playBuild()
  else for (const timer of buildTimers) clearTimeout(timer)
})

onUnmounted(() => {
  for (const timer of buildTimers) clearTimeout(timer)
})

const slideNames = ['Intro', 'Source', 'Build', 'Numbers']

const metrics = [
  { name: 'fibonacci.z', compile: '1.35 ms', run: '0.45 ms', compilerRam: '4.8 MB', runRam: '1.2 MB' },
  { name: 'pi.z', compile: '1.34 ms', run: '1.17 ms', compilerRam: '4.8 MB', runRam: '1.1 MB' },
  { name: 'bubble_sort.z', compile: '1.49 ms', run: '0.47 ms', compilerRam: '4.7 MB', runRam: '1.3 MB' },
]

const nextLabels = [
  'Show me the syntax with an example',
  'Let’s build it',
  'What about performance?',
  'I’m ready to continue',
]

const nextLabel = computed(() => nextLabels[slide.value])
const skipLabel = computed(() => slide.value === 0 ? 'I already know Zlang' : 'I see enough — I wanna use it.')
const showSkip = computed(() => slide.value < total - 1)

function goTo(index: number) {
  slide.value = index
}

function next() {
  if (slide.value < total - 1) {
    slide.value += 1
    return
  }
  emit('done')
}

function previous() {
  if (slide.value > 0) slide.value -= 1
}

function onKeydown(event: KeyboardEvent) {
  if (event.key === 'ArrowRight' || event.key === ' ') {
    event.preventDefault()
    next()
  } else if (event.key === 'ArrowLeft') {
    previous()
  } else if (event.key === 'Escape') {
    emit('done')
  }
}

onMounted(() => {
  window.addEventListener('keydown', onKeydown)
})

onUnmounted(() => {
  window.removeEventListener('keydown', onKeydown)
})
</script>

<template>
  <div class="deck" role="dialog" aria-modal="true" aria-label="Zlang introduction">
    <div class="deck-chrome">
      <img src="/favicon.svg" alt="" width="28" height="28">
      <span class="deck-chrome__label">Zlang · introduction</span>
      <span class="deck-chrome__spacer" />
      <ThemeToggle />
      <button v-if="showSkip" type="button" class="deck-skip" @click="emit('done')">
        {{ skipLabel }}
      </button>
    </div>

    <div class="deck-progress" aria-label="Presentation steps">
      <button
        v-for="(name, index) in slideNames"
        :key="name"
        type="button"
        class="deck-progress__item"
        :aria-label="`Go to ${name}`"
        :class="[
          index === slide && 'deck-progress__item--on',
          index < slide && 'deck-progress__item--done',
        ]"
        @click="goTo(index)"
      />
    </div>

    <Transition name="deck-slide" mode="out-in">
      <section v-if="slide === 0" key="intro" class="deck-slide deck-slide--center deck-slide--intro">
        <TraceBackground />
        <TranspilePipe class="deck-reveal" style="--delay: 120ms" />
        <h1 class="deck-title deck-title--words">
          <span class="deck-word" style="--word-delay: 220ms">An</span>
          <span class="deck-word deck-word--accent deck-word--coral" style="--word-delay: 290ms">expressive</span>
          <span class="deck-word" style="--word-delay: 360ms">language</span>
          <span class="deck-word" style="--word-delay: 430ms">that</span>
          <span class="deck-word" style="--word-delay: 500ms">can</span>
          <span class="deck-word" style="--word-delay: 570ms">be</span>
          <span class="deck-word deck-word--accent deck-word--amber" style="--word-delay: 640ms">transpiled</span>
          <span class="deck-word" style="--word-delay: 710ms">into</span>
          <span class="deck-word deck-word--accent deck-word--teal" style="--word-delay: 780ms">C.</span>
        </h1>
        <p class="deck-reveal deck-lead" style="--delay: 980ms">
          Let me show you the whole path. It takes less than a minute.
        </p>
      </section>

      <section v-else-if="slide === 1" key="source" class="deck-slide deck-slide--source">
        <FibonacciBackdrop />
        <FibonacciFlowers />
        <div class="deck-slide__content">
          <p class="deck-reveal deck-eyebrow" style="--delay: 260ms">Fibonacci</p>
          <h2 class="deck-reveal deck-title deck-title--sm" style="--delay: 380ms">
            Look at this example.
          </h2>
          <div class="deck-reveal deck-code" style="--delay: 560ms">
            <CodeBlock
              :code="presentationCode"
              lang="zlang"
              filename="fibonacci.z"
            />
          </div>
          <p class="deck-reveal deck-caption" style="--delay: 760ms">
            The sequence behind sunflower seeds and pinecones, in fixed memory.
          </p>
        </div>
      </section>

      <section v-else-if="slide === 2" key="build" class="deck-slide deck-slide--build">
        <AstBackdrop />
        <div class="deck-slide__content">
          <p class="deck-reveal deck-eyebrow" style="--delay: 60ms">Building</p>
          <h2 class="deck-reveal deck-title deck-title--sm" style="--delay: 140ms">
            Give me a moment.
          </h2>

          <div class="build-loading">
            <Transition name="loading-swap" mode="out-in">
              <div v-if="buildStage === 0" key="environment" class="build-loading__row">
                <span class="build-loading__icon"><ProjectFolderIcon /></span>
                <span>Creating a virtual environment...</span>
              </div>
              <div v-else-if="buildStage === 1" key="files" class="build-loading__row">
                <span class="build-loading__icon"><FileTypeIcon extension=".z" tone="z" /></span>
                <span>Creating virtual folders and Zlang source files...</span>
              </div>
              <div v-else-if="buildStage === 2" key="compiling" class="build-loading__row build-loading__row--done">
                <span class="build-loading__icon"><FileTypeIcon extension=".c" tone="c" /></span>
                <span>Compiling... takes about 0.003 seconds.</span>
              </div>
              <div v-else key="result" class="build-loading__row build-loading__row--done">
                <span class="build-loading__icon"><SuccessIcon /></span>
                <span>Here is the result.</span>
              </div>
            </Transition>
          </div>

          <Transition name="deck-slide">
            <div v-if="buildStage === 4" class="build-result">
              <CodeBlock
                :code="buildGenerated"
                lang="c"
                filename="main.c"
                source-url="https://github.com/Zen0x7/Z/blob/master/examples/algorithms/fibonacci/main.c"
                side-note="Scroll to read the entire code"
              />
            </div>
          </Transition>

          <p v-if="buildStage === 4" class="deck-caption">
            Dynamic signed indices become explicit uint32_t casts before checked access.
          </p>
        </div>
      </section>

      <section v-else key="numbers" class="deck-slide deck-slide--center deck-slide--numbers">
        <MetricBarsBackdrop />
        <p class="deck-reveal deck-eyebrow" style="--delay: 60ms">Measured here, just now</p>
        <h2 class="deck-reveal deck-title deck-title--sm" style="--delay: 140ms">
          Let the numbers talk for him.
        </h2>

        <div class="metrics-grid">
          <article
            v-for="(metric, index) in metrics"
            :key="metric.name"
            class="metric-card deck-reveal"
            :style="{ '--delay': `${260 + index * 140}ms` }"
          >
            <h3 class="metric-card__name">{{ metric.name }}</h3>
            <div class="metric-card__rows">
              <div><span>Compile time</span><strong>{{ metric.compile }}</strong></div>
              <div><span>Runtime duration</span><strong>{{ metric.run }}</strong></div>
              <div><span>Compile RAM usage</span><strong>{{ metric.compilerRam }}</strong></div>
              <div><span>Runtime RAM usage</span><strong>{{ metric.runRam }}</strong></div>
            </div>
          </article>
        </div>

        <p class="deck-caption deck-reveal" style="--delay: 760ms">
          <span class="metric-note-full">Reference metrics collected on AMD Ryzen 9 3900X · 12 cores / 24 threads · up to 4.67 GHz.</span>
          <span class="metric-note-compact">AMD Ryzen 9 3900X · 12 cores / 24 threads · 4.67 GHz max</span>
        </p>
      </section>
    </Transition>

    <div class="deck-actions">
      <button
        v-if="slide > 0"
        type="button"
        class="btn-secondary"
        @click="previous"
      >
        Back
      </button>
      <button type="button" class="btn-primary" @click="next">
        {{ nextLabel }}
        <span aria-hidden="true">→</span>
      </button>
    </div>
  </div>
</template>
