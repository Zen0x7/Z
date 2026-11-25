<template>
  <div class="min-h-screen">
    <a
      href="#main-content"
      class="sr-only focus:not-sr-only focus:fixed focus:left-4 focus:top-4 focus:z-[60] focus:bg-ink focus:px-4 focus:py-2 focus:text-sm focus:text-paper"
    >
      Skip to content
    </a>
    <SiteHeader />
    <main id="main-content">
      <NuxtPage />
    </main>
    <SiteFooter />
  </div>
</template>
