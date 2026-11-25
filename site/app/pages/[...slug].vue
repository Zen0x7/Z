<script setup lang="ts">
const route = useRoute()
const { data: navigation } = useDocsNavigation()

const { data: page } = await useAsyncData(`content-${route.path}`, () => {
  return queryCollection('content').path(route.path).first()
})

if (!page.value) {
  throw createError({ statusCode: 404, statusMessage: 'Page not found', fatal: true })
}

const isDocsPage = computed(() => route.path.startsWith('/docs'))
const pageTitle = computed(() => page.value?.title || 'Zlang')
const pageDescription = computed(() => page.value?.description || 'Documentation for the Zlang language.')
const tocLinks = computed(() => (page.value as { body?: { toc?: { links?: unknown[] } } } | null)?.body?.toc?.links ?? [])

const breadcrumbs = computed(() => {
  const match = findNavMatch(navigation.value || [], route.path)
  const items = (match?.trail ?? []).filter(item => item.path !== '/docs')
  if (match?.node) items.push(match.node)

  return items
    .map(item => ({ title: item.title || '', path: firstPagePath(item) }))
    .filter(item => item.path)
})

useSeoMeta({
  title: pageTitle,
  description: pageDescription,
})
</script>

<template>
  <div
    v-if="page"
    class="docs-page site-container py-12 lg:py-16"
    :class="isDocsPage ? 'grid gap-10 lg:grid-cols-[15rem_minmax(0,1fr)] lg:gap-14 xl:grid-cols-[15rem_minmax(0,1fr)_12rem]' : ''"
  >
    <DocsBackdrop v-if="isDocsPage" />
    <DocsNav v-if="isDocsPage" />
    <div class="docs-main min-w-0">
      <nav v-if="isDocsPage && breadcrumbs.length" class="docs-breadcrumbs" aria-label="Breadcrumb">
        <NuxtLink
          v-for="(crumb, index) in breadcrumbs"
          :key="`${crumb.path}-${index}`"
          :to="crumb.path!"
        >
          {{ crumb.title }}
        </NuxtLink>
      </nav>
      <article class="prose-zlang">
        <ContentRenderer :value="page" />
      </article>
    </div>
    <DocToc v-if="isDocsPage && tocLinks.length" :links="tocLinks as any" />
  </div>
</template>
