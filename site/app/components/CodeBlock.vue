<script setup lang="ts">
import { createHighlighterCore } from 'shiki/core'
import { createOnigurumaEngine } from 'shiki/engine/oniguruma'
import cLanguage from 'shiki/langs/c.mjs'
import bashLanguage from 'shiki/langs/bash.mjs'
import { zlangLanguage } from '../../shiki/zlang'
import { zlangTheme } from '../../shiki/zlang-theme'

type SupportedLanguage = 'zlang' | 'c' | 'bash'

const props = withDefaults(
  defineProps<{
    code: string
    lang?: SupportedLanguage
    filename?: string
    sourceUrl?: string
    sideNote?: string
  }>(),
  {
    lang: 'zlang',
    filename: undefined,
    sourceUrl: undefined,
    sideNote: undefined,
  },
)

const html = ref('')
const copied = ref(false)
let copyTimeout: ReturnType<typeof setTimeout> | undefined
let highlighterPromise: ReturnType<typeof createHighlighterCore> | undefined

function getHighlighter() {
  highlighterPromise ??= createHighlighterCore({
    themes: [zlangTheme],
    langs: [zlangLanguage, cLanguage, bashLanguage],
    engine: createOnigurumaEngine(import('shiki/wasm')),
  })

  return highlighterPromise
}

async function render() {
  const highlighter = await getHighlighter()
  html.value = highlighter.codeToHtml(props.code, {
    lang: props.lang,
    theme: 'zlang-ink',
  })
}

async function copyCode() {
  await navigator.clipboard.writeText(props.code)
  copied.value = true
  clearTimeout(copyTimeout)
  copyTimeout = setTimeout(() => {
    copied.value = false
  }, 1600)
}

onMounted(() => {
  void render()
})

watch(
  () => [props.code, props.lang],
  () => {
    void render()
  },
)

onUnmounted(() => {
  clearTimeout(copyTimeout)
})
</script>

<template>
  <div class="code-surface" :class="sideNote && 'code-surface--with-note'">
    <div class="code-toolbar">
      <div class="flex items-center gap-1.5" aria-hidden="true">
        <span class="h-2.5 w-2.5 bg-white/25" />
        <span class="h-2.5 w-2.5 bg-white/15" />
        <span class="h-2.5 w-2.5 bg-white/10" />
      </div>

      <span class="font-mono text-xs text-slate-400">
        {{ filename ?? lang }}
      </span>

      <a
        v-if="sourceUrl"
        :href="sourceUrl"
        target="_blank"
        rel="noopener noreferrer"
        class="code-source-link"
      >
        View full example ↗
      </a>

      <button
        type="button"
        class="code-copy"
        :aria-label="copied ? 'Code copied' : 'Copy code'"
        @click="copyCode"
      >
        {{ copied ? 'Copied' : 'Copy' }}
      </button>
    </div>

    <div class="px-2 pb-2 pt-1">
      <div v-if="html" v-html="html" />
      <pre v-else class="p-3 text-sm leading-7"><code>{{ code }}</code></pre>
    </div>

    <span v-if="sideNote" class="code-side-note">{{ sideNote }}</span>
  </div>
</template>
