<script setup lang="ts">
interface SearchResult {
  id: string
  title: string
  titles?: string[]
  content?: string
  snippets?: {
    content?: string
  }
}

const { isOpen, close, toggle } = useSearchPalette()
const { status, search } = useSearchCollection('content')

const query = ref('')
const results = ref<SearchResult[]>([])
const activeIndex = ref(0)
const input = ref<HTMLInputElement | null>(null)
let debounce: ReturnType<typeof setTimeout> | undefined

async function runSearch(value: string) {
  if (!value.trim()) {
    results.value = []
    return
  }

  const rows = await search(value, {
    limit: 8,
    snippet: { columns: ['content'], around: 14 },
  })

  results.value = rows as SearchResult[]
  activeIndex.value = 0
}

watch(query, (value) => {
  clearTimeout(debounce)
  debounce = setTimeout(() => {
    void runSearch(value)
  }, 160)
})

watch(isOpen, async (open) => {
  if (open) {
    await nextTick()
    input.value?.focus()
    if (query.value) await runSearch(query.value)
    return
  }

  clearTimeout(debounce)
  query.value = ''
  results.value = []
  activeIndex.value = 0
})

function go(result?: SearchResult) {
  if (!result) return
  close()
  void navigateTo(result.id)
}

function move(delta: number) {
  if (!results.value.length) return
  activeIndex.value = (activeIndex.value + delta + results.value.length) % results.value.length
}

function onInputKeydown(event: KeyboardEvent) {
  if (event.key === 'ArrowDown') {
    event.preventDefault()
    move(1)
  } else if (event.key === 'ArrowUp') {
    event.preventDefault()
    move(-1)
  } else if (event.key === 'Enter') {
    event.preventDefault()
    go(results.value[activeIndex.value])
  } else if (event.key === 'Escape') {
    event.preventDefault()
    close()
  }
}

function onGlobalKeydown(event: KeyboardEvent) {
  if ((event.metaKey || event.ctrlKey) && event.key.toLowerCase() === 'k') {
    event.preventDefault()
    toggle()
  }
}

onMounted(() => {
  window.addEventListener('keydown', onGlobalKeydown)
})

onUnmounted(() => {
  clearTimeout(debounce)
  window.removeEventListener('keydown', onGlobalKeydown)
})
</script>

<template>
  <ClientOnly>
    <Teleport to="body">
      <Transition name="search-modal">
        <div v-if="isOpen" class="search-overlay" @click.self="close">
          <section class="search-modal" role="dialog" aria-modal="true" aria-label="Search documentation">
            <div class="search-modal__input">
              <svg viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="currentColor" stroke-width="1.8" aria-hidden="true">
                <circle cx="11" cy="11" r="7" />
                <path d="m16 16 5 5" stroke-linecap="square" />
              </svg>
              <input
                ref="input"
                v-model="query"
                type="search"
                placeholder="Search Zlang documentation..."
                autocomplete="off"
                spellcheck="false"
                @keydown="onInputKeydown"
              >
              <kbd>ESC</kbd>
            </div>

            <div class="search-modal__body">
              <p v-if="status === 'loading'" class="search-modal__hint">Building local search index...</p>

              <template v-else-if="results.length">
                <button
                  v-for="(result, index) in results"
                  :key="result.id"
                  type="button"
                  class="search-result"
                  :class="index === activeIndex && 'search-result--active'"
                  @mouseenter="activeIndex = index"
                  @click="go(result)"
                >
                  <span class="search-result__title">{{ result.title }}</span>
                  <span v-if="result.titles?.length" class="search-result__path">
                    {{ result.titles.join(' › ') }}
                  </span>
                  <span v-if="result.snippets?.content" class="search-result__snippet" v-html="result.snippets.content" />
                </button>
              </template>

              <p v-else-if="query" class="search-modal__hint">No results for “{{ query }}”.</p>
              <p v-else class="search-modal__hint">
                Try <code>write</code>, <code>foreach</code>, <code>capacity</code> or <code>main</code>.
              </p>
            </div>

            <footer class="search-modal__footer">
              <span><kbd>↑</kbd><kbd>↓</kbd> move</span>
              <span><kbd>↵</kbd> open</span>
              <span><kbd>⌘K</kbd> toggle</span>
            </footer>
          </section>
        </div>
      </Transition>
    </Teleport>
  </ClientOnly>
</template>
