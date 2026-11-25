import type { Config } from 'tailwindcss'

/**
 * Visual system shared with the author's other sites: warm paper, black ink,
 * square corners and hard shadows. Colours are wired to CSS variables so the
 * dark mode toggle only has to swap tokens.
 */
export default {
  darkMode: 'class',
  content: [
    './app/**/*.{vue,js,ts}',
    './content/**/*.{md,mdc}',
  ],
  theme: {
    extend: {
      colors: {
        canvas: 'rgb(var(--paper-rgb) / <alpha-value>)',
        paper: 'rgb(var(--paper-rgb) / <alpha-value>)',
        sheet: 'rgb(var(--paper-2-rgb) / <alpha-value>)',
        wash: 'rgb(var(--wash-rgb) / <alpha-value>)',
        ink: 'rgb(var(--ink-rgb) / <alpha-value>)',
        edge: 'rgb(var(--edge-rgb) / <alpha-value>)',
        brand: {
          sky: 'rgb(var(--ink-rgb) / <alpha-value>)',
          indigo: 'rgb(var(--ink-rgb) / <alpha-value>)',
        },
        slate: {
          50: 'rgb(var(--paper-rgb) / <alpha-value>)',
          100: 'rgb(var(--wash-rgb) / <alpha-value>)',
          200: 'rgb(var(--line-rgb) / <alpha-value>)',
          300: 'rgb(var(--line-2-rgb) / <alpha-value>)',
          400: 'rgb(var(--ink-4-rgb) / <alpha-value>)',
          500: 'rgb(var(--ink-3-rgb) / <alpha-value>)',
          600: 'rgb(var(--ink-2-rgb) / <alpha-value>)',
          700: 'rgb(var(--ink-2-rgb) / <alpha-value>)',
          800: 'rgb(var(--ink-rgb) / <alpha-value>)',
          900: 'rgb(var(--ink-rgb) / <alpha-value>)',
        },
      },
      fontFamily: {
        sans: [
          'Quicksand Variable',
          'ui-sans-serif',
          'system-ui',
          '-apple-system',
          'BlinkMacSystemFont',
          'Segoe UI',
          'Roboto',
          'sans-serif',
        ],
        mono: [
          'Fira Code Variable',
          'JetBrains Mono Variable',
          'ui-monospace',
          'SFMono-Regular',
          'Menlo',
          'Consolas',
          'monospace',
        ],
      },
      borderRadius: {
        none: '0',
        sm: '0',
        DEFAULT: '0',
        md: '0',
        lg: '0',
        xl: '0',
        '2xl': '0',
        '3xl': '0',
        full: '0',
      },
      boxShadow: {
        soft: '8px 8px rgba(0, 0, 0, 0.06)',
        card: '8px 8px rgba(0, 0, 0, 0.06)',
      },
      maxWidth: {
        '7xl': '1440px',
      },
    },
  },
  plugins: [],
} satisfies Config
