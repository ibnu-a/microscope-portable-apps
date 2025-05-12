<script setup>
import { ref, watch, onMounted, onUnmounted, computed } from "vue";

const props = defineProps({
  isOpen: Boolean,
  currentImage: Object,
});

const emit = defineEmits(["close"]);

// Reactive state for zoom level and image position
const currentZoom = ref(1.0); // Initial zoom level, will be updated by WebSocket
const translateX = ref(0); // X position for panning
const translateY = ref(0); // Y position for panning

// WebSocket connection instance
let ws = null;

// Panning state variables
let isDragging = false;
let startX = 0;
let startY = 0;
let initialTranslateX = 0;
let initialTranslateY = 0;

// Function to set up the WebSocket connection
const setupWebSocket = () => {
  // IMPORTANT: Replace with the actual IP address and port of your Node.js server
  // This should match what's in your ESP32 code and server.js
  const websocketUrl = "ws://192.168.130.45:3000";

  if (ws && ws.readyState === WebSocket.OPEN) {
    console.log("WebSocket already open.");
    return;
  }

  ws = new WebSocket(websocketUrl);

  ws.onopen = () => {
    console.log("WebSocket Connected to backend server.");
  };

  ws.onmessage = (event) => {
    try {
      const message = JSON.parse(event.data);
      if (message.action === "zoom" && typeof message.level === "number") {
        // Update zoom level. Limiting to 2 decimal places for smoother display.
        const newZoomLevel = parseFloat(message.level.toFixed(2));
        if (newZoomLevel !== currentZoom.value) {
          // Only update if value actually changed
          currentZoom.value = newZoomLevel;
          console.log("Received zoom level:", currentZoom.value);
          // Optionally, reset pan position when zoom changes significantly
          // Or implement more complex logic to keep the center point in view
          // For simplicity, we won't auto-reset pan here, letting the user control it.
        }
      }
    } catch (e) {
      console.error("Error parsing WebSocket message:", e);
    }
  };

  ws.onclose = () => {
    console.log(
      "WebSocket Disconnected from backend server. Attempting to reconnect..."
    );
    // Attempt to reconnect after a delay
    setTimeout(setupWebSocket, 3000); // Reconnect after 3 seconds
  };

  ws.onerror = (error) => {
    console.error("WebSocket Error:", error);
    ws.close(); // Close and trigger reconnect on error
  };
};

// Function to close the WebSocket connection
const closeWebSocket = () => {
  if (ws) {
    ws.close();
    ws = null;
  }
};

// Mouse event handlers for panning
const startDrag = (event) => {
  // Only allow dragging if zoomed in beyond 1x
  if (currentZoom.value > 1.0) {
    isDragging = true;
    startX = event.clientX;
    startY = event.clientY;
    initialTranslateX = translateX.value;
    initialTranslateY = translateY.value;
    event.target.style.cursor = "grabbing"; // Change cursor for visual feedback
  }
};

const doDrag = (event) => {
  if (!isDragging) return;

  const dx = event.clientX - startX;
  const dy = event.clientY - startY;

  translateX.value = initialTranslateX + dx;
  translateY.value = initialTranslateY + dy;
};

const endDrag = (event) => {
  isDragging = false;
  if (event.target) {
    // Check if target exists before setting style
    event.target.style.cursor = currentZoom.value > 1.0 ? "grab" : "default";
  }
};

// Lifecycle hooks for managing WebSocket connection
onMounted(() => {
  // Establish WebSocket connection when the component is first mounted
  setupWebSocket();
});

onUnmounted(() => {
  // Close WebSocket connection when the component is unmounted
  closeWebSocket();
});

// Watch for changes in `isOpen` prop to reset zoom/pan when viewer opens/closes
watch(
  () => props.isOpen,
  (newVal) => {
    if (newVal) {
      // When the viewer opens, ensure WebSocket is connected and reset zoom/pan
      if (!ws || ws.readyState !== WebSocket.OPEN) {
        setupWebSocket();
      }
      currentZoom.value = 1.0; // Reset zoom to default (1x) when opening a new image
      translateX.value = 0; // Reset pan position
      translateY.value = 0; // Reset pan position
    } else {
      // Optionally, you might want to close the WebSocket when the viewer is closed
      // if this component is the only one using it. However, keeping it open for
      // continuous updates across multiple image views might be desirable.
      // closeWebSocket();
    }
  },
  { immediate: true }
); // Run immediately when component is mounted to handle initial state

// Computed property to generate the CSS transform style for the image
const imageTransformStyle = computed(() => {
  return {
    transform: `scale(${currentZoom.value}) translate(${translateX.value}px, ${translateY.value}px)`,
    transformOrigin: "center center", // Ensures the image scales from its center
    transition: "transform 0.05s ease-out", // Smooth transition for zoom changes
    cursor:
      currentZoom.value > 1.0 ? (isDragging ? "grabbing" : "grab") : "default",
  };
});

// Close the image viewer when clicking outside the image
const closeImageViewer = () => {
  emit("close");
};
</script>

<template>
  <div
    v-if="isOpen"
    class="image-viewer-overlay fixed inset-0 bg-black bg-opacity-90 flex items-center justify-center z-50"
    :class="{ active: isOpen }"
    @click="closeImageViewer"
  >
    <div
      class="relative w-full h-full flex items-center justify-center overflow-hidden"
    >
      <button
        class="absolute top-4 right-4 text-white bg-black bg-opacity-50 hover:bg-opacity-70 p-2 rounded-full transition-all duration-200 z-20"
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
        v-if="currentImage"
        class="image-wrapper flex items-center justify-center w-full h-full"
        @click.stop
      >
        <img
          :src="currentImage.src"
          :alt="currentImage.name"
          class="viewer-image object-contain shadow-2xl"
          :style="imageTransformStyle"
          @mousedown="startDrag"
          @mousemove="doDrag"
          @mouseup="endDrag"
          @mouseleave="endDrag"
        />
      </div>
    </div>
  </div>
</template>

<style scoped>
.image-viewer-overlay {
  animation: fade-in 0.2s ease;
}

@keyframes fade-in {
  from {
    opacity: 0;
  }
  to {
    opacity: 1;
  }
}

.viewer-image {
  /* The transform is now controlled by the style binding */
  /* Remove max-w-full and max-h-[90vh] if you want the image to be able to scale beyond viewport */
  /* For proper zoom/pan, the image should be able to scale freely. */
  /* It will be contained by the parent with `overflow-hidden`. */
  /* A general image sizing for initial fit might be needed, e.g., setting its own width/height */
  max-width: 100%; /* Ensure it initially fits within its container */
  max-height: 90vh; /* Ensure it initially fits within the screen height */
  will-change: transform; /* Optimize for smooth transformations */
  user-select: none; /* Prevent image dragging behavior by default */
}

/* No specific animation needed on .viewer-image now, as transform is dynamic */
/* @keyframes scale-in {
  from {
    transform: scale(0.9);
    opacity: 0;
  }
  to {
    transform: scale(1);
    opacity: 1;
  }
} */

.image-wrapper {
  /* This wrapper helps to center the image initially and provides a canvas for panning */
  /* No explicit max-width/height here, allowing the image to grow via transform */
}
</style>
