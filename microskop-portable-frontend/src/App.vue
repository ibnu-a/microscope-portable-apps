<script setup>
import { onMounted, onUnmounted, ref, watch } from "vue";
import { createClient } from "@supabase/supabase-js";
import { io } from "socket.io-client";

import ImageGrid from "./components/ImageGrid.vue";
import UploadModal from "./components/UploadModal.vue";
import ImageViewer from "./components/ImageViewer.vue";
import DeleteConfirmationModal from "./components/DeleteConfirmationModal.vue";

// --- Supabase Configuration ---
const supabaseUrl = import.meta.env.VITE_SUPABASE_URL;
const supabaseKey = import.meta.env.VITE_SUPABASE_ANON_KEY;
const supabase = createClient(supabaseUrl, supabaseKey);

// --- Data Gambar ---
const images = ref([]);
const isModalOpen = ref(false);
const isImageViewerOpen = ref(false);
const currentImage = ref(null);
const isLoading = ref(false);
const currentImageIndex = ref(0);

const isDeleteModalOpen = ref(false);
const imageToDelete = ref(null);
const isDeleting = ref(false);

// --- Socket.IO State ---
const connectionStatus = ref("Connecting...");
const microscopeData = ref({
  zoomLevel: 1.0,
  panPosition: 0.0,
  tiltPosition: 0.0,
  zoomPercentage: 0,
  panPercentage: 0,
  tiltPercentage: 0,
});
let socket = null;

// Fungsi untuk mengambil gambar dari server
const fetchImages = async () => {
  try {
    isLoading.value = true;
    const { data, error } = await supabase
      .from("image_metadata")
      .select("*")
      .order("created_at", { ascending: false });

    if (error) {
      return;
    }

    images.value = data;

    if (images.value.length > 0) {
      currentImage.value = images.value[currentImageIndex.value];
    }
  } catch (err) {
    console.error("Terjadi error:", err);
  } finally {
    isLoading.value = false;
  }
};

// --- Socket.IO connection and event listener setup ---
const setupSocketIOListerners = () => {
  const BACKEND_URL = import.meta.env.VITE_BACKEND_URL;

  if (socket) {
    socket.disconnect();
  }

  socket = io(BACKEND_URL);

  socket.on("connect", () => {
    console.log("Connected to Socket.IO server from Parent");
    connectionStatus.value = "Connected";
  });

  socket.on("disconnect", () => {
    console.log("Disconnected from Socket.IO server from Parent");
    connectionStatus.value = "Connection Error";
    scheduleReconnect();
  });

  socket.on("connect_error", (err) => {
    console.error("Socket.IO connection error from Parent:", err.message);
    connectionStatus.value = "Connection Error";
    scheduleReconnect();
  });

  // Listener untuk event 'potentiometerUpdate' (sekarang termasuk tilt)
  socket.on("potentiometerUpdate", (data) => {
    microscopeData.value.zoomLevel = data.zoomLevel;
    microscopeData.value.panPosition = data.panPosition;
    microscopeData.value.tiltPosition = data.tiltPosition;
    microscopeData.value.zoomPercentage = data.zoomPercentage;
    microscopeData.value.panPercentage = data.panPercentage;
    microscopeData.value.tiltPercentage = data.tiltPercentage;
  });

  // Listener untuk event 'imageNavigation' (dari rotary encoder)
  socket.on("imageNavigation", (data) => {
    if (data.action === "next") {
      nextImage();
    } else if (data.action === "previous") {
      prevImage();
    }
  });
};

const closeSocketIOListeners = () => {
  if (socket) {
    socket.disconnect();
    socket = null;
  }
};

const scheduleReconnect = () => {
  setTimeout(() => {
    if (!socket?.connected) {
      connectionStatus.value = "Connecting...";
      setupSocketIOListerners();
    }
  }, 5000);
};

// --- Fungsi Navigasi Gambar (Tidak Berubah) ---
const nextImage = () => {
  if (images.value.length === 0) return;
  currentImageIndex.value = (currentImageIndex.value + 1) % images.value.length;
  currentImage.value = images.value[currentImageIndex.value];
};

const prevImage = () => {
  if (images.value.length === 0) return;
  currentImageIndex.value =
    (currentImageIndex.value - 1 + images.value.length) % images.value.length;
  if (currentImageIndex.value < 0) {
    currentImageIndex.value = images.value.length - 1;
  }
  currentImage.value = images.value[currentImageIndex.value];
};

// --- Handle Modal dan Viewer (Tidak Berubah) ---
const openUploadModal = () => {
  isModalOpen.value = true;
};

const closeModal = () => {
  isModalOpen.value = false;
};

const openImageViewer = (image) => {
  currentImage.value = image;
  const index = images.value.findIndex((img) => img.id === image.id);
  if (index !== -1) {
    currentImageIndex.value = index;
  }
  isImageViewerOpen.value = true;
};

const closeImageViewer = () => {
  isImageViewerOpen.value = false;
};

const handleUploadSuccess = async () => {
  await fetchImages();
  closeModal();
};

const handleUploadLoading = (loading) => {
  isLoading.value = loading;
};

// --- Fungsi untuk Penghapusan Gambar ---
const handleDeleteImage = (image) => {
  imageToDelete.value = image;
  isDeleteModalOpen.value = true;
};

const confirmDeleteAction = async () => {
  if (!imageToDelete.value) return;

  isDeleting.value = true;
  try {
    const { data: storageData, error: storageError } = await supabase.storage
      .from("images")
      .remove([imageToDelete.value.storage_path]);

    if (storageError) {
      alert("Failed to delete image file. Please try again.");
      return;
    }

    const { error: dbError } = await supabase
      .from("image_metadata")
      .delete()
      .eq("id", imageToDelete.value.id);

    if (dbError) {
      alert("Failed to delete image metadata. Please try again.");
      return;
    }

    await fetchImages();
    closeDeleteModal();
  } catch (error) {
    alert("An unexpected error occurred during deletion.");
  } finally {
    isDeleting.value = false;
    imageToDelete.value = null;
  }
};

const closeDeleteModal = () => {
  isDeleteModalOpen.value = false;
  imageToDelete.value = null;
};

// --- Lifecycle Hooks ---
onMounted(async () => {
  await fetchImages();
  setupSocketIOListerners();
});

onUnmounted(() => {
  closeSocketIOListeners();
});

watch(
  currentImageIndex,
  (newIndex) => {
    if (
      images.value.length > 0 &&
      newIndex >= 0 &&
      newIndex < images.value.length
    ) {
      currentImage.value = images.value[newIndex];
    } else {
      currentImage.value = null;
    }
  },
  { immediate: true }
);
</script>

<template>
  <div class="min-h-screen bg-gray-50 font-sans antialiased">
    <header class="bg-teal-600 text-white p-4 shadow-sm">
      <div class="container mx-auto">
        <h1 class="text-2xl font-semibold">Mikroskop Portable</h1>
      </div>
    </header>

    <div class="bg-white py-4 px-6 shadow-sm mb-6 sticky top-0 z-10">
      <div
        class="container mx-auto flex flex-col sm:flex-row items-end justify-end gap-4"
      >
        <button
          @click="openUploadModal"
          :disabled="isLoading"
          class="w-full sm:w-auto bg-teal-500 hover:bg-teal-600 text-white font-medium py-2 px-4 rounded-lg shadow-sm transition duration-200 ease-in-out flex items-center justify-center gap-2"
          :class="{ 'opacity-50 cursor-not-allowed': isLoading }"
        >
          <svg
            xmlns="http://www.w3.org/2000/svg"
            class="h-5 w-5"
            viewBox="0 0 20 20"
            fill="currentColor"
          >
            <path
              fill-rule="evenodd"
              d="M3 17a1 1 0 011-1h12a1 1 0 110 2H4a1 1 0 01-1-1zM6.293 6.707a1 1 0 010-1.414l3-3a1 1 0 011.414 0l3 3a1 1 0 01-1.414 1.414L11 5.414V13a1 1 0 11-2 0V5.414L7.707 6.707a1 1 0 01-1.414 0z"
            />
          </svg>
          <span v-if="isLoading">Mengupload...</span>
          <span v-else>Upload Gambar</span>
        </button>
      </div>
    </div>

    <main class="container mx-auto px-4 pb-8">
      <div
        v-if="images.length === 0 && !isLoading"
        class="flex flex-col items-center justify-center p-8 text-center"
      >
        <svg
          xmlns="http://www.w3.org/2000/svg"
          class="h-16 w-16 text-gray-300 mb-4"
          fill="none"
          viewBox="0 0 24 24"
          stroke="currentColor"
        >
          <path
            stroke-linecap="round"
            stroke-linejoin="round"
            stroke-width="1.5"
            d="M4 16l4.586-4.586a2 2 0 012.828 0L16 16m-2-2l1.586-1.586a2 2 0 012.828 0L20 14m-6-6h.01M6 20h12a2 2 0 002-2V6a2 2 0 00-2-2H6a2 2 0 00-2 2v12a2 2 0 002 2z"
          />
        </svg>
        <h2 class="text-xl font-medium text-gray-700 mb-2">
          Belum ada gambar tersedia
        </h2>
        <p class="text-gray-500 mb-4">
          Silakan upload gambar terlebih dahulu untuk memulai
        </p>
        <button
          @click="openUploadModal"
          :disabled="isLoading"
          class="bg-teal-500 hover:bg-teal-600 text-white font-medium py-2 px-4 rounded-lg shadow-sm transition duration-200 ease-in-out flex items-center justify-center gap-2"
          :class="{ 'opacity-50 cursor-not-allowed': isLoading }"
        >
          <svg
            xmlns="http://www.w3.org/2000/svg"
            class="h-5 w-5"
            viewBox="0 0 20 20"
            fill="currentColor"
          >
            <path
              fill-rule="evenodd"
              d="M3 17a1 1 0 011-1h12a1 1 0 110 2H4a1 1 0 01-1-1zM6.293 6.707a1 1 0 010-1.414l3-3a1 1 0 011.414 0l3 3a1 1 0 01-1.414 1.414L11 5.414V13a1 1 0 11-2 0V5.414L7.707 6.707a1 1 0 01-1.414 0z"
            />
          </svg>
          <span v-if="isLoading">Mengupload...</span>
          <span v-else>Upload Gambar Sekarang</span>
        </button>
      </div>

      <div
        v-else-if="isLoading"
        class="flex items-center justify-center text-gray-700 p-8"
      >
        <div
          class="animate-spin rounded-full h-8 w-8 border-4 border-teal-500 border-t-transparent mr-3"
        ></div>
        <span>Memuat gambar...</span>
      </div>

      <ImageGrid
        v-else
        :images="images"
        @image-click="openImageViewer"
        @delete-image="handleDeleteImage"
      />
    </main>

    <UploadModal
      :is-open="isModalOpen"
      :is-loading="isLoading"
      @close="closeModal"
      @upload-success="handleUploadSuccess"
      @upload-loading="handleUploadLoading"
    />

    <ImageViewer
      :is-open="isImageViewerOpen"
      :current-image="currentImage"
      :current-image-index="currentImageIndex"
      :total-images="images.length"
      :microscope-data="microscopeData"
      :connection-status="connectionStatus"
      @close="closeImageViewer"
    />

    <DeleteConfirmationModal
      :is-open="isDeleteModalOpen"
      :image-name="imageToDelete ? imageToDelete.original_filename : ''"
      :is-loading="isDeleting"
      @confirm="confirmDeleteAction"
      @cancel="closeDeleteModal"
    />
  </div>
</template>
