<script setup lang="ts">
import { useAttrs } from 'vue'

defineOptions({ inheritAttrs: false })

const props = defineProps<{
  code?: string
  language?: string | null
  filename?: string | null
}>()

const attrs = useAttrs()
const copied = ref(false)
let copyTimeout: ReturnType<typeof setTimeout> | undefined

async function copyCode() {
  await navigator.clipboard.writeText(props.code ?? '')
  copied.value = true
  clearTimeout(copyTimeout)
  copyTimeout = setTimeout(() => {
    copied.value = false
  }, 1600)
}

onUnmounted(() => {
  clearTimeout(copyTimeout)
})
</script>

<template>
  <div class="code-surface docs-code">
    <div class="code-toolbar">
      <span class="font-mono text-xs text-slate-400">
        {{ filename ?? language ?? 'code' }}
      </span>
      <button
        type="button"
        class="code-copy"
        :aria-label="copied ? 'Code copied' : 'Copy code'"
        @click="copyCode"
      >
        {{ copied ? 'Copied' : 'Copy' }}
      </button>
    </div>
    <pre :class="attrs.class"><slot /></pre>
  </div>
</template>
