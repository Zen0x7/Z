<script setup lang="ts">
import { docsNavigation, projectLinks } from '../config/navigation'

const route = useRoute()
const menuOpen = ref(false)

watch(
  () => route.fullPath,
  () => {
    menuOpen.value = false
  },
)

function isDocsActive(to: string) {
  return to === '/docs' ? route.path === '/docs' : route.path.startsWith(to)
}
</script>

<template>
  <header class="bg-paper">
    <div class="site-container topbar">
      <NuxtLink to="/" class="brand" aria-label="Zlang home">
        <BrandLogo width="144" height="36" class="brand__logo" />
        <span class="brand__tag">source → C99</span>
      </NuxtLink>

      <div class="hidden items-center gap-2 lg:flex">
        <ThemeToggle />
        <NuxtLink to="/doxygen" class="btn-secondary btn--tiny">
          API
        </NuxtLink>
        <a
          href="https://github.com/Zen0x7/Z"
          target="_blank"
          rel="noopener noreferrer"
          class="btn-primary btn--tiny"
        >
          GitHub
          <GitHubIcon />
        </a>
      </div>

      <div class="flex items-center gap-2 lg:hidden">
        <ThemeToggle />
        <button
          type="button"
          class="btn btn--tiny"
          :aria-expanded="menuOpen"
          aria-controls="mobile-navigation"
          aria-label="Toggle navigation"
          @click="menuOpen = !menuOpen"
        >
          {{ menuOpen ? 'Close' : 'Menu' }}
        </button>
      </div>
    </div>

    <div class="site-container hidden lg:block">
      <nav class="nav" aria-label="Documentation">
        <NuxtLink
          v-for="link in docsNavigation"
          :key="link.to"
          :to="link.to"
          class="nav__item"
          :class="isDocsActive(link.to) && 'nav__item--on'"
        >
          {{ link.label }}
        </NuxtLink>
      </nav>
    </div>

    <div v-if="menuOpen" id="mobile-navigation" class="site-container lg:hidden">
      <nav class="nav" aria-label="Mobile documentation">
        <NuxtLink
          v-for="link in docsNavigation"
          :key="link.to"
          :to="link.to"
          class="nav__item"
          :class="isDocsActive(link.to) && 'nav__item--on'"
        >
          {{ link.label }}
        </NuxtLink>
        <template v-for="link in projectLinks" :key="link.label">
          <NuxtLink v-if="'to' in link" :to="link.to" class="nav__item">
            {{ link.label }}
          </NuxtLink>
          <a v-else :href="link.href" target="_blank" rel="noopener noreferrer" class="nav__item">
            {{ link.label }}
          </a>
        </template>
      </nav>
    </div>
  </header>
</template>
