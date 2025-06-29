<template>
  <div
    v-if="isOpen"
    class="image-viewer-overlay fixed inset-0 bg-black bg-opacity-95 flex items-center justify-center z-50"
    :class="{ active: isOpen }"
    @click="closeImageViewer"
  >
    <div
      class="absolute top-4 left-4 bg-black/60 text-white p-4 rounded-lg border border-gray-600 z-20"
    >
      <div class="space-y-2 text-sm">
        <div class="flex items-center space-x-2">
          <div
            :class="connectionStatusClass"
            class="w-2 h-2 rounded-full"
          ></div>
          <span>{{ connectionStatus }}</span>
        </div>
        <div>
          Zoom: {{ currentZoom.toFixed(2) }}x ({{
            microscopeData.zoomPercentage
          }}%)
        </div>
        <div>
          Pan: {{ microscopeData.panPosition.toFixed(1) }} ({{
            microscopeData.panPercentage
          }}%)
        </div>
        <div>
          Tilt: {{ microscopeData.tiltPosition.toFixed(1) }} ({{
            microscopeData.tiltPercentage
          }}%)
        </div>
      </div>
    </div>

    <button
      class="absolute top-4 right-4 text-white bg-black bg-opacity-70 hover:bg-opacity-90 p-3 rounded-full transition-all duration-200 z-20 border border-gray-600"
      @click.stop="closeImageViewer"
    >
      <svg
        class="h-6 w-6"
        fill="none"
        stroke="currentColor"
        viewBox="0 0 24 24"
      >
        <path
          stroke-linecap="round"
          stroke-linejoin="round"
          stroke-width="2"
          d="M6 18L18 6M6 6l12 12"
        ></path>
      </svg>
    </button>

    <div
      class="relative w-full h-full flex items-center justify-center overflow-hidden"
      @click.stop
    >
      <div
        v-if="currentImage"
        class="image-wrapper flex items-center justify-center w-full h-full"
      >
        <img
          :src="currentImage.public_url"
          :alt="currentImage.original_filename"
          class="viewer-image object-contain shadow-2xl"
          :style="imageTransformStyle"
          @dragstart.prevent
          @load="onImageLoad"
          @error="onImageError"
        />
      </div>

      <div v-else class="flex items-center justify-center text-white">
        <div
          class="animate-spin rounded-full h-16 w-16 border-4 border-blue-500 border-t-transparent"
        ></div>
        <span class="ml-4 text-lg">Loading microscope image...</span>
      </div>

      <div
        v-if="imageError"
        class="flex flex-col items-center justify-center text-white"
      >
        <svg
          class="h-16 w-16 text-red-500 mb-4"
          fill="none"
          stroke="currentColor"
          viewBox="0 0 24 24"
        >
          <path
            stroke-linecap="round"
            stroke-linejoin="round"
            stroke-width="2"
            d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-2.5L13.732 4c-.77-.833-1.996-.833-2.732 0L3.732 16.5c-.77.833.192 2.5 1.732 2.5z"
          ></path>
        </svg>
        <span class="text-lg">Failed to load image</span>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, watch, onMounted, onUnmounted, computed } from "vue";

const props = defineProps({
  isOpen: Boolean,
  currentImage: Object,
  currentImageIndex: Number, // PROP BARU
  totalImages: Number, // PROP BARU
  microscopeData: Object,
  connectionStatus: String,
});

const emit = defineEmits(["close"]);

const currentZoom = ref(1.0);
const panOffset = ref(0);
const tiltOffset = ref(0); // STATE BARU: Untuk pergeseran vertikal
const imageError = ref(false);

// Animation control
let zoomAnimationFrame = null;
let panAnimationFrame = null;
let tiltAnimationFrame = null; // ANIMASI BARU untuk tilt

// Computed properties
const connectionStatusClass = computed(() => {
  switch (props.connectionStatus) {
    case "Connected":
      return "bg-green-500";
    case "Connecting...":
      return "bg-yellow-500 animate-pulse";
    case "Connection Error":
      return "bg-red-500";
    default:
      return "bg-gray-500";
  }
});

const imageTransformStyle = computed(() => {
  // Ganti transform menjadi transform: `scale(...) translateX(...) translateY(...)`
  return {
    transform: `scale(${currentZoom.value}) translateX(${panOffset.value}px) translateY(${tiltOffset.value}px)`,
    transformOrigin: "center center",
    transition: "none",
    maxWidth: "90vw",
    maxHeight: "90vh",
    willChange: "transform",
  };
});

// Animation functions (disederhanakan untuk tilt)
const animateZoomTo = (targetZoom) => {
  if (zoomAnimationFrame) {
    cancelAnimationFrame(zoomAnimationFrame);
  }
  const startZoom = currentZoom.value;
  const zoomDifference = targetZoom - startZoom;
  const animationDuration = 50;
  const startTime = performance.now();

  const animate = (currentTime) => {
    const elapsed = currentTime - startTime;
    const progress = Math.min(elapsed / animationDuration, 1);
    const easeInOutCubic =
      progress < 0.5
        ? 4 * progress * progress * progress
        : 1 - Math.pow(-2 * progress + 2, 3) / 2;
    currentZoom.value = startZoom + zoomDifference * easeInOutCubic;
    if (progress < 1) {
      zoomAnimationFrame = requestAnimationFrame(animate);
    } else {
      currentZoom.value = targetZoom;
      zoomAnimationFrame = null;
    }
  };
  zoomAnimationFrame = requestAnimationFrame(animate);
};

const animatePanTo = (targetPan) => {
  if (panAnimationFrame) {
    cancelAnimationFrame(panAnimationFrame);
  }
  const startPan = panOffset.value;
  const panDifference = targetPan - startPan;
  const animationDuration = 50;
  const startTime = performance.now();

  const animate = (currentTime) => {
    const elapsed = currentTime - startTime;
    const progress = Math.min(elapsed / animationDuration, 1);
    const easeOut = 1 - Math.pow(1 - progress, 3);
    panOffset.value = startPan + panDifference * easeOut;
    if (progress < 1) {
      panAnimationFrame = requestAnimationFrame(animate);
    } else {
      panOffset.value = targetPan;
      panAnimationFrame = null;
    }
  };
  panAnimationFrame = requestAnimationFrame(animate);
};

// FUNGSI BARU: Animasi Tilt
const animateTiltTo = (targetTilt) => {
  if (tiltAnimationFrame) {
    cancelAnimationFrame(tiltAnimationFrame);
  }
  const startTilt = tiltOffset.value;
  const tiltDifference = targetTilt - startTilt;
  const animationDuration = 50;
  const startTime = performance.now();

  const animate = (currentTime) => {
    const elapsed = currentTime - startTime;
    const progress = Math.min(elapsed / animationDuration, 1);
    const easeOut = 1 - Math.pow(1 - progress, 3);
    tiltOffset.value = startTilt + tiltDifference * easeOut;
    if (progress < 1) {
      tiltAnimationFrame = requestAnimationFrame(animate);
    } else {
      tiltOffset.value = targetTilt;
      tiltAnimationFrame = null;
    }
  };
  tiltAnimationFrame = requestAnimationFrame(animate);
};

const updatePanPosition = () => {
  const maxPanOffset = Math.min(window.innerWidth * 0.3, 400);
  const newPanOffset = (props.microscopeData.panPosition / 100) * maxPanOffset;
  animatePanTo(newPanOffset);
};

// FUNGSI BARU: Update Tilt Position
const updateTiltPosition = () => {
  const maxTiltOffset = Math.min(window.innerHeight * 0.3, 300); // Max offset vertikal (sesuaikan)
  const newTiltOffset =
    (props.microscopeData.tiltPosition / 100) * maxTiltOffset;
  animateTiltTo(newTiltOffset);
};

// Event handlers
const closeImageViewer = () => {
  emit("close");
};

// Reset view akan mereset zoom, pan, dan tilt
const resetView = () => {
  animateZoomTo(1.0);
  animatePanTo(0);
  animateTiltTo(0); // Reset tilt juga
};

const onImageLoad = () => {
  imageError.value = false;
};

const onImageError = () => {
  imageError.value = true;
};

const handleKeyPress = (event) => {
  if (!props.isOpen) return;
  switch (event.key) {
    case "Escape":
      closeImageViewer();
      break;
    case "r":
    case "R":
      resetView();
      break;
  }
};

// Lifecycle
onMounted(() => {
  document.addEventListener("keydown", handleKeyPress);
  // Tidak ada setup Socket.IO di sini lagi
});

onUnmounted(() => {
  document.removeEventListener("keydown", handleKeyPress);
  // Tidak ada close Socket.IO di sini lagi
  if (zoomAnimationFrame) cancelAnimationFrame(zoomAnimationFrame);
  if (panAnimationFrame) cancelAnimationFrame(panAnimationFrame);
  if (tiltAnimationFrame) cancelAnimationFrame(tiltAnimationFrame); // Batalkan animasi tilt
});

// Watch for prop changes
watch(
  () => props.microscopeData.zoomLevel,
  (newZoomLevel) => {
    const newZoom = Math.max(0.5, Math.min(5.0, newZoomLevel));
    animateZoomTo(newZoom);
  }
);

watch(
  () => props.microscopeData.panPosition,
  () => {
    updatePanPosition();
  }
);

// WATCHER BARU untuk Tilt Position
watch(
  () => props.microscopeData.tiltPosition,
  () => {
    updateTiltPosition();
  }
);

// Watch currentImage prop untuk mereset tampilan saat gambar berubah
watch(
  () => props.currentImage,
  (newImage, oldImage) => {
    if (newImage && newImage !== oldImage) {
      console.log("Image changed, resetting view...");
      resetView(); // Reset zoom, pan, dan tilt saat gambar baru dimuat
    }
  }
);
</script>

<style scoped>
/* CSS Anda tetap sama */
.image-viewer-overlay {
  backdrop-filter: blur(8px);
}

.image-viewer-overlay.active {
  animation: fadeIn 0.4s ease-out;
}

@keyframes fadeIn {
  from {
    opacity: 0;
  }
  to {
    opacity: 1;
  }
}

.viewer-image {
  user-select: none;
  -webkit-user-drag: none;
  image-rendering: crisp-edges;
}

/* Scrollbar styling */
::-webkit-scrollbar {
  width: 4px;
}

::-webkit-scrollbar-track {
  background: rgba(0, 0, 0, 0.1);
}

::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.3);
  border-radius: 2px;
}

::-webkit-scrollbar-thumb:hover {
  background: rgba(255, 255, 255, 0.5);
}

/* Performance optimizations */
.image-wrapper {
  transform: translateZ(0);
}
</style>
