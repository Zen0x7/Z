/// <reference lib="webworker" />

import { runZlangCore } from '~/lib/zlangCore'

interface RunRequest {
  id: number
  source: string
}

self.addEventListener('message', async (event: MessageEvent<RunRequest>) => {
  const { id, source } = event.data

  try {
    const result = await runZlangCore(source, stage => {
      self.postMessage({ id, type: 'stage', stage })
    })
    self.postMessage(
      { id, type: 'done', result },
      [result.wasm.buffer],
    )
  } catch (error) {
    self.postMessage({
      id,
      type: 'error',
      message: error instanceof Error ? error.message : String(error),
    })
  }
})
