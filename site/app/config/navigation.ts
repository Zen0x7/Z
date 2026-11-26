export const docsNavigation = [
  { label: 'Overview', to: '/docs/overview', description: 'What Zlang is' },
  { label: 'Quick start', to: '/docs/quick-start', description: 'Build and run a program' },
  { label: 'Language', to: '/docs/language', description: 'Syntax and semantics' },
  { label: 'Specification', to: '/docs/specification', description: 'Normative contract' },
  { label: 'Playground', to: '/playground', description: 'Compile and run in the browser' },
  { label: 'Compiler', to: '/docs/compiler', description: 'Pipeline and emitted C' },
  { label: 'Standard library', to: '/docs/standard-library', description: 'Domains and imports' },
] as const

export const projectLinks = [
  { label: 'Doxygen API', href: 'https://z-doxygen.iantorres.cl', description: 'libzlang and zc reference' },
  { label: 'GitHub', href: 'https://github.com/Zen0x7/Z', description: 'Source repository' },
] as const
