<script setup lang="ts">
import { useAttrs } from 'vue'

defineOptions({ inheritAttrs: false })

const props = withDefaults(defineProps<{ alt?: string }>(), {
  alt: 'Zlang',
})

const attrs = useAttrs()
const colorMode = useColorMode()
const src = computed(() => colorMode.value === 'dark' ? '/logo-dark.svg' : '/logo-light.svg')
</script>

<template>
  <img v-bind="attrs" :src="src" :alt="props.alt">
</template>
