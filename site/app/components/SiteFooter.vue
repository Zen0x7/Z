<script setup lang="ts">
// Mirrors cmake/ZlangVersion.cmake.
const version = '0.21.0'
const year = new Date().getFullYear()
</script>

<template>
  <footer class="foot">
    <a
      class="foot__avatar-link"
      href="https://www.iantorres.cl"
      target="_blank"
      rel="noopener noreferrer"
      aria-label="Ian Torres"
    >
      <img class="foot__avatar" src="/avatar.png" alt="Ian Torres" width="76" height="76">
    </a>
    <p class="foot__by">
      Made by
      <a
        class="foot__name"
        href="https://www.iantorres.cl"
        target="_blank"
        rel="noopener noreferrer"
      >
        <strong>Ian Torres</strong>
      </a>
    </p>
    <p class="foot__rights">© {{ year }} · All rights reserved</p>
    <a
      class="foot__link"
      href="https://github.com/Zen0x7/Z"
      target="_blank"
      rel="noopener noreferrer"
    >
      <GitHubIcon />
      <span>Source on GitHub</span>
    </a>
    <span class="foot__ver num">Version {{ version }}</span>
  </footer>
</template>
