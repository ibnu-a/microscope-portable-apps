<script setup>
import { ref } from "vue";
import axios from "axios";

const props = defineProps({
  isOpen: Boolean,
  isLoading: Boolean, // Ini akan dikendalikan oleh komponen induk melalui emit
});

const emit = defineEmits(["close", "upload-loading", "upload-success"]);

const fileInput = ref(null);
const uploadError = ref(null);

// Tutup modal ketika mengklik backdrop
const handleModalClick = (event) => {
  if (event.target.classList.contains("modal-overlay")) {
    emit("close");
  }
};

// Trigger klik input file
const triggerFileInput = () => {
  if (fileInput.value) {
    fileInput.value.click();
  }
};

// Handle upload gambar ke backend
const handleFileChange = async (event) => {
  const files = event.target.files;
  if (!files.length) return;

  emit("upload-loading", true); // Set loading state di komponen induk
  uploadError.value = null; // Bersihkan error

  const file = files[0]; // Hanya ambil file pertama karena backend kita menerima single image

  // Buat FormData untuk mengirim file
  const formData = new FormData();
  // Sesuaikan nama field dengan yang diharapkan oleh Multer di backend ('image')
  formData.append("image", file);

  // URL endpoint upload di backend Anda
  const backendUploadUrl = "http://localhost:3000/api/images/upload"; // Sesuaikan dengan URL backend Anda

  try {
    // Kirim file ke backend untuk diupload ke Cloudinary
    const response = await axios.post(backendUploadUrl, formData, {
      headers: {},
    });

    const uploadedImageData = response.data.image; // Backend mengembalikan { message, image: {...} }
    console.log(
      "Upload berhasil ke backend dan Cloudinary:",
      uploadedImageData
    );

    // Emit success event ke komponen induk, sertakan data gambar yang diupload
    emit("upload-success", uploadedImageData);

    // Bersihkan input file setelah sukses
    if (fileInput.value) {
      fileInput.value.value = "";
    }
  } catch (error) {
    console.error(
      "Gagal mengupload gambar via backend:",
      error.response ? error.response.data : error.message || error
    );
    // Tampilkan pesan error dari backend jika tersedia
    uploadError.value =
      error.response?.data?.message ||
      `Gagal mengupload "${file.name}". Silakan coba lagi.`;
  } finally {
    emit("upload-loading", false); // Sembunyikan loading di komponen induk
  }
};
</script>

<template>
  <div
    v-if="isOpen"
    class="modal-overlay fixed inset-0 bg-black/50 flex items-center justify-center z-50"
    @click="handleModalClick"
  >
    <div
      class="bg-white rounded-lg shadow-xl w-full max-w-md mx-4 overflow-hidden"
      @click.stop
    >
      <div class="flex justify-between items-center border-b px-6 py-4">
        <h3 class="text-lg font-medium text-gray-900">Upload Gambar</h3>
        <button
          class="text-gray-400 hover:text-gray-500"
          @click="$emit('close')"
          :disabled="isLoading"
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
      </div>
      <div class="p-6">
        <div
          class="border-2 border-dashed border-gray-300 rounded-lg p-8 text-center transition-colors duration-200"
          :class="{
            'hover:border-teal-500 cursor-pointer': !isLoading,
            'opacity-60 cursor-not-allowed': isLoading,
          }"
          @click="!isLoading && triggerFileInput()"
        >
          <svg
            xmlns="http://www.w3.org/2000/svg"
            class="mx-auto h-12 w-12 text-gray-400"
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
          <h4 class="mt-2 text-sm font-medium text-gray-700">
            Pilih file atau seret ke sini
          </h4>
          <p class="mt-1 text-xs text-gray-500">PNG, JPG, JPEG hingga 10MB</p>
          <input
            type="file"
            ref="fileInput"
            @change="handleFileChange"
            accept="image/*"
            class="hidden"
            :disabled="isLoading"
          />
        </div>
        <p v-if="isLoading" class="text-center text-teal-600 mt-4">
          Sedang mengupload gambar...
        </p>
        <p v-if="uploadError" class="text-center text-red-500 mt-4">
          {{ uploadError }}
        </p>
      </div>
      <div class="bg-gray-50 px-6 py-4 flex justify-end">
        <button
          class="mr-2 px-4 py-2 text-sm font-medium text-gray-700 hover:bg-gray-100 rounded-md"
          @click="$emit('close')"
          :disabled="isLoading"
        >
          Batal
        </button>
        <button
          class="px-4 py-2 text-sm font-medium text-white bg-teal-500 hover:bg-teal-600 rounded-md shadow-sm"
          @click="triggerFileInput"
          :disabled="isLoading"
        >
          Pilih File
        </button>
      </div>
    </div>
  </div>
</template>

<style scoped>
.modal-overlay {
  backdrop-filter: blur(2px);
  transition: all 0.3s ease;
}
</style>
