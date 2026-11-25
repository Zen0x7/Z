<script setup lang="ts">
import {
  firstPagePath,
  hasIndexPage,
  useDocsNavigation,
  type NavItem,
} from '~/composables/useDocsNavigation'

const route = useRoute()
const { data: navigation } = useDocsNavigation()

function isPrefix(path: string | undefined) {
  return Boolean(path && (route.path === path || route.path.startsWith(`${path}/`)))
}

const sectionRoot = computed<NavItem | null>(() => {
  const nodes = navigation.value || []
  const candidates: NavItem[] = []

  function walk(items: NavItem[]) {
    for (const item of items) {
      if (isPrefix(item.path) && item.children?.length && hasIndexPage(item)) {
        candidates.push(item)
      }
      if (item.children?.length) walk(item.children)
    }
  }

  walk(nodes)
  candidates.sort((left, right) => (right.path?.length ?? 0) - (left.path?.length ?? 0))
  return candidates[0] ?? nodes.find(item => item.path === '/docs') ?? null
})

const items = computed(() => sectionRoot.value?.children ?? [])
</script>

<template>
  <aside class="lg:sticky lg:top-24 lg:h-fit">
    <nav aria-label="Documentation sections">
      <p class="eyebrow mb-4">Sections</p>

      <ul class="docs-tree docs-current-list">
        <DocsNavItem
          v-for="item in items"
          :key="item.path || item.title"
          :item="item"
          :depth="0"
        />
      </ul>
    </nav>
  </aside>
</template>
