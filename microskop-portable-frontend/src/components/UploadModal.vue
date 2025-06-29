<script setup>
import { ref } from "vue";
import { createClient } from "@supabase/supabase-js";

// Konfigurasi Supabase
const supabaseUrl = import.meta.env.VITE_SUPABASE_URL;
const supabaseKey = import.meta.env.VITE_SUPABASE_ANON_KEY;
const supabase = createClient(supabaseUrl, supabaseKey);

const staticEmail = import.meta.env.VITE_SUPABASE_STATIC_EMAIL;
const staticPassword = import.meta.env.VITE_SUPABASE_STATIC_PASSWORD;

// Fungsi login otomatis menggunakan email & password dari .env
const loginWithStaticCredentials = async () => {
  const { data, error } = await supabase.auth.signInWithPassword({
    email: staticEmail,
    password: staticPassword,
  });

  if (error) {
    console.error("Gagal login Supabase:", error.message);
    throw new Error("Login gagal, cek kredensial di .env");
  }

  return data.user;
};

// Nama tabel untuk menyimpan metadata gambar
const IMAGE_METADATA_TABLE = "image_metadata";

const props = defineProps({
  isOpen: Boolean,
  isLoading: Boolean,
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

// Generate unique filename untuk mencegah overwrite file dengan nama yang sama
const generateUniqueFileName = (file) => {
  const timestamp = new Date().getTime();
  const randomString = Math.random().toString(36).substring(2, 10);
  const extension = file.name.split(".").pop();
  return `${timestamp}-${randomString}.${extension}`;
};

// Handle upload gambar langsung ke Supabase Storage
const handleFileChange = async (event) => {
  const files = event.target.files;
  if (!files.length) return;

  emit("upload-loading", true); // Set loading state di komponen induk
  uploadError.value = null; // Bersihkan error

  const file = files[0]; // Hanya ambil file pertama

  // Validasi ukuran file (max 10MB)
  const MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB dalam bytes
  if (file.size > MAX_FILE_SIZE) {
    uploadError.value = `File terlalu besar. Maksimal ukuran file adalah 10MB.`;
    emit("upload-loading", false);
    return;
  }

  // Validasi tipe file
  const allowedTypes = ["image/jpeg", "image/png", "image/jpg"];
  if (!allowedTypes.includes(file.type)) {
    uploadError.value = `Tipe file tidak didukung. Hanya PNG, JPG, dan JPEG yang diizinkan.`;
    emit("upload-loading", false);
    return;
  }

  // Generate nama file unik untuk mencegah konflik nama
  const uniqueFileName = generateUniqueFileName(file);
  const filePath = `public/${uniqueFileName}`;

  try {
    const dataUser = await loginWithStaticCredentials();

    // Upload file ke bucket 'images' di Supabase Storage
    const { data, error } = await supabase.storage
      .from("images")
      .upload(filePath, file, {
        cacheControl: "3600",
        upsert: false,
      });

    if (error) {
      console.error("Upload error:", error);
    }
    if (error) throw error;

    // Dapatkan URL publik dari file yang diunggah
    const { data: publicURLData } = supabase.storage
      .from("images")
      .getPublicUrl(filePath);

    const publicUrl = publicURLData.publicUrl;

    // Simpan metadata gambar ke tabel database
    const imageMetadata = {
      original_filename: file.name,
      storage_path: filePath,
      public_url: publicUrl,
      file_size: file.size,
      mime_type: file.type,
      created_at: new Date().toISOString(), // Timestamp saat upload
      user_id: dataUser?.id,
    };

    // Insert metadata ke tabel Supabase
    const { data: metadataData, error: metadataError } = await supabase
      .from(IMAGE_METADATA_TABLE)
      .insert([imageMetadata])
      .select();

    if (metadataError) {
      console.error(
        "Berhasil upload gambar tetapi gagal menyimpan metadata:",
        metadataError
      );
      // Tidak throw error karena upload gambar sudah berhasil
    }

    // Tambahkan id dari database jika insert metadata berhasil
    const uploadedImageData = {
      id: metadataData?.[0]?.id, // ID dari database jika berhasil disimpan
      url: publicUrl,
      path: filePath,
      filename: uniqueFileName,
      originalFilename: file.name,
      size: file.size,
      mimetype: file.type,
    };

    // Emit success event ke komponen induk, sertakan data gambar yang diupload
    emit("upload-success", uploadedImageData);

    // Bersihkan input file setelah sukses
    if (fileInput.value) {
      fileInput.value.value = "";
    }
  } catch (error) {
    console.error(
      "Gagal mengupload gambar ke Supabase:",
      error.message || error
    );
    uploadError.value = `Gagal mengupload "${file.name}". ${
      error.message || "Silakan coba lagi."
    }`;
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
