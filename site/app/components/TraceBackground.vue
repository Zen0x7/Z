<script setup lang="ts">
import { zlangTokenTraces as traces } from '~/config/zlangTokens'

const props = defineProps<{ compact?: boolean }>()
</script>

<template>
  <div
    class="trace-field"
    :class="compact && 'trace-field--compact'"
    aria-hidden="true"
  >
    <span class="trace-field__line" />
    <span
      v-for="(trace, index) in traces"
      :key="`${trace.from}-${index}`"
      class="trace-token"
      :style="{
        left: trace.left,
        top: trace.top,
        '--trace-delay': `-${trace.delay}ms`,
        '--twinkle-delay': `-${trace.delay % 1900}ms`,
      }"
    >
      <span class="trace-token__from">{{ trace.from }}</span>
      <span class="trace-token__to">{{ trace.to }}</span>
    </span>
  </div>
</template>
