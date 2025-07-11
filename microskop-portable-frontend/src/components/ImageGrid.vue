<script setup>
const emit = defineEmits(["image-click", "delete-image"]);

const props = defineProps({
  images: {
    type: Array,
    required: true,
  },
});

const showImageViewer = (image) => {
  emit("image-click", image);
};

const handleDeleteClick = (event, image) => {
  event.stopPropagation();
  emit("delete-image", image);
};
</script>

<template>
  <div
    class="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap-6"
  >
    <div
      v-for="image in images"
      :key="image.id"
      class="bg-white rounded-md overflow-hidden shadow-sm hover:shadow-md transition-shadow duration-200 cursor-pointer relative group"
      @click="showImageViewer(image)"
    >
      <div class="relative pb-[75%] overflow-hidden bg-gray-100">
        <img
          :src="image.public_url"
          :alt="image.original_filename"
          class="absolute inset-0 w-full h-full object-cover hover:scale-105 transition-transform duration-300"
        />
        <div
          class="absolute inset-0 bg-black opacity-0 group-hover:opacity-10 transition-opacity duration-300"
        ></div>

        <button
          @click="handleDeleteClick($event, image)"
          class="absolute top-2 right-2 p-1 bg-red-600 text-white rounded-full opacity-100 transition-opacity duration-300 transform scale-90 group-hover:scale-100 focus:outline-none focus:ring-2 focus:ring-red-500 focus:ring-opacity-50 cursor-pointer"
          title="Delete Image"
        >
          <svg
            xmlns="http://www.w3.org/2000/svg"
            class="h-5 w-5"
            viewBox="0 0 20 20"
            fill="currentColor"
          >
            <path
              fill-rule="evenodd"
              d="M9 2a1 1 0 00-.894.553L7.382 4H4a1 1 0 000 2v10a2 2 0 002 2h8a2 2 0 002-2V6a1 1 0 100-2h-3.382l-.724-1.447A1 1 0 0011 2H9zM7 8a1 1 0 011-1h4a1 1 0 110 2H8a1 1 0 01-1-1zm1 3a1 1 0 100 2h4a1 1 0 100-2H8z"
              clip-rule="evenodd"
            />
          </svg>
        </button>
      </div>
      <div class="p-4">
        <h3 class="font-medium text-gray-800 truncate">
          {{ image.original_filename }}
        </h3>
      </div>
    </div>
  </div>
</template>
