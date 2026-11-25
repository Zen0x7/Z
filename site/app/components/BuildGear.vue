<template>
  <svg class="build-gear" viewBox="0 0 120 120" aria-hidden="true">
    <g fill="none" stroke="currentColor" stroke-width="2">
      <circle cx="60" cy="60" r="27" />
      <circle cx="60" cy="60" r="10" />
      <g v-for="tooth in 10" :key="tooth" :transform="`rotate(${tooth * 36} 60 60)`">
        <rect x="54" y="8" width="12" height="20" />
      </g>
    </g>
  </svg>
</template>
