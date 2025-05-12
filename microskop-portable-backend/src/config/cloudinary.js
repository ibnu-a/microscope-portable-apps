const cloudinary = require("cloudinary").v2;

// Mengonfigurasi Cloudinary dengan kredensial dari environment variables
cloudinary.config({
  cloud_name: process.env.CLOUDINARY_CLOUD_NAME,
  api_key: process.env.CLOUDINARY_API_KEY,
  api_secret: process.env.CLOUDINARY_API_SECRET,
  secure: true, // Menggunakan HTTPS untuk semua URL Cloudinary
});

// Mengekspor objek cloudinary yang sudah dikonfigurasi
module.exports = cloudinary;
