<script setup>
import { onMounted, ref } from "vue";
import ImageGrid from "./components/ImageGrid.vue";
import UploadModal from "./components/UploadModal.vue";
import ImageViewer from "./components/ImageViewer.vue";
import axios from "axios";

// Data gambar
const images = ref([]);
const isModalOpen = ref(false);
const isImageViewerOpen = ref(false);
const currentImage = ref(null);
const isLoading = ref(false);

// Fungsi untuk mengambil gambar dari server
const fetchImages = async () => {
  try {
    const response = await axios.get("http://localhost:3000/api/images");
    images.value = response.data;
  } catch (err) {
    console.error("Failed to fetch images:", err);
  }
};

onMounted(async () => {
  isLoading.value = true;
  await fetchImages();
  isLoading.value = false;
});

// Handle membuka modal upload
const openUploadModal = () => {
  isModalOpen.value = true;
};

// Handle menutup modal
const closeModal = () => {
  isModalOpen.value = false;
};

// Buka image viewer dengan gambar yang dipilih
const openImageViewer = (image) => {
  currentImage.value = image;
  isImageViewerOpen.value = true;
};

// Tutup image viewer
const closeImageViewer = () => {
  isImageViewerOpen.value = false;
  setTimeout(() => {
    currentImage.value = null;
  }, 300); // Menunggu animasi selesai
};

// Handle ketika upload berhasil
const handleUploadSuccess = async () => {
  await fetchImages();
  closeModal();
};

// Handle loading state
const handleUploadLoading = (loading) => {
  isLoading.value = loading;
};
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
        v-if="images.length === 0"
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

      <ImageGrid v-else :images="images" @image-click="openImageViewer" />
    </main>

    <!-- Komponen Modal Upload -->
    <UploadModal
      :is-open="isModalOpen"
      :is-loading="isLoading"
      @close="closeModal"
      @upload-success="handleUploadSuccess"
      @upload-loading="handleUploadLoading"
    />

    <!-- Komponen Image Viewer -->
    <ImageViewer
      :is-open="isImageViewerOpen"
      :current-image="currentImage"
      @close="closeImageViewer"
    />
  </div>
</template>
