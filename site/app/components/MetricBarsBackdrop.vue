<script setup lang="ts">
const ready = ref(false)

const bars = [
  { x: 28, height: 54, delay: 0 },
  { x: 82, height: 96, delay: 380 },
  { x: 136, height: 72, delay: 760 },
  { x: 190, height: 128, delay: 1140 },
  { x: 244, height: 88, delay: 1520 },
  { x: 298, height: 112, delay: 1900 },
  { x: 352, height: 66, delay: 2280 },
]

onMounted(() => {
  requestAnimationFrame(() => {
    requestAnimationFrame(() => {
      ready.value = true
    })
  })
})
</script>

<template>
  <div
    class="metric-bars-backdrop"
    :class="ready && 'metric-bars-backdrop--ready'"
    aria-hidden="true"
  >
    <svg viewBox="0 0 400 220" preserveAspectRatio="none">
      <g class="metric-bars-backdrop__grid">
        <line x1="0" y1="55" x2="400" y2="55" />
        <line x1="0" y1="110" x2="400" y2="110" />
        <line x1="0" y1="165" x2="400" y2="165" />
        <line x1="0" y1="205" x2="400" y2="205" />
      </g>
      <g class="metric-bars-backdrop__bars">
        <rect
          v-for="bar in bars"
          :key="bar.x"
          :x="bar.x"
          :y="205 - bar.height"
          width="34"
          :height="bar.height"
          :style="{ '--bar-delay': `${bar.delay}ms` }"
        />
      </g>
    </svg>
  </div>
</template>
