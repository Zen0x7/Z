<script setup lang="ts">
import { downloadWasm, runZlang } from '~/lib/zlangRunner'

const DEFAULT_SOURCE = `import { write } from "@std/io";

function main(): int32 {
    write("Hello from Zlang in the browser");
    return 0;
}
`

const source = ref(DEFAULT_SOURCE)
const output = ref('')
const log = ref<string[]>([])
const wasmBytes = ref<Uint8Array | null>(null)
const busy = ref(false)
const panelOpen = ref(false)
const hasResult = ref(false)
const activeTab = ref<'output' | 'log'>('output')
const error = ref('')

async function run() {
  busy.value = true
  panelOpen.value = true
  error.value = ''
  output.value = ''
  log.value = []
  wasmBytes.value = null
  hasResult.value = false
  activeTab.value = 'output'

  try {
    const result = await runZlang(source.value)
    output.value = result.output
    log.value = result.log
    wasmBytes.value = result.wasm
    hasResult.value = true
  } catch (caught) {
    error.value = caught instanceof Error ? caught.message : String(caught)
  } finally {
    busy.value = false
  }
}

function download() {
  downloadWasm(wasmBytes.value)
}
</script>

<template>
  <div class="playground-panel mx-auto w-full max-w-4xl">
    <div class="code-surface docs-code">
      <div class="code-toolbar">
        <span class="font-mono text-xs text-slate-400">main.z</span>
        <div class="playground-actions">
          <button
            type="button"
            class="code-copy"
            :disabled="busy"
            :aria-label="busy ? 'Running code' : 'Run code'"
            @click="run"
          >
            {{ busy ? 'Building…' : 'Run' }}
          </button>
          <button
            type="button"
            class="code-copy"
            :disabled="!wasmBytes"
            aria-label="Download WebAssembly binary"
            @click="download"
          >
            Download .wasm
          </button>
        </div>
      </div>

      <ZlangEditor v-model="source" />

      <Transition name="play-panel">
        <div v-if="panelOpen" class="playground-result">
          <FibonacciBackdrop compact />
          <div class="playground-result__content">
            <Transition name="play-fade" mode="out-in">
              <div v-if="busy" key="status" class="playground-status" role="status" aria-live="polite">
                <span class="playground-status__label playground-status__label--pulse">Compiling...</span>
              </div>

              <div v-else key="result">
                <p v-if="error" class="mb-3 text-sm text-red-700">{{ error }}</p>
                <div class="playground-tabs" role="tablist" aria-label="Run result">
                  <button
                    type="button"
                    role="tab"
                    class="playground-tab"
                    :class="activeTab === 'output' && 'playground-tab--on'"
                    :aria-selected="activeTab === 'output'"
                    @click="activeTab = 'output'"
                  >
                    Output
                  </button>
                  <button
                    type="button"
                    role="tab"
                    class="playground-tab"
                    :class="activeTab === 'log' && 'playground-tab--on'"
                    :aria-selected="activeTab === 'log'"
                    @click="activeTab = 'log'"
                  >
                    Build log
                  </button>
                </div>
                <Transition name="play-fade" mode="out-in">
                  <pre
                    :key="activeTab"
                    class="playground-output"
                    :class="activeTab === 'log' && 'playground-output--log'"
                    role="tabpanel"
                  >{{ activeTab === 'output' ? output || 'Program finished without output.' : log.join('\n') }}</pre>
                </Transition>
              </div>
            </Transition>
          </div>
        </div>
      </Transition>
    </div>
  </div>
</template>

<style scoped>
.playground-actions {
  display: flex;
  align-items: center;
  gap: 8px;
}

.playground-panel :deep(.zlang-editor) {
  height: 380px;
  border: 0;
}

.playground-result {
  position: relative;
  min-height: 120px;
  overflow: hidden;
  border-top: 1px solid var(--code-border);
  background: var(--code-bg);
}

.playground-result__content {
  position: relative;
  z-index: 1;
  display: flex;
  flex-direction: column;
  min-height: 96px;
  padding: 12px 18px 16px;
}

.playground-output {
  width: 100%;
  min-height: 48px;
  max-height: 160px;
  overflow: auto;
  margin: 0;
  color: var(--code-fg);
  font-family: 'Fira Code Variable', 'JetBrains Mono Variable', ui-monospace, monospace;
  font-size: 0.8125rem;
  line-height: 1.6;
  white-space: pre-wrap;
}

.playground-output--log {
  font-size: 0.75rem;
}

.playground-status {
  display: flex;
  flex: 1;
  align-items: center;
  justify-content: center;
  min-height: 96px;
}

.playground-status__label {
  font-family: 'Fira Code Variable', 'JetBrains Mono Variable', ui-monospace, monospace;
  font-size: 0.8125rem;
  text-align: center;
  color: var(--code-fg);
}

.playground-status__label--pulse {
  animation: compile-pulse 1.2s ease-in-out infinite;
}

@keyframes compile-pulse {
  0%, 100% { opacity: 0.55; }
  50% { opacity: 1; }
}

.playground-tabs {
  display: flex;
  gap: 6px;
  margin-bottom: 10px;
}

.playground-tab {
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

.playground-tab--on {
  background: var(--code-fg);
  border-color: var(--code-fg);
  color: var(--code-bg);
}

.play-panel-enter-active,
.play-panel-leave-active,
.play-fade-enter-active,
.play-fade-leave-active {
  transition: opacity 0.18s ease;
}

.play-panel-enter-from,
.play-panel-leave-to,
.play-fade-enter-from,
.play-fade-leave-to {
  opacity: 0;
}

@media (prefers-reduced-motion: reduce) {
  .play-panel-enter-active,
  .play-panel-leave-active,
  .play-fade-enter-active,
  .play-fade-leave-active {
    transition: none;
  }

  .playground-status__label--pulse {
    animation: none;
  }
}
</style>
