/**
 * Main-thread facade for the Zlang Web Worker.
 *
 * The UI stays responsive while clang and lld run. Stage updates are forwarded
 * to the caller so code blocks can show the current step.
 */

interface PendingRun {
  resolve: (value: any) => void
  reject: (reason: Error) => void
  onStage: (stage: string) => void
}

let worker: Worker | null = null
let nextId = 0
const pending = new Map<number, PendingRun>()

function getWorker(): Worker {
  if (worker) return worker

  worker = new Worker(new URL('../workers/zlang.worker.ts', import.meta.url), {
    type: 'module',
  })

  worker.addEventListener('message', event => {
    const message = event.data
    const job = pending.get(message.id)
    if (!job) return

    if (message.type === 'stage') {
      job.onStage(message.stage)
      return
    }

    pending.delete(message.id)
    if (message.type === 'done') job.resolve(message.result)
    else job.reject(new Error(message.message || 'Zlang run failed'))
  })

  worker.addEventListener('error', event => {
    const failure = new Error(event.message || 'Zlang worker failed')
    for (const job of pending.values()) job.reject(failure)
    pending.clear()
  })

  return worker
}

export function runZlang(source: string, onStage: (stage: string) => void = () => {}): Promise<any> {
  return new Promise((resolve, reject) => {
    const id = ++nextId
    pending.set(id, { resolve, reject, onStage })
    getWorker().postMessage({ id, source })
  })
}

export function downloadWasm(wasm: Uint8Array | null, name = 'app.wasm') {
  if (!wasm) return
  const blob = new Blob([wasm], { type: 'application/wasm' })
  const url = URL.createObjectURL(blob)
  const anchor = document.createElement('a')
  anchor.href = url
  anchor.download = name
  anchor.click()
  URL.revokeObjectURL(url)
}
