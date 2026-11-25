<script setup lang="ts">
const traces = [
  { from: 'integer', to: 'int32_t', left: '6%', top: '26%', delay: 80 },
  { from: 'unsigned', to: 'uint32_t', left: '78%', top: '27%', delay: 380 },
  { from: 'boolean', to: 'bool', left: '84%', top: '35%', delay: 680 },
  { from: 'string', to: 'zlang_string_t', left: '4%', top: '39%', delay: 980 },
  { from: 'array[10]', to: 'data_[10]', left: '68%', top: '73%', delay: 1280 },
  { from: 'const', to: 'const', left: '12%', top: '69%', delay: 1580 },
  { from: 'function', to: 'static', left: '80%', top: '61%', delay: 1880 },
  { from: 'record', to: 'struct', left: '5%', top: '82%', delay: 2180 },
  { from: 'enum', to: 'enum', left: '76%', top: '86%', delay: 2480 },
  { from: 'if', to: 'if', left: '22%', top: '30%', delay: 2780 },
  { from: 'else', to: 'else', left: '54%', top: '34%', delay: 3080 },
  { from: 'for', to: 'for', left: '31%', top: '88%', delay: 3380 },
  { from: 'foreach', to: 'for', left: '61%', top: '80%', delay: 3680 },
  { from: 'while', to: 'while', left: '2%', top: '55%', delay: 3980 },
  { from: 'repeat', to: 'for', left: '89%', top: '49%', delay: 4280 },
  { from: 'return', to: 'return', left: '17%', top: '47%', delay: 4580 },
  { from: 'break', to: 'break', left: '72%', top: '38%', delay: 4880 },
  { from: 'continue', to: 'continue', left: '28%', top: '59%', delay: 5180 },
  { from: 'clone', to: 'value', left: '87%', top: '24%', delay: 5480 },
  { from: 'reference', to: 'pointer', left: '9%', top: '45%', delay: 5780 },
  { from: 'import', to: 'activate', left: '64%', top: '31%', delay: 6080 },
  { from: 'length', to: 'extent', left: '43%', top: '86%', delay: 6380 },
  { from: 'write', to: 'runtime call', left: '38%', top: '27%', delay: 6680 },
  { from: 'true', to: 'true', left: '93%', top: '69%', delay: 6980 },
  { from: 'false', to: 'false', left: '1%', top: '91%', delay: 7280 },
]
</script>

<template>
  <div class="trace-field" aria-hidden="true">
    <span class="trace-field__line" />
    <span
      v-for="(trace, index) in traces"
      :key="`${trace.from}-${index}`"
      class="trace-token"
      :style="{
        left: trace.left,
        top: trace.top,
        '--trace-delay': `-${trace.delay}ms`,
        '--twinkle-delay': `-${trace.delay % 1900}ms`,
      }"
    >
      <span class="trace-token__from">{{ trace.from }}</span>
      <span class="trace-token__to">{{ trace.to }}</span>
    </span>
  </div>
</template>
