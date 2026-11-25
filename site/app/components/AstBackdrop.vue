<script setup lang="ts">
const edges = [
  { x1: 200, y1: 34, x2: 96, y2: 96, delay: 0 },
  { x1: 200, y1: 34, x2: 304, y2: 96, delay: 420 },
  { x1: 96, y1: 96, x2: 48, y2: 166, delay: 840 },
  { x1: 96, y1: 96, x2: 144, y2: 166, delay: 1260 },
  { x1: 304, y1: 96, x2: 256, y2: 166, delay: 1680 },
  { x1: 304, y1: 96, x2: 352, y2: 166, delay: 2100 },
  { x1: 48, y1: 166, x2: 28, y2: 236, delay: 2520 },
  { x1: 144, y1: 166, x2: 152, y2: 236, delay: 2940 },
  { x1: 256, y1: 166, x2: 248, y2: 236, delay: 3360 },
  { x1: 352, y1: 166, x2: 372, y2: 236, delay: 3780 },
]

const nodes = [
  { x: 200, y: 34, delay: 0 },
  { x: 96, y: 96, delay: 420 },
  { x: 304, y: 96, delay: 840 },
  { x: 48, y: 166, delay: 1260 },
  { x: 144, y: 166, delay: 1680 },
  { x: 256, y: 166, delay: 2100 },
  { x: 352, y: 166, delay: 2520 },
  { x: 28, y: 236, delay: 2940 },
  { x: 152, y: 236, delay: 3360 },
  { x: 248, y: 236, delay: 3780 },
  { x: 372, y: 236, delay: 4200 },
]
</script>

<template>
  <div class="ast-backdrop" aria-hidden="true">
    <svg viewBox="0 0 400 270" preserveAspectRatio="none">
      <g class="ast-backdrop__edges">
        <line
          v-for="(edge, index) in edges"
          :key="`edge-${index}`"
          :x1="edge.x1"
          :y1="edge.y1"
          :x2="edge.x2"
          :y2="edge.y2"
          pathLength="1"
          :style="{ '--ast-delay': `${edge.delay}ms` }"
        />
      </g>
      <g class="ast-backdrop__nodes">
        <rect
          v-for="(node, index) in nodes"
          :key="`node-${index}`"
          :x="node.x - 5"
          :y="node.y - 5"
          width="10"
          height="10"
          :style="{ '--ast-delay': `${node.delay}ms` }"
        />
      </g>
    </svg>
  </div>
</template>
