<script setup lang="ts">
import { firstPagePath, hasIndexPage, type NavItem } from '~/composables/useDocsNavigation'

const props = defineProps<{
  item: NavItem
  depth?: number
}>()

const route = useRoute()
const depth = computed(() => props.depth ?? 0)
const hasChildren = computed(() => Boolean(props.item.children?.length))
const hasOwnIndex = computed(() => hasChildren.value && hasIndexPage(props.item))

function containsRoute(path?: string) {
  return Boolean(path && (route.path === path || route.path.startsWith(`${path}/`)))
}
</script>

<template>
  <li>
    <NuxtLink
      v-if="hasOwnIndex && props.item.path"
      :to="props.item.path"
      class="docs-link"
      :class="route.path === props.item.path && 'docs-link--on'"
    >
      <span class="docs-link__label">{{ item.title }}</span>
      <span v-if="item.description" class="docs-link__description">{{ item.description }}</span>
    </NuxtLink>

    <details
      v-else-if="hasChildren"
      class="docs-group"
      :open="containsRoute(item.path)"
    >
      <summary class="docs-group__item">
        <span class="docs-link__label">{{ item.title }}</span>
        <span v-if="item.description" class="docs-link__description">{{ item.description }}</span>
      </summary>
      <ul class="docs-children">
        <DocsNavItem
          v-for="child in item.children"
          :key="child.path || child.title"
          :item="child"
          :depth="depth + 1"
        />
      </ul>
    </details>

    <NuxtLink
      v-else-if="item.path"
      :to="firstPagePath(item) || item.path"
      class="docs-link"
      :class="route.path === item.path && 'docs-link--on'"
    >
      <span class="docs-link__label">{{ item.title }}</span>
      <span v-if="item.description" class="docs-link__description">{{ item.description }}</span>
    </NuxtLink>
  </li>
</template>
