<script setup lang="ts">
const tile = 360
const ready = ref(false)

onMounted(() => {
  requestAnimationFrame(() => {
    ready.value = true
  })
})

const flowers = [
  { x: 78, y: 88, petals: 2, rx: 5, ry: 14, rotation: 18 },
  { x: 258, y: 74, petals: 3, rx: 6, ry: 16, rotation: 137.5 },
  { x: 146, y: 236, petals: 5, rx: 6, ry: 17, rotation: 274 },
  { x: 310, y: 286, petals: 8, rx: 5, ry: 15, rotation: 51 },
  { x: 66, y: 318, petals: 13, rx: 4, ry: 13, rotation: 222 },
]
</script>

<template>
  <svg
    class="fib-flowers"
    :class="ready && 'fib-flowers--ready'"
    aria-hidden="true"
    preserveAspectRatio="none"
  >
    <defs>
      <pattern
        id="fibonacci-flower-tile"
        :width="tile"
        :height="tile"
        patternUnits="userSpaceOnUse"
        patternTransform="rotate(137.5)"
      >
        <g fill="none" stroke="currentColor" stroke-width="1.2">
          <g
            v-for="flower in flowers"
            :key="flower.petals"
            :transform="`translate(${flower.x} ${flower.y}) rotate(${flower.rotation})`"
          >
            <ellipse
              v-for="petal in flower.petals"
              :key="petal"
              cx="0"
              :cy="-flower.ry"
              :rx="flower.rx"
              :ry="flower.ry"
              :transform="`rotate(${(petal * 360) / flower.petals})`"
            />
            <circle r="3" />
          </g>
        </g>
      </pattern>
    </defs>

    <rect width="100%" height="100%" fill="url(#fibonacci-flower-tile)" />
  </svg>
</template>
