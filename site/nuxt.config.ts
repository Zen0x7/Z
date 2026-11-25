import { zlangLanguage } from './shiki/zlang'
import { zlangTheme } from './shiki/zlang-theme'

export default defineNuxtConfig({
  modules: [
    '@nuxt/content',
    '@nuxtjs/tailwindcss',
    '@nuxtjs/color-mode',
  ],

  compatibilityDate: '2024-04-03',
  devtools: { enabled: true },

  nitro: {
    prerender: {
      crawlLinks: true,
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
