import type { ThemeRegistrationRaw } from 'shiki'

/**
 * Material-inspired palette without blue or purple, tuned to the paper/ink
 * code surface. Each token class has a distinct, restrained colour.
 */
export const zlangTheme: ThemeRegistrationRaw = {
  name: 'zlang-ink',
  type: 'dark',
  fg: '#e8e8e3',
  bg: '#111111',
  colors: {
    'editor.background': '#111111',
    'editor.foreground': '#e8e8e3',
  },
  tokenColors: [
    {
      settings: {
        foreground: '#e8e8e3',
        background: '#111111',
      },
    },
    {
      scope: ['comment'],
      settings: {
        foreground: '#8a8a84',
        fontStyle: 'italic',
      },
    },
    {
      scope: ['keyword', 'keyword.control', 'storage.modifier'],
      settings: {
        foreground: '#ffcb6b',
      },
    },
    {
      scope: ['storage.type', 'storage.type.function', 'entity.name.type'],
      settings: {
        foreground: '#f78c6c',
      },
    },
    {
      scope: ['string', 'constant.character.escape'],
      settings: {
        foreground: '#c3e88d',
      },
    },
    {
      scope: ['constant.numeric', 'constant.language', 'constant.language.boolean', 'constant.language.enum'],
      settings: {
        foreground: '#f07178',
      },
    },
    {
      scope: ['entity.name.function', 'support.function'],
      settings: {
        foreground: '#8fd3c7',
      },
    },
    {
      scope: ['variable', 'variable.other.declaration', 'variable.other.assignment'],
      settings: {
        foreground: '#e6d9c8',
      },
    },
    {
      scope: ['variable.language.self.zlang', 'variable.language'],
      settings: {
        foreground: '#ffcb6b',
      },
    },
    {
      scope: ['keyword.operator', 'keyword.operator.assignment'],
      settings: {
        foreground: '#ffcb6b',
      },
    },
    {
      scope: ['punctuation'],
      settings: {
        foreground: '#c8c8c2',
      },
    },
  ],
}
