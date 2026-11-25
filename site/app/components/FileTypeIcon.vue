<script setup lang="ts">
const props = defineProps<{
  extension: string
  tone: 'z' | 'c' | 'exe'
}>()

const toneColor = computed(() => {
  if (props.tone === 'z') return '#f78c6c'
  if (props.tone === 'c') return '#8fd3c7'
  return '#ffcb6b'
})
</script>

<template>
  <svg viewBox="0 0 56 56" class="file-type-icon" aria-hidden="true">
    <path
      d="M14 4h20l10 10v34a4 4 0 0 1-4 4H14a4 4 0 0 1-4-4V8a4 4 0 0 1 4-4Z"
      fill="var(--paper-2)"
      stroke="var(--edge)"
      stroke-width="2"
    />
    <path d="M34 4v10h10" fill="none" stroke="var(--edge)" stroke-width="2" />

    <g fill="var(--ink-3)">
      <rect x="15" y="19" width="12" height="2" />
      <rect x="15" y="24" width="8" height="2" />
      <rect x="15" y="29" width="10" height="2" />
    </g>

    <rect x="30" y="34" width="20" height="16" :fill="toneColor" stroke="var(--edge)" stroke-width="1.5" />
    <text
      x="40"
      y="45"
      text-anchor="middle"
      font-family="'Fira Code Variable', ui-monospace, monospace"
      font-size="8"
      font-weight="700"
      fill="#111111"
    >
      {{ extension }}
    </text>
  </svg>
</template>
