<script setup>
import { defineEmits } from "vue";

const emit = defineEmits(["image-click"]);

const props = defineProps({
  images: {
    type: Array,
    required: true,
  },
});

// Fungsi untuk menampilkan detail gambar
const showImageViewer = (image) => {
  emit("image-click", image);
};
</script>

<template>
  <div
    class="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap-6"
  >
    <div
      v-for="image in images"
      :key="image.id"
      class="bg-white rounded-md overflow-hidden shadow-sm hover:shadow-md transition-shadow duration-200 cursor-pointer"
      @click="showImageViewer(image)"
    >
      <div class="relative pb-[75%] overflow-hidden bg-gray-100">
        <img
          :src="image.src"
          :alt="image.name"
          class="absolute inset-0 w-full h-full object-cover hover:scale-105 transition-transform duration-300"
        />
        <div
          class="absolute inset-0 bg-black opacity-0 hover:opacity-10 transition-opacity duration-300"
        ></div>
      </div>
      <!-- <div class="p-4">
        <h3 class="font-medium text-gray-800 truncate">{{ image.name }}</h3>
        <p class="text-xs text-gray-500 mt-1">{{ image.date }}</p>
      </div> -->
    </div>
  </div>
</template>
