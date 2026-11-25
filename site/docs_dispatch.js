const brief = `
You are documenting the Zlang standard library. Work only inside the assigned directory. Do not edit shared files, navigation config, global docs, or git. Do not invent APIs. Do not document inheritance, traits, polymorphism, or any WIP feature.

Use relative paths from the repository root. Never write absolute filesystem paths that contain a user name.

Read first:
- AGENTS.md
- SPEC.md
- compiler/include/zlang/compiler/stdlib.h
- compiler/src/stdlib.c
- include/zlang/standard/**
- site/DOCS_STYLE_GUIDE.md
- site/STDLIB_DOCS_SPEC.md
- site/content/docs/standard-library/namespaces/io/**

Write the same structure as the io pilot:
- <domain>/.navigation.yml with title, description, navigation.title and navigation.description
- <domain>/index.md as a domain overview with import, what it does, pages at this level
- <domain>/functions/.navigation.yml with title Functions and description
- <domain>/functions/index.md as the catalog and pages at this level
- one <function>.md per implemented function name, grouping overloads in one page when the registry has overloads

Every function page must include: Summary, Import, Signature, Parameters, Result, Bounds and ownership, Errors and traps, State and concurrency, Generated C, Example, See also. Use exact registry signatures, C symbols and activation macros. If registry and generated C disagree, say so explicitly. Use explicit main and current imports in examples. No marketing language. No proposed functions as implemented.

Report: files created, source files consulted, mismatches found, anything intentionally left undocumented.
`

const result = await runs.run('program_options', {
  agent: 'worker',
  task: `${brief}
Assigned directory: site/content/docs/standard-library/namespaces/program_options
Namespace: @std/program_options
Functions from registry:
- argument_count -> unsigned | zlang_std_program_options_argument_count | ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT
- argument_at -> string | zlang_std_program_options_argument_at | ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_AT
- has_option -> boolean | zlang_std_program_options_has_option | ZLANG_STD_PROGRAM_OPTIONS_HAS_OPTION
- option_value -> string | zlang_std_program_options_option_value | ZLANG_STD_PROGRAM_OPTIONS_OPTION_VALUE
- has_flag -> boolean | zlang_std_program_options_has_flag | ZLANG_STD_PROGRAM_OPTIONS_HAS_FLAG

Read exact parameter types and modes from compiler/src/stdlib.c and the generated header. Document host argument access with explicit main where relevant. There was a known activation mismatch during earlier review; document it explicitly if it is present.
`,
})

return {
  ok: result.ok,
  runId: result.runId,
  output: result.output,
}
