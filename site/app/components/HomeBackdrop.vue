<template>
  <div class="home-backdrop" aria-hidden="true">
    <svg viewBox="0 0 1200 720" preserveAspectRatio="xMidYMid slice">
      <defs>
        <pattern id="home-grid" width="48" height="48" patternUnits="userSpaceOnUse">
          <circle cx="1" cy="1" r="1" fill="currentColor" />
        </pattern>
      </defs>

      <rect width="1200" height="720" fill="url(#home-grid)" />

      <g class="home-backdrop__ruler">
        <path d="M520 190h620" />
        <path d="M540 184v12M600 187v6M660 187v6M720 184v12M780 187v6M840 187v6M900 184v12M960 187v6M1020 187v6M1080 184v12M1140 187v6" />
      </g>

      <g class="home-backdrop__flow">
        <path class="home-backdrop__flow-line" d="M540 190h600" />

        <g class="home-backdrop__node" transform="translate(570 160)">
          <rect width="60" height="60" />
          <text x="30" y="38" text-anchor="middle">.z</text>
          <text class="home-backdrop__caption" x="30" y="80" text-anchor="middle">source</text>
        </g>

        <g class="home-backdrop__node" transform="translate(810 160)">
          <rect width="60" height="60" />
          <text x="30" y="38" text-anchor="middle">zc</text>
          <text class="home-backdrop__caption" x="30" y="80" text-anchor="middle">compiler</text>
        </g>

        <g class="home-backdrop__node" transform="translate(1050 160)">
          <rect width="60" height="60" />
          <text x="30" y="38" text-anchor="middle">C</text>
          <text class="home-backdrop__caption" x="30" y="80" text-anchor="middle">output</text>
        </g>
      </g>
    </svg>
  </div>
</template>
