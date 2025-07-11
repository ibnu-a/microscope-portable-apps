<script setup>
const props = defineProps({
  isOpen: {
    type: Boolean,
    required: true,
  },
  imageName: {
    type: String,
    default: "this file",
  },
  isLoading: {
    type: Boolean,
    default: false,
  },
});

const emit = defineEmits(["confirm", "cancel"]);

const confirmDelete = () => {
  emit("confirm");
};

const cancelDelete = () => {
  emit("cancel");
};
</script>

<template>
  <div
    v-if="isOpen"
    class="fixed inset-0 bg-black/70 flex items-center justify-center z-50 transition-opacity duration-300"
  >
    <div
      class="bg-white rounded-lg shadow-xl p-6 m-4 max-w-sm w-full transform transition-transform duration-300 scale-95 opacity-0"
      :class="{ 'scale-100 opacity-100': isOpen }"
      @click.stop
    >
      <h3 class="text-lg font-semibold text-gray-900 mb-4">
        Konfirmasi Penghapusan
      </h3>
      <p class="text-gray-700 mb-6">
        Apakah Anda yakin ingin menghapus
        <span class="font-medium">"{{ imageName }}"</span>? Tindakan ini tidak
        dapat dibatalkan.
      </p>

      <div class="flex justify-end space-x-3">
        <button
          @click="cancelDelete"
          :disabled="isLoading"
          class="px-4 py-2 text-sm font-medium text-gray-700 bg-gray-200 rounded-md hover:bg-gray-300 focus:outline-none focus:ring-2 focus:ring-gray-500 focus:ring-opacity-50 transition-colors duration-200"
        >
          Cancel
        </button>
        <button
          @click="confirmDelete"
          :disabled="isLoading"
          class="px-4 py-2 text-sm font-medium text-white bg-red-600 rounded-md hover:bg-red-700 focus:outline-none focus:ring-2 focus:ring-red-500 focus:ring-opacity-50 transition-colors duration-200"
        >
          <span v-if="isLoading" class="flex items-center">
            <svg
              class="animate-spin -ml-1 mr-2 h-4 w-4 text-white"
              xmlns="http://www.w3.org/2000/svg"
              fill="none"
              viewBox="0 0 24 24"
            >
              <circle
                class="opacity-25"
                cx="12"
                cy="12"
                r="10"
                stroke="currentColor"
                stroke-width="4"
              ></circle>
              <path
                class="opacity-75"
                fill="currentColor"
                d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"
              ></path>
            </svg>
            Deleting...
          </span>
          <span v-else>Delete</span>
        </button>
      </div>
    </div>
  </div>
</template>

<style scoped>
/* Styling tambahan untuk modal jika diperlukan */
.scale-95.opacity-0 {
  transition: transform 0.3s ease-out, opacity 0.3s ease-out;
}
.scale-100.opacity-100 {
  transition: transform 0.3s ease-out, opacity 0.3s ease-out;
}
</style>
