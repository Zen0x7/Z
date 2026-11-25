<script setup lang="ts">
interface TocLink {
  id: string
  text: string
  depth: number
  children?: TocLink[]
}

defineProps<{
  link: TocLink
  activeId: string
}>()
</script>

<template>
  <li>
    <a
      :href="`#${link.id}`"
      class="docs-toc__link"
      :class="activeId === link.id && 'docs-toc__link--on'"
    >
      {{ link.text }}
    </a>
    <ul v-if="link.children?.length" class="docs-toc__children">
      <DocTocItem
        v-for="child in link.children"
        :key="child.id"
        :link="child"
        :active-id="activeId"
      />
    </ul>
  </li>
</template>
