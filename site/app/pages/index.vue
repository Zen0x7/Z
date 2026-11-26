<script setup lang="ts">
const INTRO_KEY = 'zlang.intro.v1'

const deckOpen = ref(false)
const deckRun = ref(0)

function finishDeck() {
  localStorage.setItem(INTRO_KEY, '1')
  deckOpen.value = false
  document.body.classList.remove('intro-active')
}

function replayDeck() {
  deckRun.value += 1
  deckOpen.value = true
  document.body.classList.add('intro-active')
}

onMounted(() => {
  if (!localStorage.getItem(INTRO_KEY)) {
    deckOpen.value = true
    document.body.classList.add('intro-active')
  }
})

onUnmounted(() => {
  document.body.classList.remove('intro-active')
})

const destinations = [
  {
    label: 'Quick start',
    icon: 'quick',
    to: '/docs/quick-start',
    note: 'Rebuild the environment and run one program.',
  },
  {
    label: 'Language guide',
    icon: 'language',
    to: '/docs/language',
    note: 'Syntax, declarations, loops, functions and errors.',
  },
  {
    label: 'Language specification',
    icon: 'spec',
    to: '/docs/specification',
    note: 'The normative contract for Zlang and its emitted C.',
  },
  {
    label: 'Standard library',
    icon: 'stdlib',
    to: '/docs/standard-library',
    note: 'Domains, imports and naming rules.',
  },
  {
    label: 'Compiler',
    icon: 'compiler',
    to: '/docs/compiler',
    note: 'Storage plan, pipeline and diagnostics.',
  },
  {
    label: 'Playground',
    icon: 'playground',
    to: '/playground',
    note: 'Compile and run Zlang in the browser.',
  },
  {
    label: 'Doxygen API',
    icon: 'doxygen',
    href: 'https://z-doxygen.iantorres.cl',
    note: 'Headers and functions generated from source comments.',
  },
  {
    label: 'Get the source',
    icon: 'source',
    href: 'https://github.com/Zen0x7/Z',
    note: 'Clone the repository and build from source.',
  },
]

useSeoMeta({
  title: 'Zlang — a language transpiled into pure C',
  description: 'Zlang is a language that can be transpiled into pure C. Start with the quick start, language guide, specification, playground or API reference.',
})
</script>

<template>
  <div class="home-page">
    <HomeBackdrop />
    <PresentationDeck v-if="deckOpen" :key="deckRun" @done="finishDeck" />

    <section class="site-container py-16 lg:py-24">
      <div class="flex flex-wrap items-end justify-between gap-6">
        <div class="max-w-2xl">
          <p class="eyebrow">Workspace</p>
          <h1 class="mt-3 text-4xl font-bold tracking-tight text-ink sm:text-5xl">
            Welcome back.
          </h1>
          <p class="mt-5 text-lg leading-8 text-slate-600">
            You already saw the presentation. Pick the thing you came for.
          </p>
        </div>
        <button type="button" class="btn-secondary btn--tiny" @click="replayDeck">
          Replay presentation
        </button>
      </div>

      <div class="mt-12 grid gap-4 md:grid-cols-2 lg:grid-cols-3">
        <template v-for="item in destinations" :key="item.label">
          <NuxtLink
            v-if="'to' in item"
            :to="item.to"
            class="workspace-card surface-card group relative flex flex-col justify-between overflow-hidden p-6 transition-colors hover:border-ink"
          >
            <WorkspaceIcon :name="item.icon" class="workspace-card__bg" />
            <div class="workspace-card__content">
              <h2 class="text-base font-semibold text-ink">{{ item.label }}</h2>
              <p class="mt-2 text-sm leading-6 text-slate-600">{{ item.note }}</p>
            </div>
            <span class="workspace-card__content mt-6 text-sm font-semibold text-ink transition group-hover:translate-x-1">
              Open →
            </span>
          </NuxtLink>
          <a
            v-else
            :href="item.href"
            target="_blank"
            rel="noopener noreferrer"
            class="workspace-card surface-card group relative flex flex-col justify-between overflow-hidden p-6 transition-colors hover:border-ink"
          >
            <WorkspaceIcon :name="item.icon" class="workspace-card__bg" />
            <div class="workspace-card__content">
              <h2 class="text-base font-semibold text-ink">{{ item.label }}</h2>
              <p class="mt-2 text-sm leading-6 text-slate-600">{{ item.note }}</p>
            </div>
            <span class="workspace-card__content mt-6 text-sm font-semibold text-ink transition group-hover:translate-x-1">
              Open ↗
            </span>
          </a>
        </template>
      </div>
    </section>

    <section class="site-container pb-16 lg:pb-24">
      <div class="border-t border-[color:var(--line)] pt-10">
        <p class="eyebrow">Playground</p>
        <h2 class="mt-3 text-2xl font-bold tracking-tight text-ink sm:text-3xl">
          Run Zlang here
        </h2>
        <p class="mt-3 max-w-2xl leading-7 text-slate-600">
          Write a program, compile it to C, link it to WebAssembly and run it without leaving the page.
        </p>
        <PlaygroundPanel />
      </div>
    </section>
  </div>
</template>
