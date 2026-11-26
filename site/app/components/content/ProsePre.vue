<script setup lang="ts">
import { useAttrs } from 'vue'
import { runZlang } from '~/lib/zlangRunner'

defineOptions({ inheritAttrs: false })

const props = defineProps<{
  code?: string
  language?: string | null
  filename?: string | null
}>()

const attrs = useAttrs()
const copied = ref(false)
const running = ref(false)
const panelOpen = ref(false)
const activeTab = ref<'output' | 'log'>('output')
const output = ref('')
const log = ref<string[]>([])
let copyTimeout: ReturnType<typeof setTimeout> | undefined

const isZlang = computed(() => props.language === 'zlang')
const hasKeywordBackdrop = computed(() => props.language === 'zlang' || props.language === 'c')

async function copyCode() {
  await navigator.clipboard.writeText(props.code ?? '')
  copied.value = true
  clearTimeout(copyTimeout)
  copyTimeout = setTimeout(() => {
    copied.value = false
  }, 1600)
}

async function execute() {
  panelOpen.value = true
  running.value = true
  activeTab.value = 'output'
  output.value = ''
  log.value = []

  try {
    if (!props.code?.includes('function main')) {
      log.value = ['This snippet needs function main to run.']
      return
    }
    const result = await runZlang(props.code)
    output.value = result.output
    log.value = result.log
  } catch (error) {
    log.value = [error instanceof Error ? error.message : String(error)]
  } finally {
    running.value = false
  }
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
      <div class="code-toolbar__actions">
        <button
          v-if="isZlang"
          type="button"
          class="code-copy"
          :aria-label="running ? 'Running code' : 'Run code'"
          :disabled="running"
          @click="execute"
        >
          {{ running ? 'Running…' : 'Run' }}
        </button>
        <button
          type="button"
          class="code-copy"
          :aria-label="copied ? 'Code copied' : 'Copy code'"
          @click="copyCode"
        >
          {{ copied ? 'Copied' : 'Copy' }}
        </button>
      </div>
    </div>
    <div class="code-source">
      <TraceBackground v-if="hasKeywordBackdrop" compact />
      <pre :class="attrs.class"><slot /></pre>
    </div>

    <Transition name="code-panel">
      <div v-if="panelOpen" class="code-run-result">
        <FibonacciBackdrop compact />

        <div class="code-run-result__content">
          <Transition name="code-fade" mode="out-in">
            <div v-if="running" key="status" class="code-run-status" role="status" aria-live="polite">
              <span class="code-run-status__label">Compiling...</span>
            </div>

            <div v-else key="result">
              <div class="code-run-tabs" role="tablist" aria-label="Run result">
              <button
                type="button"
                role="tab"
                class="code-run-tab"
                :class="activeTab === 'output' && 'code-run-tab--on'"
                :aria-selected="activeTab === 'output'"
                @click="activeTab = 'output'"
              >
                Output
              </button>
              <button
                type="button"
                role="tab"
                class="code-run-tab"
                :class="activeTab === 'log' && 'code-run-tab--on'"
                :aria-selected="activeTab === 'log'"
                @click="activeTab = 'log'"
              >
                Build log
              </button>
            </div>

            <Transition name="code-fade" mode="out-in">
              <pre
                :key="activeTab"
                class="code-run-result__text"
                :class="activeTab === 'log' && 'code-run-result__text--log'"
                role="tabpanel"
              >{{ activeTab === 'output' ? output || 'Program finished without output.' : log.join('\n') }}</pre>
              </Transition>
            </div>
          </Transition>
        </div>
      </div>
    </Transition>
  </div>
</template>

<style scoped>
.code-source {
  position: relative;
}

.code-source pre {
  position: relative;
  z-index: 1;
}

.code-toolbar__actions {
  display: flex;
  align-items: center;
  gap: 8px;
}

.code-copy:disabled {
  opacity: 0.55;
  cursor: wait;
}

.code-run-result {
  position: relative;
  min-height: 120px;
  overflow: hidden;
  border-top: 1px solid var(--code-border);
  background: var(--code-bg);
}

.code-run-result__content {
  position: relative;
  z-index: 1;
  display: flex;
  flex-direction: column;
  min-height: 98px;
  padding: 10px 14px 12px;
}

.code-run-status {
  display: flex;
  flex: 1;
  align-items: center;
  justify-content: center;
  min-height: 98px;
  padding: 0;
}

.code-run-status__label {
  font-family: 'Fira Code Variable', 'JetBrains Mono Variable', ui-monospace, monospace;
  font-size: 0.8125rem;
  text-align: center;
  color: var(--code-fg);
  animation: compile-pulse 1.2s ease-in-out infinite;
}

@keyframes compile-pulse {
  0%, 100% { opacity: 0.55; }
  50% { opacity: 1; }
}

.code-run-tabs {
  display: flex;
  gap: 6px;
  margin-bottom: 8px;
}

.code-run-tab {
  padding: 5px 10px;
  border: 1px solid var(--code-border);
  background: rgba(0, 0, 0, 0.35);
  color: var(--code-fg);
  font-family: inherit;
  font-size: 0.6875rem;
  font-weight: 700;
  letter-spacing: 0.08em;
  text-transform: uppercase;
  cursor: pointer;
}

.code-run-tab--on {
  background: var(--code-fg);
  border-color: var(--code-fg);
  color: var(--code-bg);
}

.code-run-result__text {
  margin: 0;
  min-height: 24px;
  max-height: 120px;
  overflow: auto;
  white-space: pre-wrap;
  font-family: 'Fira Code Variable', 'JetBrains Mono Variable', ui-monospace, monospace;
  font-size: 0.8125rem;
  line-height: 1.6;
  color: var(--code-fg);
}

.code-run-result__text--log {
  font-size: 0.75rem;
}

.code-panel-enter-active,
.code-panel-leave-active,
.code-fade-enter-active,
.code-fade-leave-active {
  transition: opacity 0.18s ease;
}

.code-panel-enter-from,
.code-panel-leave-to,
.code-fade-enter-from,
.code-fade-leave-to {
  opacity: 0;
}

@media (prefers-reduced-motion: reduce) {
  .code-panel-enter-active,
  .code-panel-leave-active,
  .code-fade-enter-active,
  .code-fade-leave-active {
    transition: none;
  }

  .code-run-status__label {
    animation: none;
  }
}
</style>
