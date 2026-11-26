<script setup lang="ts">
useSeoMeta({
  title: 'Playground — Zlang in the browser',
  description: 'Compile Zlang to C and C to WebAssembly entirely in the browser.',
})
</script>

<template>
  <div class="playground-page site-container py-12 lg:py-16">
    <PlaygroundBackdrop />

    <p class="eyebrow text-center">Playground</p>
    <h1 class="mt-3 text-center text-3xl font-bold tracking-tight text-ink sm:text-4xl">
      Zlang in the browser
    </h1>
    <p class="mx-auto mt-4 max-w-2xl text-center leading-7 text-slate-600">
      The compiler, clang, lld and the sysroot run as WebAssembly in this page. Nothing is sent to a server.
    </p>

    <div class="mt-8">
      <PlaygroundPanel />
    </div>

    <p class="mx-auto mt-6 text-center text-xs text-slate-500">
      Toolchain artifacts are distributed with their upstream notices in
      <code>site/THIRD_PARTY_NOTICES.md</code>.
    </p>
  </div>
</template>

<style scoped>
.playground-page {
  position: relative;
  isolation: isolate;
}

.playground-page > :not(.playground-backdrop) {
  position: relative;
  z-index: 1;
}
</style>
