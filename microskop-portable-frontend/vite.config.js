// vite.config.js
import { defineConfig } from "vite";
import vue from "@vitejs/plugin-vue";
import tailwindcss from "@tailwindcss/vite"; // Pastikan plugin ini terinstal dengan benar
import { VitePWA } from "vite-plugin-pwa";

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [
    vue(),
    tailwindcss(), // Pastikan @tailwindcss/vite sudah diinstal
    VitePWA({
      registerType: "autoUpdate", // Otomatis memperbarui Service Worker saat ada update

      // devOptions harus dinonaktifkan di produksi.
      // Ini hanya digunakan untuk pengembangan (misalnya, menampilkan pesan debug PWA di konsol).
      devOptions: {
        enabled: false, // <-- PENTING: Setel ke false untuk produksi!
      },

      injectRegister: "auto", // Otomatis menyuntikkan kode pendaftaran Service Worker

      // Konfigurasi Workbox untuk caching aset
      workbox: {
        // globPatterns: Pola untuk aset yang akan dicache oleh Service Worker
        globPatterns: ["**/*.{js,css,html,ico,png,svg,woff,woff2,json}"], // Tambahkan '.json' jika Anda memiliki file JSON statis

        // runtimeCaching (Opsional tapi Direkomendasikan):
        // Strategi caching untuk aset yang tidak di-precache (misalnya, gambar dari Cloudinary, respons API backend).
        // Ganti 'YOUR_BACKEND_PUBLIC_URL' dengan URL backend Anda yang sebenarnya.
        runtimeCaching: [
          {
            urlPattern: ({ url }) =>
              url.origin === "https://res.cloudinary.com",
            handler: "CacheFirst", // Strategi: Utamakan cache. Jika tidak ada di cache, ambil dari jaringan.
            options: {
              cacheName: "cloudinary-images-cache",
              expiration: {
                maxEntries: 50, // Maksimal 50 gambar di cache
                maxAgeSeconds: 60 * 60 * 24 * 30, // Cache selama 30 hari
              },
              cacheableResponse: {
                statuses: [0, 200], // Cache respons OK (200) dan opaque (0 untuk cross-origin)
              },
            },
          },
          {
            // Contoh caching untuk endpoint API gambar dari backend Anda
            // Ganti 'YOUR_BACKEND_PUBLIC_URL' dengan domain backend Anda yang sudah di-deploy
            urlPattern: ({ url }) =>
              url.origin === "YOUR_BACKEND_PUBLIC_URL" &&
              url.pathname.startsWith("/api/images"),
            handler: "StaleWhileRevalidate", // Strategi: Sajikan dari cache, lalu perbarui cache di background.
            options: {
              cacheName: "api-images-cache",
              expiration: {
                maxEntries: 10, // Maksimal 10 respons API di cache
                maxAgeSeconds: 60 * 5, // Cache selama 5 menit
              },
              cacheableResponse: {
                statuses: [0, 200],
              },
            },
          },
        ],
      },

      // Konfigurasi Manifest PWA
      manifest: {
        name: "My Microscope App", // Nama lengkap aplikasi
        short_name: "MicroscopeApp", // Nama pendek untuk ikon di layar beranda
        description: "A simple microscope image management application.", // Deskripsi aplikasi
        theme_color: "#ffffff", // Warna tema browser/status bar
        background_color: "#ffffff", // Warna latar belakang aplikasi saat dimuat
        display: "standalone", // Cara aplikasi ditampilkan (seperti aplikasi native)
        icons: [
          {
            src: "pwa-192x192.png",
            sizes: "192x192",
            type: "image/png",
          },
          {
            src: "pwa-512x512.png",
            sizes: "512x512",
            type: "image/png",
            purpose: "any maskable", // Untuk ikon adaptif di Android
          },
          // Anda dapat menambahkan ukuran ikon lain jika diperlukan untuk kompatibilitas yang lebih luas
          // {
          //   src: 'pwa-512x512-maskable.png',
          //   sizes: '512x512',
          //   type: 'image/png',
          //   purpose: 'maskable',
          // },
          // {
          //   src: 'pwa-144x144.png',
          //   sizes: '144x144',
          //   type: 'image/png',
          // },
        ],
      },
    }),
  ],

  // Opsi Build untuk Produksi
  build: {
    // sourcemap: Mengontrol pembuatan source map.
    // Untuk keamanan (mencegah kode sumber asli mudah dilihat di browser)
    // dan ukuran file yang lebih kecil, disarankan untuk menonaktifkannya.
    // 'false': Tidak menghasilkan source map sama sekali.
    // 'hidden': Menghasilkan source map tetapi tidak menautkannya di file output (berguna untuk alat pelaporan error).
    sourcemap: false, // <-- PENTING: Direkomendasikan 'false' atau 'hidden' untuk produksi

    // Vite secara default sudah mengoptimalkan build untuk produksi (minifikasi, tree-shaking).
    // Anda tidak perlu secara eksplisit mengaktifkannya lagi di sini.
  },

  // Variabel Lingkungan
  // Vite secara otomatis memuat variabel dari .env dan mengekspos yang diawali 'VITE_'
  // ke kode frontend.
  // Pastikan Anda hanya menyimpan variabel non-sensitif di .env frontend.
  // Contoh: URL backend Anda.
  // Contoh penggunaan di kode frontend: import.meta.env.VITE_BACKEND_API_URL
});
