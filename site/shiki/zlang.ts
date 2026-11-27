import type { LanguageRegistration } from 'shiki'

/**
 * TextMate grammar for Zlang sources (`.z` / `.zlang`).
 * Scope names use conventional TextMate scopes so Shiki themes can colour
 * keywords, types, variables, functions, strings and comments independently.
 */
export const zlangLanguage: LanguageRegistration = {
  name: 'zlang',
  displayName: 'Zlang',
  scopeName: 'source.zlang',
  aliases: ['z'],
  patterns: [
    { include: '#comments' },
    { include: '#strings' },
    { include: '#numbers' },
    { include: '#declarations' },
    { include: '#generics' },
    { include: '#variables' },
    { include: '#keywords' },
    { include: '#modifiers' },
    { include: '#types' },
    { include: '#constants' },
    { include: '#builtins' },
    { include: '#functions' },
    { include: '#identifiers' },
    { include: '#operators' },
    { include: '#punctuation' },
  ],
  repository: {
    comments: {
      patterns: [
        {
          name: 'comment.line.double-slash.zlang',
          match: String.raw`//.*$`,
        },
        {
          name: 'comment.block.zlang',
          begin: String.raw`/\*`,
          end: String.raw`\*/`,
        },
      ],
    },
    strings: {
      patterns: [
        {
          name: 'string.quoted.double.zlang',
          begin: String.raw`"`,
          end: String.raw`"`,
          patterns: [
            {
              name: 'constant.character.escape.zlang',
              match: String.raw`\\["\\nrt0]`,
            },
          ],
        },
      ],
    },
    numbers: {
      patterns: [
        {
          name: 'constant.numeric.integer.zlang',
          match: String.raw`\b\d+(?:\.\d+)?(?:[uU]|[iI]64|[uU]64|[fF])?\b`,
        },
      ],
    },
    declarations: {
      patterns: [
        {
          match: String.raw`\b([A-Za-z_]\w*)(<)([A-Za-z_]\w*(?:\s*,\s*[A-Za-z_]\w*)*)(>)(?=\s*\()`,
          captures: {
            1: { name: 'entity.name.function.zlang' },
            2: { name: 'punctuation.zlang' },
            3: { name: 'storage.type.zlang' },
            4: { name: 'punctuation.zlang' },
          },
        },
        {
          match: String.raw`\b(function)\s+([A-Za-z_]\w*)(<)([A-Za-z_]\w*(?:\s*,\s*[A-Za-z_]\w*)*)(>)`,
          captures: {
            1: { name: 'keyword.control.zlang' },
            2: { name: 'entity.name.function.zlang' },
            3: { name: 'punctuation.zlang' },
            4: { name: 'storage.type.zlang' },
            5: { name: 'punctuation.zlang' },
          },
        },
        {
          match: String.raw`\b(function)\s+([A-Za-z_]\w*)`,
          captures: {
            1: { name: 'keyword.control.zlang' },
            2: { name: 'entity.name.function.zlang' },
          },
        },
        {
          match: String.raw`\b(interface|trait|struct|enum)\s+([A-Za-z_]\w*)(<)([A-Za-z_]\w*(?:\s*,\s*[A-Za-z_]\w*)*)(>)`,
          captures: {
            1: { name: 'keyword.control.zlang' },
            2: { name: 'entity.name.type.zlang' },
            3: { name: 'punctuation.zlang' },
            4: { name: 'storage.type.zlang' },
            5: { name: 'punctuation.zlang' },
          },
        },
        {
          match: String.raw`\b(interface|trait|struct|enum)\s+([A-Za-z_]\w*)`,
          captures: {
            1: { name: 'keyword.control.zlang' },
            2: { name: 'entity.name.type.zlang' },
          },
        },
        {
          match: String.raw`\b(implements)\s+([A-Za-z_]\w*)`,
          captures: {
            1: { name: 'keyword.control.zlang' },
            2: { name: 'entity.name.type.zlang' },
          },
        },
        {
          match: String.raw`\b(use|uses)\s+([A-Za-z_]\w*)`,
          captures: {
            1: { name: 'keyword.control.zlang' },
            2: { name: 'entity.name.type.zlang' },
          },
        },
        {
          match: String.raw`\b(constructor)\s*(?=\()`,
          captures: {
            1: { name: 'keyword.control.zlang' },
          },
        },
      ],
    },
    generics: {
      patterns: [
        {
          begin: String.raw`\b([A-Za-z_]\w*)(<)`,
          beginCaptures: {
            1: { name: 'entity.name.type.zlang' },
            2: { name: 'punctuation.zlang' },
          },
          end: String.raw`>`,
          endCaptures: {
            0: { name: 'punctuation.zlang' },
          },
          contentName: 'storage.type.zlang',
          patterns: [
            {
              name: 'storage.type.zlang',
              match: String.raw`[A-Za-z_]\w*`,
            },
            {
              name: 'punctuation.zlang',
              match: String.raw`,`,
            },
          ],
        },
      ],
    },
    variables: {
      patterns: [
        {
          name: 'variable.language.self.zlang',
          match: String.raw`\bself\b`,
        },
        {
          match: String.raw`\b(boolean|int32|uint32|int64|uint64|float|double|string|arguments|status_code|result_type)(?:(\[)([^\]]*)(\]))?\s+([A-Za-z_]\w*)`,
          captures: {
            1: { name: 'storage.type.zlang' },
            2: { name: 'punctuation.zlang' },
            3: { name: 'constant.numeric.integer.zlang' },
            4: { name: 'punctuation.zlang' },
            5: { name: 'variable.other.declaration.zlang' },
          },
        },
        {
          match: String.raw`\b(array)(?:(\[)([^\]]*)(\]))?\s+(boolean|int32|uint32|int64|uint64|float|double|string|arguments|status_code|result_type|[A-Za-z_]\w*)(?:(\[)([^\]]*)(\]))?\s+([A-Za-z_]\w*)`,
          captures: {
            1: { name: 'storage.type.zlang' },
            2: { name: 'punctuation.zlang' },
            3: { name: 'constant.numeric.integer.zlang' },
            4: { name: 'punctuation.zlang' },
            5: { name: 'storage.type.zlang' },
            6: { name: 'punctuation.zlang' },
            7: { name: 'constant.numeric.integer.zlang' },
            8: { name: 'punctuation.zlang' },
            9: { name: 'variable.other.declaration.zlang' },
          },
        },
        {
          match: String.raw`\b([A-Za-z_]\w*)(?=\s*(?:\+=|-=|\*=|/=|%=|=)(?!=))`,
          captures: {
            1: { name: 'variable.other.assignment.zlang' },
          },
        },
      ],
    },
    keywords: {
      patterns: [
        {
          name: 'keyword.control.zlang',
          match: String.raw`\b(if|else|while|for|foreach|in|repeat|break|continue|return|import|from)\b`,
        },
        {
          name: 'keyword.control.zlang',
          match: String.raw`\b(jump|switch|case|default|do|match)\b`,
        },
        {
          name: 'keyword.control.zlang',
          match: String.raw`\b(interface|implements|use|uses|requires)\b`,
        },
      ],
    },
    modifiers: {
      patterns: [
        {
          name: 'storage.modifier.zlang',
          match: String.raw`\b(const|clone|reference|static)\b`,
        },
      ],
    },
    types: {
      patterns: [
        {
          name: 'storage.type.zlang',
          match: String.raw`\b(boolean|int32|uint32|int64|uint64|float|double|string|array|arguments|stream|status_code|result_type)\b`,
        },
        {
          name: 'storage.type.zlang',
          match: String.raw`\b[A-Z][A-Za-z0-9_]*\b(?=\s+[A-Za-z_]\w*\s*(?:[,)={;]|\s))`,
        },
        {
          name: 'entity.name.type.zlang',
          match: String.raw`\b(?:use|uses|implements|requires)\s+([A-Z][A-Za-z0-9_]*)\b`,
          captures: {
            1: { name: 'entity.name.type.zlang' },
          },
        },
        {
          name: 'entity.name.type.zlang',
          match: String.raw`\b[A-Z][A-Za-z0-9_]*\b(?=\s*\.)`,
        },
        {
          name: 'entity.name.type.zlang',
          match: String.raw`\b[A-Z][A-Za-z0-9_]*\b(?=\s*\()`,
        },
        {
          match: String.raw`(:)\s*([A-Z][A-Za-z0-9_]*)\b(?=\s*\{)`,
          captures: {
            1: { name: 'punctuation.zlang' },
            2: { name: 'entity.name.type.zlang' },
          },
        },
      ],
    },
    constants: {
      patterns: [
        {
          name: 'constant.language.boolean.zlang',
          match: String.raw`\b(true|false)\b`,
        },
        {
          name: 'constant.language.wildcard.zlang',
          match: String.raw`(?<![A-Za-z0-9_])_(?![A-Za-z0-9_])`,
        },
        {
          name: 'constant.language.enum.zlang',
          match: String.raw`\b[A-Z][A-Z0-9_]*\b`,
        },
      ],
    },
    builtins: {
      patterns: [
        {
          name: 'support.function.builtin.zlang',
          match: String.raw`\b(length|write)\b(?=\s*\()`,
        },
        {
          name: 'support.function.result.zlang',
          match: String.raw`\b(is_ok|get|message|time|dump)\b(?=\s*\()`,
        },
      ],
    },
    functions: {
      patterns: [
        {
          match: String.raw`\b([A-Za-z_]\w*)(?=\s*\()`,
          captures: {
            1: { name: 'entity.name.function.zlang' },
          },
        },
      ],
    },
    identifiers: {
      patterns: [
        {
          name: 'variable.other.read.zlang',
          match: String.raw`\b[A-Za-z_]\w*\b`,
        },
      ],
    },
    operators: {
      patterns: [
        {
          name: 'keyword.operator.assignment.zlang',
          match: String.raw`(?:\+=|-=|\*=|/=|%=|=)`,
        },
        {
          name: 'keyword.operator.match.zlang',
          match: String.raw`=>`,
        },
        {
          name: 'keyword.operator.range.zlang',
          match: String.raw`\.\.`,
        },
        {
          name: 'keyword.operator.ternary.zlang',
          match: String.raw`\?`,
        },
        {
          name: 'keyword.operator.zlang',
          match: String.raw`&&|\|\||<<|>>|==|!=|<=|>=|[+\-*/%<>!&|^~]`,
        },
      ],
    },
    punctuation: {
      patterns: [
        {
          name: 'punctuation.zlang',
          match: String.raw`[{}()\[\];,.:]`,
        },
      ],
    },
  },
}
