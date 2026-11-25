<script setup lang="ts">
const colorMode = useColorMode()
const isDark = computed(() => colorMode.value === 'dark')

function toggleColorMode() {
  colorMode.preference = isDark.value ? 'light' : 'dark'
}
</script>

<template>
  <button
    type="button"
    class="btn-secondary btn--tiny theme-toggle"
    :aria-label="isDark ? 'Switch to light mode' : 'Switch to dark mode'"
    :title="isDark ? 'Light mode' : 'Dark mode'"
    @click="toggleColorMode"
  >
    <svg
      v-if="isDark"
      viewBox="0 0 24 24"
      class="theme-toggle__icon"
      fill="none"
      stroke="currentColor"
      stroke-width="1.8"
      stroke-linecap="square"
      aria-hidden="true"
    >
      <circle cx="12" cy="12" r="4" />
      <path d="M12 2v2M12 20v2M4.9 4.9l1.4 1.4M17.7 17.7l1.4 1.4M2 12h2M20 12h2M4.9 19.1l1.4-1.4M17.7 6.3l1.4-1.4" />
    </svg>
    <svg
      v-else
      viewBox="0 0 24 24"
      class="theme-toggle__icon"
      fill="none"
      stroke="currentColor"
      stroke-width="1.8"
      stroke-linecap="square"
      stroke-linejoin="miter"
      aria-hidden="true"
    >
      <path d="M20 14.5A8.5 8.5 0 0 1 9.5 4a8.5 8.5 0 1 0 10.5 10.5Z" />
    </svg>
  </button>
</template>
