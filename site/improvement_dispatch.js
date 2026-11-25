const common = `
You are improving the Zlang site and documentation. Use relative paths from the repository root only. Never write absolute filesystem paths containing a user name.

Read first:
- AGENTS.md
- SPEC.md
- site/DOCS_STYLE_GUIDE.md
- site/STDLIB_DOCS_SPEC.md
- site/README.md

Rules:
- Do not edit compiler/**, include/**, tools/**, cmake/**, tests/**, ROADMAP.md, HANDOFF.md, or any WIP source from another agent.
- Do not document inheritance, traits, polymorphism, or any WIP feature as available.
- Do not invent APIs, functions, signatures, C symbols or macros.
- Keep the established design and voice: concrete, active, no marketing copy.
- Do not run git add, git commit or git push.
- Report every file changed and every issue found.
`

const tasks = [
  {
    key: 'language-overview',
    task: `${common}
Own these paths only:
- site/content/docs/language/**
- site/content/docs/overview/**

Audit and fix correctness and coherence:
- current syntax must match SPEC.md and compiler/src/lexer.c; use struct, not record; no print keyword;
- no WIP features presented as available;
- frontmatter and navigation titles/descriptions present;
- internal links resolve;
- pages at this level list real siblings;
- prose follows the style guide.
Make minimal targeted edits. Do not touch other directories.
`,
  },
  {
    key: 'quickstart-compiler',
    task: `${common}
Own these paths only:
- site/content/docs/quick-start/**
- site/content/docs/compiler/**

Audit and fix correctness and coherence:
- build commands, sanitizers, coverage, fuzzers and header checks match AGENTS.md;
- Quick start is the short path and Build is the options/reference page;
- emitted C excerpt matches examples/algorithms/fibonacci/main.c byte-for-byte where quoted;
- no stale print examples; use explicit main and @std/io write;
- frontmatter, navigation and internal links are consistent;
- no WIP features.
Make minimal targeted edits. Do not touch other directories.
`,
  },
  {
    key: 'stdlib-io-random',
    task: `${common}
Own these paths only:
- site/content/docs/standard-library/namespaces/io/**
- site/content/docs/standard-library/namespaces/random/**

Audit and fix:
- exact registry signatures, C symbols and activation macros from compiler/src/stdlib.c;
- required section order on every function page: Summary, Import, Signature, Parameters, Result, Bounds and ownership, Errors and traps, State and concurrency, Generated C, Example, See also;
- explicit mismatches where registry and generated C disagree;
- random functions are opaque/entropy-backed where the source says so;
- no invented deterministic guarantees;
- frontmatter, navigation and internal links.
Do not edit shared namespace index. Make minimal targeted edits.
`,
  },
  {
    key: 'stdlib-math-strings-options',
    task: `${common}
Own these paths only:
- site/content/docs/standard-library/namespaces/math/**
- site/content/docs/standard-library/namespaces/strings/**
- site/content/docs/standard-library/namespaces/program_options/**

Audit and fix:
- exact registry signatures, C symbols and activation macros from compiler/src/stdlib.c;
- required section order on every function page;
- documented mismatches remain explicit and accurate;
- no invented proposed APIs, no WIP features;
- frontmatter, navigation and internal links;
- style guide compliance.
Do not edit shared namespace index. Make minimal targeted edits.
`,
  },
  {
    key: 'ui-polish',
    task: `${common}
Own these paths only:
- site/app/components/SiteHeader.vue
- site/app/components/SiteFooter.vue
- site/app/components/CodeBlock.vue
- site/app/components/DocToc.vue
- site/app/components/DocTocItem.vue
- site/app/components/DocsNav.vue
- site/app/components/DocsNavItem.vue
- site/app/assets/css/main.css

Audit and fix:
- ToC active bar has real spacing; no inline padding-left 0;
- topbar has horizontal padding on desktop and mobile;
- header API link points to https://z-doxygen.iantorres.cl;
- footer portrait and name link to https://www.iantorres.cl;
- dark mode remains default and saved preference still wins;
- square blocks, no rounded corners where the design uses square;
- mobile deck buttons and chrome do not steal content space;
- no unnecessary borders; no dead conflicting rules.
Make minimal targeted edits. Do not edit docs or config.
`,
  },
  {
    key: 'navigation-global',
    task: `${common}
Own these paths only:
- site/app/config/navigation.ts
- site/content/docs/index.md
- site/content/docs/standard-library/index.md
- site/content/docs/standard-library/namespaces/index.md
- site/nuxt.config.ts

Audit and fix:
- top nav links use /docs/overview, /docs/quick-start, /docs/language, /docs/compiler, /docs/standard-library;
- Doxygen link is external https://z-doxygen.iantorres.cl;
- prerender routes are generated from content/docs/** without crawler;
- no /doxygen local route;
- namespace index lists every implemented domain: io, random, math, strings, program_options;
- standard library hub does not claim proposed domains are implemented;
- no WIP features.
Make minimal targeted edits. Do not edit other docs.
`,
  },
]

const results = await runs.all(tasks.map(task => ({
  key: task.key,
  agent: 'worker',
  task: task.task,
})))

return results.map((result, index) => ({
  area: tasks[index].key,
  ok: result.ok,
  runId: result.runId,
  output: result.output,
}))
