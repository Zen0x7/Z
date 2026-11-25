# Zlang site

Nuxt site for the Z language: landing page, guides and a mounted path for the generated Doxygen reference.

## Development

```bash
nvm use 26
yarn install
yarn dev
```

The site runs at `http://localhost:3000`.

## Structure

- `app/pages/index.vue` — landing page with real algorithm examples.
- `app/pages/[...slug].vue` — Markdown guides rendered with Nuxt Content.
- `app/pages/doxygen/index.vue` — entry point for the API reference.
- `content/docs/` — documentation source.
- `shiki/zlang.ts` — TextMate grammar for `.z` / `.zlang` sources.
- `shiki/zlang-theme.ts` — coloured Shiki theme tuned to the site palette.
- `app/components/content/ProsePre.vue` — copy button for Markdown code blocks.

## Visual system

The site follows the paper/ink visual system used in the author's other projects: warm paper `#fcfcf8`, black ink `#111111`, square corners, softened 2px borders and hard shadows. Quicksand is the UI font; Fira Code and JetBrains Mono handle code. Dark mode is the default; the header (and the intro deck) toggle switches through `@nuxtjs/color-mode`. A visitor's saved choice still wins over the default. All colours come from CSS variables.

`public/logo-light.svg` and `public/logo-dark.svg` keep the original Z mark and wordmark but restyle them with that palette. `BrandLogo.vue` picks the right file from the active colour mode. The original `docs/logo.svg` is untouched.

The footer is the shared author footer: avatar, `Made by Ian Torres`, rights, repository link and version from `cmake/ZlangVersion.cmake`.

Code uses Fira Code and JetBrains Mono variable fonts with ligatures. Syntax colours are Material-inspired but avoid blue and purple: amber keywords, coral types, green strings, teal functions and cream variables.

## Syntax highlighting

Nuxt Content registers the Zlang grammar and theme in `nuxt.config.ts`, so fenced blocks tagged `zlang` are highlighted in the guides. `ProsePre.vue` adds a copy button to every Markdown code block. The landing page uses the same grammar and theme through `CodeBlock.vue`.

## Doxygen path

The generated Doxygen HTML is served from `public/doxygen/`, so it is available at `/doxygen/index.html` on the same origin as the Nuxt site. `app/pages/doxygen/index.vue` provides the `/doxygen` entry page.

For a local build, run `scripts/build_site.sh` from the repository root. It builds Doxygen, copies `build/docs/html` into `site/public/doxygen`, then runs `yarn generate`.

Vercel uses the same script through `vercel.json`. The final output is `site/.output/public`. Set the Vercel Node version to 26 or keep it compatible with Nuxt 4's current engine requirement.
