<script setup lang="ts">
useSeoMeta({
  title: 'Doxygen API reference',
  description: 'Doxygen reference for libzlang and the zc compiler. The generated HTML is served under /doxygen.',
})
</script>

<template>
  <div class="site-container py-16 lg:py-24">
    <section class="surface-card mx-auto max-w-3xl px-6 py-12 text-center sm:px-12">
      <p class="eyebrow">Reference</p>
      <h1 class="mt-4 text-4xl font-bold tracking-tight text-ink sm:text-5xl">
        Doxygen API reference
      </h1>
      <p class="mx-auto mt-5 max-w-xl text-base leading-7 text-slate-600">
        The generated reference for <code class="bg-wash px-1.5 py-0.5 font-mono text-sm text-ink">libzlang</code>
        and <code class="bg-wash px-1.5 py-0.5 font-mono text-sm text-ink">zc</code>
        lives under this path, alongside the rest of the Zlang site.
      </p>
      <div class="mt-8 flex flex-wrap justify-center gap-3">
        <a href="/doxygen/index.html" class="btn-primary">
          Open the reference
        </a>
        <NuxtLink to="/docs" class="btn-secondary">
          Read the guides
        </NuxtLink>
      </div>
      <p class="mt-8 text-xs leading-6 text-slate-400">
        Deployment note: place the Doxygen HTML output in <code class="font-mono">public/doxygen/</code>.
        Nuxt serves it at <code class="font-mono">/doxygen/</code> without leaving the site.
      </p>
    </section>
  </div>
</template>
