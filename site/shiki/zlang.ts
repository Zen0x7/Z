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
          match: String.raw`\b\d+[uU]?\b`,
        },
      ],
    },
    declarations: {
      patterns: [
        {
          match: String.raw`\b(function)\s+([A-Za-z_]\w*)`,
          captures: {
            1: { name: 'storage.type.function.zlang' },
            2: { name: 'entity.name.function.zlang' },
          },
        },
        {
          match: String.raw`\b(struct|enum)\s+([A-Za-z_]\w*)`,
          captures: {
            1: { name: 'storage.type.zlang' },
            2: { name: 'entity.name.type.zlang' },
          },
        },
      ],
    },
    variables: {
      patterns: [
        {
          match: String.raw`\b(boolean|integer|unsigned|string)(?:\[([^\]]*)\])?\s+([A-Za-z_]\w*)`,
          captures: {
            1: { name: 'storage.type.zlang' },
            2: { name: 'constant.numeric.integer.zlang' },
            3: { name: 'variable.other.declaration.zlang' },
          },
        },
        {
          match: String.raw`\b(array)(?:\[([^\]]*)\])?\s+(boolean|integer|unsigned|string|[A-Za-z_]\w*)(?:\[([^\]]*)\])?\s+([A-Za-z_]\w*)`,
          captures: {
            1: { name: 'storage.type.zlang' },
            2: { name: 'constant.numeric.integer.zlang' },
            3: { name: 'storage.type.zlang' },
            4: { name: 'constant.numeric.integer.zlang' },
            5: { name: 'variable.other.declaration.zlang' },
          },
        },
        {
          match: String.raw`\b([A-Za-z_]\w*)\s*(?:\+=|-=|\*=|/=|%=|=)(?!=)`,
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
      ],
    },
    modifiers: {
      patterns: [
        {
          name: 'storage.modifier.zlang',
          match: String.raw`\b(const|clone|reference)\b`,
        },
      ],
    },
    types: {
      patterns: [
        {
          name: 'storage.type.zlang',
          match: String.raw`\b(boolean|integer|unsigned|string|array|struct|enum|function|arguments)\b`,
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
          name: 'keyword.operator.zlang',
          match: String.raw`&&|\|\||==|!=|<=|>=|[+\-*/%<>!]`,
        },
      ],
    },
    punctuation: {
      patterns: [
        {
          name: 'punctuation.zlang',
          match: String.raw`[{}()\[\];,.]`,
        },
      ],
    },
  },
}
