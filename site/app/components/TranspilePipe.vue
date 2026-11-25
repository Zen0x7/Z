<template>
  <div class="transpile-flow" aria-hidden="true">
    <div class="transpile-stage">
      <ProjectFolderIcon />
      <span class="transpile-stage__label">project .z</span>
    </div>

    <span class="transpile-arrow">
      <span class="transpile-arrow__line" />
      <span class="transpile-arrow__head" />
    </span>

    <div class="transpile-stage">
      <FileTypeIcon extension=".c" tone="c" />
      <span class="transpile-stage__label">C source</span>
    </div>

    <span class="transpile-arrow">
      <span class="transpile-arrow__line" />
      <span class="transpile-arrow__head" />
    </span>

    <div class="transpile-stage">
      <FileTypeIcon extension=".exe" tone="exe" />
      <span class="transpile-stage__label">program</span>
    </div>
  </div>
</template>
