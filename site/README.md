# Zlang site

Nuxt site for the Z language: landing page and guides. The generated source-level API reference is hosted separately at <https://z-doxygen.iantorres.cl>.

## Development

```bash
nvm use 26
yarn install
yarn dev
```

The site runs at `http://localhost:3000`.

## Structure

- `app/pages/index.vue` — landing page with an interactive tour.
- `app/pages/[...slug].vue` — Markdown guides rendered with Nuxt Content.
- `content/docs/` — documentation source.
- `shiki/zlang.ts` — TextMate grammar for `.z` / `.zlang` sources.
- `shiki/zlang-theme.ts` — coloured Shiki theme tuned to the site palette.
- `app/components/content/ProsePre.vue` — copy button for Markdown code blocks.

## Visual system

The site follows the paper/ink visual system used in the author's other projects: warm paper `#fcfcf8`, black ink `#111111`, square corners, softened 2px borders and hard shadows. Quicksand is the UI font; Fira Code and JetBrains Mono handle code. Dark mode is the default, while a visitor's saved choice still wins.

The logo has light and dark variants. `BrandLogo.vue` selects the right file from the active colour mode.

The footer uses the shared author footer: portrait, name and links to <https://www.iantorres.cl>.

## Syntax highlighting

Nuxt Content registers the Zlang grammar and theme in `nuxt.config.ts`. `ProsePre.vue` adds a copy button to every Markdown code block. The landing page uses the same grammar and theme through `CodeBlock.vue`.

## Doxygen

Doxygen is a separate deployment at <https://z-doxygen.iantorres.cl>. Use `scripts/build_site.sh` from the repository root when you need to generate the Doxygen HTML locally. That script now only generates Doxygen; it does not build Nuxt.

Nuxt deployment is managed by Vercel as a standard Nuxt project. There is no `vercel.json` in this repository.
