<script setup lang="ts">
interface TocLink {
  id: string
  text: string
  depth: number
  children?: TocLink[]
}

defineProps<{
  links: TocLink[]
}>()

const route = useRoute()
const activeId = ref('')
let observer: IntersectionObserver | undefined

function observeHeadings() {
  observer?.disconnect()
  activeId.value = ''

  const headings = document.querySelectorAll<HTMLElement>('h2[id], h3[id], h4[id]')
  if (!headings.length || !('IntersectionObserver' in window)) return

  observer = new IntersectionObserver((entries) => {
    const visible = entries.find(entry => entry.isIntersecting)
    if (visible?.target.id) activeId.value = visible.target.id
  }, { rootMargin: '-80px 0px -70% 0px' })

  headings.forEach(heading => observer?.observe(heading))
}

onMounted(() => {
  nextTick(observeHeadings)
})

watch(() => route.path, () => {
  nextTick(observeHeadings)
})

onUnmounted(() => {
  observer?.disconnect()
})
</script>

<template>
  <aside class="docs-toc" aria-label="On this page">
    <p class="eyebrow mb-4">On this page</p>
    <ul class="docs-toc__list">
      <DocTocItem
        v-for="link in links"
        :key="link.id"
        :link="link"
        :active-id="activeId"
      />
    </ul>
  </aside>
</template>
