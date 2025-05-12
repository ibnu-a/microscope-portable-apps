// backend/src/routes/imageRoutes.js
const express = require("express");
const imageController = require("../controllers/imageController"); // Import controller
const multer = require("multer"); // Import multer
const path = require("path"); // Import path untuk menentukan folder upload temporer

const router = express.Router();

// --- Konfigurasi Multer ---
// Tujuan penyimpanan file sementara
const storage = multer.diskStorage({
  destination: (req, file, cb) => {
    // Pastikan folder 'uploads' ada atau buat jika belum
    const uploadDir = path.join(__dirname, "../../uploads");
    if (!fs.existsSync(uploadDir)) {
      fs.mkdirSync(uploadDir);
    }
    cb(null, uploadDir); // Simpan di folder 'backend/uploads'
  },
  filename: (req, file, cb) => {
    // Nama file unik (misalnya, timestamp + nama asli file)
    cb(null, Date.now() + "-" + file.originalname);
  },
});

// Filter jenis file yang diizinkan (hanya gambar)
const fileFilter = (req, file, cb) => {
  if (file.mimetype.startsWith("image/")) {
    cb(null, true);
  } else {
    cb(new Error("Only image files are allowed!"), false);
  }
};

// Inisialisasi Multer dengan konfigurasi
const upload = multer({
  storage: storage,
  fileFilter: fileFilter,
  limits: { fileSize: 10 * 1024 * 1024 }, // Batas ukuran file 10MB
});
const fs = require("fs"); // Perlu diimpor di sini juga untuk mkdirSync

// --- Definisi Routes API ---

// Route GET untuk mengambil daftar gambar (Sudah Ada)
// Path: /api/images
router.get("/", imageController.getImages);

// Route POST untuk mengupload gambar
// Path: /api/images/upload
// 'image' adalah nama field di FormData yang berisi file
router.post("/upload", upload.single("image"), imageController.uploadImage);

// Mengekspor router
module.exports = router;
