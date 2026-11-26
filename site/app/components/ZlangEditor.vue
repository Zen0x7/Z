<script setup lang="ts">
import { EditorState } from '@codemirror/state'
import {
  EditorView,
  drawSelection,
  highlightActiveLine,
  highlightActiveLineGutter,
  keymap,
  lineNumbers,
} from '@codemirror/view'
import { defaultKeymap, history, historyKeymap, indentWithTab } from '@codemirror/commands'
import {
  autocompletion,
  closeBrackets,
  closeBracketsKeymap,
  completionKeymap,
  type Completion,
  type CompletionContext,
} from '@codemirror/autocomplete'
import {
  HighlightStyle,
  StreamLanguage,
  bracketMatching,
  indentOnInput,
  indentUnit,
  syntaxHighlighting,
} from '@codemirror/language'
import { tags } from '@lezer/highlight'

const props = defineProps<{ modelValue: string }>()
const emit = defineEmits<{ 'update:modelValue': [value: string] }>()

const host = ref<HTMLDivElement | null>(null)
let view: EditorView | null = null

const keywords = new Set([
  'arguments', 'array', 'boolean', 'break', 'clone', 'const', 'constructor', 'continue',
  'double', 'else', 'enum', 'false', 'float', 'for', 'foreach', 'from', 'function', 'generic',
  'if', 'import', 'in', 'int32', 'int64', 'length', 'reference', 'repeat', 'requires', 'return',
  'static', 'string', 'struct', 'trait', 'true', 'uint32', 'uint64', 'uses', 'while',
])

const typeNames = new Set([
  'result_type', 'status_code', 'void', 'zlang_string_t', 'zlang_arguments_t',
])

const completionOptions: Completion[] = [
  ...[...keywords].map(label => ({ label, type: 'keyword' })),
  ...[...typeNames, 'int32', 'uint32', 'int64', 'uint64', 'float', 'double', 'boolean', 'string'].map(
    label => ({ label, type: 'type' }),
  ),
  ...[
    'write', 'read', 'read_line', 'length', 'to_int32', 'to_uint32', 'to_int64', 'to_uint64',
    'to_float', 'to_double', 'generate_int32', 'generate_uint32', 'argument_count', 'argument_at',
    'has_flag', 'has_option', 'option_value', 'abs', 'clamp', 'min', 'max', 'sqrt', 'compare',
    'contains', 'starts_with',
  ].map(label => ({ label, type: 'function' })),
  ...['true', 'false', 'STATUS_SUCCESS', 'STATUS_END_OF_INPUT', 'STATUS_IO_ERROR',
    'STATUS_INVALID_ARGUMENT', 'STATUS_CAPACITY'].map(label => ({ label, type: 'constant' })),
  {
    label: 'main',
    type: 'function',
    apply: 'function main(): int32 {\n    \n    return 0;\n}',
    info: 'Zlang entry point',
  },
  {
    label: 'result_type<int32, status_code>',
    type: 'type',
    info: 'Native result layout',
  },
]

function zlangCompletions(context: CompletionContext) {
  const word = context.matchBefore(/\w*/)
  if (!word || (word.from === word.to && !context.explicit)) return null
  return { from: word.from, options: completionOptions, validFor: /^\w*$/ }
}

const zlangStreamLanguage = StreamLanguage.define({
  startState: () => ({ inBlockComment: false }),
  token(stream, state) {
    if (state.inBlockComment) {
      if (stream.skipTo('*/')) {
        stream.match('*/')
        state.inBlockComment = false
      } else {
        stream.skipToEnd()
      }
      return 'comment'
    }
    if (stream.match('//')) {
      stream.skipToEnd()
      return 'comment'
    }
    if (stream.match('/*')) {
      state.inBlockComment = true
      return 'comment'
    }
    if (stream.match(/^"(?:[^"\\]|\\.)*"?/)) return 'string'
    if (stream.match(/^\d+(?:\.\d+)?(?:[uU]|[iI]64|[uU]64|[fF])?\b/)) return 'number'
    if (stream.match(/^[A-Za-z_]\w*/)) {
      const word = stream.current()
      if (keywords.has(word)) return 'keyword'
      if (typeNames.has(word)) return 'typeName'
      if (word === 'write' || word === 'length') return 'function'
      return 'variableName'
    }
    if (stream.match(/^(?:==|!=|<=|>=|&&|\|\||\+=|-=|\*=|\/=|%=|[+\-*\/%<>=!])/)) return 'operator'
    if (stream.match(/^[{}()[\];,.]/)) return 'punctuation'
    stream.next()
    return null
  },
})

const zlangHighlight = HighlightStyle.define([
  { tag: tags.comment, color: '#8a8a84', fontStyle: 'italic' },
  { tag: tags.keyword, color: '#ffcb6b' },
  { tag: tags.typeName, color: '#f78c6c' },
  { tag: tags.bool, color: '#f07178' },
  { tag: tags.number, color: '#f07178' },
  { tag: tags.string, color: '#c3e88d' },
  { tag: tags.function(tags.variableName), color: '#8fd3c7' },
  { tag: tags.variableName, color: '#e6d9c8' },
  { tag: tags.operator, color: '#ffcb6b' },
  { tag: tags.punctuation, color: '#c8c8c2' },
])

const editorTheme = EditorView.theme({
  '&': {
    height: '100%',
    backgroundColor: 'var(--code-bg)',
    color: 'var(--code-fg)',
    fontSize: '0.875rem',
  },
  '.cm-scroller': {
    overflow: 'auto',
    fontFamily: "'Fira Code Variable', 'JetBrains Mono Variable', ui-monospace, monospace",
    lineHeight: '1.65',
  },
  '.cm-content': {
    padding: '16px 0',
    caretColor: 'var(--code-fg)',
  },
  '.cm-gutters': {
    backgroundColor: 'var(--code-bg)',
    color: '#77776f',
    border: 'none',
  },
  '.cm-activeLine': { backgroundColor: 'rgba(255, 255, 255, 0.045)' },
  '.cm-activeLineGutter': { backgroundColor: 'transparent', color: 'var(--code-fg)' },
  '.cm-cursor': { borderLeftColor: 'var(--code-fg)' },
}, { dark: true })

onMounted(() => {
  if (!host.value) return
  view = new EditorView({
    state: EditorState.create({
      doc: props.modelValue,
      extensions: [
        lineNumbers(),
        highlightActiveLineGutter(),
        highlightActiveLine(),
        drawSelection(),
        history(),
        indentOnInput(),
        bracketMatching(),
        indentUnit.of('    '),
        zlangStreamLanguage,
        syntaxHighlighting(zlangHighlight),
        autocompletion({ override: [zlangCompletions], activateOnTyping: true }),
        closeBrackets(),
        editorTheme,
        EditorView.lineWrapping,
        keymap.of([
          indentWithTab,
          ...closeBracketsKeymap,
          ...completionKeymap,
          ...defaultKeymap,
          ...historyKeymap,
        ]),
        EditorView.updateListener.of(update => {
          if (update.docChanged) emit('update:modelValue', update.state.doc.toString())
        }),
      ],
    }),
    parent: host.value,
  })
})

watch(() => props.modelValue, value => {
  if (!view || view.state.doc.toString() === value) return
  view.dispatch({ changes: { from: 0, to: view.state.doc.length, insert: value } })
})

onUnmounted(() => {
  view?.destroy()
  view = null
})
</script>

<template>
  <div ref="host" class="zlang-editor" />
</template>

<style scoped>
.zlang-editor {
  height: 360px;
  overflow: hidden;
  border: 2px solid var(--code-border);
  background: var(--code-bg);
}
</style>
