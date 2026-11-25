export interface NavItem {
  title?: string
  description?: string
  path?: string
  page?: boolean
  children?: NavItem[]
}

export interface NavMatch {
  node: NavItem
  trail: NavItem[]
}

export function useDocsNavigation() {
  return useAsyncData('docs-navigation', () => {
    return queryCollectionNavigation('content', ['description'])
  })
}

export function collectNavMatches(nodes: NavItem[], path: string, trail: NavItem[], found: NavMatch[]) {
  for (const node of nodes) {
    if (node.path === path) {
      found.push({ node, trail })
    }
    if (node.children?.length) {
      collectNavMatches(node.children, path, [...trail, node], found)
    }
  }
}

export function findNavMatch(nodes: NavItem[], path: string): NavMatch | null {
  const found: NavMatch[] = []
  collectNavMatches(nodes, path, [], found)
  const leaf = found.find(item => !item.node.children?.length && item.node.page !== false)
  return leaf ?? found.find(item => item.node.page !== false) ?? found[0] ?? null
}

export function firstPagePath(item: NavItem): string | undefined {
  if (item.page !== false && item.path) return item.path
  for (const child of item.children || []) {
    const path = firstPagePath(child)
    if (path) return path
  }
  return undefined
}

export function hasIndexPage(item: NavItem) {
  return Boolean(item.children?.some(child => child.path === item.path && child.page !== false))
}
