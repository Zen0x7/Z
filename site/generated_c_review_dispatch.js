const common = `
Review and fix documentation in the Zlang site. Use relative paths from the repository root only. Never write absolute filesystem paths containing a user name.

Focus:
- Every "Generated C" section or emitted-C snippet must be useful to a reader.
- No compiler-generated symbol such as zlang_gen_literal_0, zlang_gen_read_3, app_local_*, zlang_gen_foreach_* may appear without explaining what source construct it comes from or showing the declaration that gives it meaning.
- Do not mix @std/io write wrappers into another function's contract. If a snippet includes write_*, explain that it belongs to the example, not to the documented function.
- Exact C snippets must match zc output or the committed golden C. If shortened, say it is an excerpt.
- Registry signatures, C symbols and activation macros must match compiler/src/stdlib.c.
- Do not invent APIs. Do not document WIP features.
- Keep prose concrete and active. No marketing language.

Read:
- AGENTS.md
- SPEC.md
- site/DOCS_STYLE_GUIDE.md
- site/STDLIB_DOCS_SPEC.md
- compiler/include/zlang/compiler/stdlib.h
- compiler/src/stdlib.c
- examples/**

Do not run git add, git commit or git push.
Report files changed, issues fixed, and anything left for another owner.
`

const tasks = [
  {
    key: 'core-docs',
    task: `${common}
Own only:
- site/content/docs/overview/**
- site/content/docs/quick-start/**
- site/content/docs/compiler/**

Review emitted-C examples in compiler pages, quick-start examples and overview pipeline wording. Fix unexplained generated names, stale snippets, and wrapper confusion.
`,
  },
  {
    key: 'language-docs',
    task: `${common}
Own only:
- site/content/docs/language/**

Review all code snippets and any generated-C reference. Ensure examples use current syntax: explicit main, @std/io write, struct not record. Fix unexplained generated names and stale diagnostic messages.
`,
  },
  {
    key: 'stdlib-io-random',
    task: `${common}
Own only:
- site/content/docs/standard-library/namespaces/io/**
- site/content/docs/standard-library/namespaces/random/**

Review every Generated C section. Explain every generated name. Distinguish function call from output wrapper. Verify registry fields and exact snippets.
`,
  },
  {
    key: 'stdlib-math-strings-options',
    task: `${common}
Own only:
- site/content/docs/standard-library/namespaces/math/**
- site/content/docs/standard-library/namespaces/strings/**
- site/content/docs/standard-library/namespaces/program_options/**

Review every Generated C section. Explain interned literals and generated temporaries. Keep write_* wrappers out of function contracts. Verify registry symbols and macros.
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
