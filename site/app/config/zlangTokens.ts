export interface ZlangTokenTrace {
  from: string
  to: string
  left: string
  top: string
  delay: number
}

export const zlangTokenTraces: ZlangTokenTrace[] = [
  { from: 'int32', to: 'int32_t', left: '6%', top: '26%', delay: 80 },
  { from: 'uint32', to: 'uint32_t', left: '78%', top: '27%', delay: 380 },
  { from: 'int64', to: 'int64_t', left: '84%', top: '35%', delay: 680 },
  { from: 'uint64', to: 'uint64_t', left: '4%', top: '39%', delay: 980 },
  { from: 'boolean', to: 'bool', left: '68%', top: '73%', delay: 1280 },
  { from: 'string', to: 'zlang_string_t', left: '12%', top: '69%', delay: 1580 },
  { from: 'arguments', to: 'zlang_arguments_t', left: '80%', top: '61%', delay: 1880 },
  { from: 'array[10]', to: 'data_[10]', left: '5%', top: '82%', delay: 2180 },
  { from: 'function', to: 'app_fn_', left: '76%', top: '86%', delay: 2480 },
  { from: 'struct', to: 'app_struct_', left: '22%', top: '30%', delay: 2780 },
  { from: 'enum', to: 'ZLANG_', left: '54%', top: '34%', delay: 3080 },
  { from: 'trait', to: 'method', left: '31%', top: '88%', delay: 3380 },
  { from: 'uses', to: 'direct call', left: '61%', top: '80%', delay: 3680 },
  { from: 'requires', to: 'fields', left: '2%', top: '55%', delay: 3980 },
  { from: 'constructor', to: 'self', left: '89%', top: '49%', delay: 4280 },
  { from: 'static', to: 'app_fn_', left: '17%', top: '47%', delay: 4580 },
  { from: 'generic', to: 'specialization', left: '72%', top: '38%', delay: 4880 },
  { from: 'foreach', to: 'for', left: '28%', top: '59%', delay: 5180 },
  { from: 'repeat', to: 'for', left: '87%', top: '24%', delay: 5480 },
  { from: 'reference', to: 'pointer', left: '9%', top: '45%', delay: 5780 },
  { from: 'clone', to: 'value', left: '64%', top: '31%', delay: 6080 },
  { from: 'import', to: 'activate', left: '43%', top: '86%', delay: 6380 },
  { from: 'length', to: 'extent', left: '38%', top: '27%', delay: 6680 },
  { from: 'result_type<T, status_code>', to: 'result layout', left: '93%', top: '69%', delay: 6980 },
  { from: 'write', to: 'runtime call', left: '1%', top: '91%', delay: 7280 },
]
