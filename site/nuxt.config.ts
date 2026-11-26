import { readFileSync, readdirSync } from 'node:fs'
import { join } from 'node:path'
import { fileURLToPath } from 'node:url'

import { zlangLanguage } from './shiki/zlang'
import { zlangTheme } from './shiki/zlang-theme'

const contentRoot = fileURLToPath(new URL('./content/docs', import.meta.url))
const versionFile = fileURLToPath(new URL('../cmake/ZlangVersion.cmake', import.meta.url))

function readZlangVersion(): string {
  const source = readFileSync(versionFile, 'utf8')
  return source.match(/set\(ZLANG_PROJECT_VERSION\s+([^\s)]+)\)/)?.[1] ?? '0.0.0'
}

function collectContentRoutes(directory: string, base = '/docs'): string[] {
  const routes: string[] = []

  for (const entry of readdirSync(directory, { withFileTypes: true })) {
    if (entry.isDirectory()) {
      routes.push(...collectContentRoutes(join(directory, entry.name), `${base}/${entry.name}`))
      continue
    }

    if (!entry.name.endsWith('.md')) continue

    const stem = entry.name.replace(/\.md$/, '')
    if (stem === 'index') routes.push(base)
    else routes.push(`${base}/${stem}`)
  }

  return routes
}

const prerenderRoutes = ['/', '/playground', ...new Set(collectContentRoutes(contentRoot))]

export default defineNuxtConfig({
  modules: [
    '@nuxt/content',
    '@nuxtjs/tailwindcss',
    '@nuxtjs/color-mode',
  ],

  compatibilityDate: '2024-04-03',
  devtools: { enabled: true },

  runtimeConfig: {
    public: {
      zlangVersion: readZlangVersion(),
    },
  },

  nitro: {
    prerender: {
      crawlLinks: false,
      routes: prerenderRoutes,
    },
  },

  css: [
    '@fontsource-variable/quicksand/index.css',
    '@fontsource-variable/fira-code/index.css',
    '@fontsource-variable/jetbrains-mono/index.css',
  ],

  colorMode: {
    preference: 'dark',
    fallback: 'dark',
    classSuffix: '',
    storageKey: 'zlang-color-mode',
  },

  tailwindcss: {
    cssPath: './app/assets/css/main.css',
    configPath: 'tailwind.config.ts',
  },

  content: {
    build: {
      markdown: {
        toc: { depth: 3, searchDepth: 2 },
        highlight: {
          theme: { default: zlangTheme },
          langs: [
            zlangLanguage,
            'bash',
            'c',
            'console',
            'css',
            'html',
            'json',
            'md',
            'mdc',
            'ts',
            'vue',
            'yaml',
          ],
        },
      },
    },
  },

  app: {
    head: {
      titleTemplate: '%s · Zlang',
      meta: [
        {
          name: 'viewport',
          content: 'width=device-width, initial-scale=1, viewport-fit=cover',
        },
        {
          name: 'description',
          content: 'Zlang is a language that can be transpiled into pure C, with a fixed memory plan and a header-only runtime.',
        },
        { name: 'theme-color', content: '#111111' },
      ],
      link: [
        { rel: 'icon', type: 'image/svg+xml', href: '/favicon.svg' },
      ],
    },
  },
})
