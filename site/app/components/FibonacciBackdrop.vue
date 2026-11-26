<script setup lang="ts">
import * as THREE from 'three'

const props = defineProps<{ compact?: boolean }>()

const colorMode = useColorMode()
const host = ref<HTMLDivElement | null>(null)
const ready = ref(false)

const count = props.compact ? 420 : 900
const dotSize = 3.7
const dotOpacity = 0.42
const rotationStep = props.compact ? 0.0008 : 0.0006
const compactCanvasSize = 1000
const goldenAngle = Math.PI * (3 - Math.sqrt(5))
const positions = new Float32Array(count * 3)

for (let index = 0; index < count; index += 1) {
  const radius = Math.sqrt(index / count) * 2.8
  const angle = index * goldenAngle
  positions[index * 3] = Math.cos(angle) * radius
  positions[index * 3 + 1] = Math.sin(angle) * radius
  positions[index * 3 + 2] = 0
}

let renderer: THREE.WebGLRenderer | undefined
let scene: THREE.Scene | undefined
let camera: THREE.OrthographicCamera | undefined
let geometry: THREE.BufferGeometry | undefined
let dotTexture: THREE.CanvasTexture | undefined
let material: THREE.PointsMaterial | undefined
let cluster: THREE.Group | undefined
let frame = 0
let resizeObserver: ResizeObserver | undefined
let reducedMotion = false

function createDotTexture() {
  const canvas = document.createElement('canvas')
  canvas.width = 64
  canvas.height = 64

  const context = canvas.getContext('2d')
  if (!context) return undefined

  const gradient = context.createRadialGradient(32, 32, 0, 32, 32, 32)
  gradient.addColorStop(0, 'rgba(255,255,255,1)')
  gradient.addColorStop(0.45, 'rgba(255,255,255,0.9)')
  gradient.addColorStop(1, 'rgba(255,255,255,0)')

  context.fillStyle = gradient
  context.fillRect(0, 0, 64, 64)

  return new THREE.CanvasTexture(canvas)
}

function resize() {
  if (!renderer || !camera || !host.value) return

  const width = props.compact ? compactCanvasSize : window.innerWidth * 0.8
  const height = props.compact ? compactCanvasSize : window.innerHeight * 0.8
  const aspect = width / Math.max(height, 1)

  camera.left = -2.1 * aspect
  camera.right = 2.1 * aspect
  camera.top = 2.1
  camera.bottom = -2.1
  camera.updateProjectionMatrix()

  if (cluster) {
    cluster.position.x = props.compact ? 0 : camera.right * 0.72
  }

  renderer.setSize(width, height, false)
  renderer.render(scene!, camera!)
  ready.value = true
}

function tick() {
  if (cluster && renderer && camera && scene) {
    cluster.rotation.z += rotationStep
    renderer.render(scene, camera)
  }
  frame = requestAnimationFrame(tick)
}

onMounted(() => {
  if (!host.value) return

  reducedMotion = window.matchMedia('(prefers-reduced-motion: reduce)').matches

  try {
    renderer = new THREE.WebGLRenderer({ alpha: true, antialias: true })
  } catch {
    return
  }

  renderer.setPixelRatio(Math.min(window.devicePixelRatio || 1, 2))
  renderer.setClearColor(0x000000, 0)
  renderer.domElement.setAttribute('aria-hidden', 'true')
  host.value.appendChild(renderer.domElement)

  scene = new THREE.Scene()
  camera = new THREE.OrthographicCamera(-2, 2, 2, -2, 0.1, 10)
  camera.position.z = 2

  cluster = new THREE.Group()

  geometry = new THREE.BufferGeometry()
  geometry.setAttribute('position', new THREE.BufferAttribute(positions, 3))

  dotTexture = createDotTexture()

  material = new THREE.PointsMaterial({
    color: props.compact ? 0xfcfcf8 : colorMode.value === 'dark' ? 0xfcfcf8 : 0x111111,
    map: dotTexture,
    alphaTest: 0.02,
    size: dotSize,
    sizeAttenuation: false,
    transparent: true,
    opacity: dotOpacity,
  })

  cluster.add(new THREE.Points(geometry, material))
  scene.add(cluster)

  resize()
  resizeObserver = new ResizeObserver(resize)
  resizeObserver.observe(host.value)

  if (!reducedMotion) frame = requestAnimationFrame(tick)
})

watch(() => colorMode.value, () => {
  if (props.compact || !material) return
  material.color.set(colorMode.value === 'dark' ? 0xfcfcf8 : 0x111111)
  if (renderer && camera && scene) renderer.render(scene, camera)
})

onUnmounted(() => {
  cancelAnimationFrame(frame)
  resizeObserver?.disconnect()
  geometry?.dispose()
  material?.dispose()
  dotTexture?.dispose()
  renderer?.dispose()
  if (renderer?.domElement.parentNode) {
    renderer.domElement.parentNode.removeChild(renderer.domElement)
  }
})
</script>

<template>
  <div
    ref="host"
    class="fib-backdrop"
    :class="[ready && 'fib-backdrop--ready', compact && 'fib-backdrop--compact']"
    aria-hidden="true"
  />
</template>
