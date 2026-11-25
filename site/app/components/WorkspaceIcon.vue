<script setup lang="ts">
defineProps<{ name: string }>()
</script>

<template>
  <svg viewBox="0 0 80 80" fill="none" stroke="currentColor" stroke-width="1.5" aria-hidden="true">
    <g v-if="name === 'quick'">
      <rect x="12" y="16" width="56" height="48" rx="1" />
      <path d="M22 31l7 6-7 6M34 45h20" />
    </g>

    <g v-else-if="name === 'language'">
      <path d="M31 18c-8 0-8 8-8 15s0 9-8 9c8 0 8 2 8 9s0 15 8 15" />
      <path d="M49 18c8 0 8 8 8 15s0 9 8 9c-8 0-8 2-8 9s0 15-8 15" />
      <path d="M36 39h8" />
    </g>

    <g v-else-if="name === 'spec'">
      <rect x="20" y="12" width="40" height="56" rx="1" />
      <path d="M28 25h24M28 34h24M28 43h16M28 52h20" />
    </g>

    <g v-else-if="name === 'stdlib'">
      <rect x="14" y="14" width="22" height="22" rx="1" />
      <rect x="44" y="14" width="22" height="22" rx="1" />
      <rect x="14" y="44" width="22" height="22" rx="1" />
      <rect x="44" y="44" width="22" height="22" rx="1" />
    </g>

    <g v-else-if="name === 'compiler'">
      <rect x="14" y="27" width="18" height="18" rx="1" />
      <rect x="48" y="27" width="18" height="18" rx="1" />
      <path d="M32 36h16M40 20v7M40 45v7" />
      <path d="M24 20h8M48 53h8" />
    </g>

    <g v-else-if="name === 'doxygen'">
      <path d="M14 18h20c4 0 6 2 6 6v42c0-4-2-6-6-6H14V18Z" />
      <path d="M66 18H46c-4 0-6 2-6 6v42c0-4 2-6 6-6h20V18Z" />
      <path d="M22 30h10M22 38h10M48 30h10M48 38h10" />
    </g>

    <g v-else-if="name === 'source'">
      <circle cx="24" cy="20" r="5" />
      <circle cx="24" cy="60" r="5" />
      <circle cx="56" cy="40" r="5" />
      <path d="M24 25v30M29 20h12c8 0 9 20 10 20" />
    </g>
  </svg>
</template>
