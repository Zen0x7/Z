<script setup lang="ts">
import { zlangTokenTraces } from '~/config/zlangTokens'

const words = zlangTokenTraces.slice(0, 10)
</script>

<template>
  <div class="playground-backdrop" aria-hidden="true">
    <span class="playground-backdrop__grid" />
    <span class="playground-backdrop__scan" />
    <span
      v-for="(word, index) in words"
      :key="word.from"
      class="playground-backdrop__word"
      :class="`playground-backdrop__word--${index % 3}`"
      :style="{
        left: word.left,
        top: word.top,
        '--word-delay': `-${word.delay}ms`,
      }"
    >{{ word.from }}</span>
  </div>
</template>

<style scoped>
.playground-backdrop {
  position: fixed;
  inset: 0;
  z-index: 0;
  overflow: hidden;
  pointer-events: none;
  opacity: 0.22;
  -webkit-mask-image: linear-gradient(to bottom, transparent 0%, #000 12%, #000 88%, transparent 100%);
  mask-image: linear-gradient(to bottom, transparent 0%, #000 12%, #000 88%, transparent 100%);
}

.playground-backdrop__grid {
  position: absolute;
  inset: -20%;
  background-image: radial-gradient(circle, var(--ink-4) 0.9px, transparent 1px);
  background-size: 24px 24px;
  animation: playground-grid-drift 30s linear infinite;
}

.playground-backdrop__scan {
  position: absolute;
  left: -40%;
  right: -40%;
  top: 46%;
  height: 1px;
  background: linear-gradient(90deg, transparent, var(--ink-3), transparent);
  animation: playground-scan 16s ease-in-out infinite;
}

.playground-backdrop__word {
  position: absolute;
  color: var(--ink-3);
  font-family: 'Fira Code Variable', 'JetBrains Mono Variable', ui-monospace, monospace;
  font-size: 0.72rem;
  letter-spacing: 0.04em;
  white-space: nowrap;
  animation: playground-word 9s ease-in-out infinite;
  animation-delay: var(--word-delay);
}

.playground-backdrop__word--0 { color: #f78c6c; }
.playground-backdrop__word--1 { color: #ffcb6b; }
.playground-backdrop__word--2 { color: #8fd3c7; }

@keyframes playground-grid-drift {
  to { transform: translate3d(24px, 24px, 0); }
}

@keyframes playground-scan {
  0%, 100% { transform: translateX(-12%); opacity: 0; }
  50% { transform: translateX(12%); opacity: 0.7; }
}

@keyframes playground-word {
  0%, 100% { opacity: 0.18; transform: translateX(-6px); }
  50% { opacity: 0.55; transform: translateX(6px); }
}

@media (prefers-reduced-motion: reduce) {
  .playground-backdrop__grid,
  .playground-backdrop__scan,
  .playground-backdrop__word {
    animation: none;
  }
}
</style>
