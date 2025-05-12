const cloudinary = require("../config/cloudinary"); // Import objek cloudinary yang sudah dikonfigurasi
const fs = require("fs");

// Fungsi controller untuk mengambil daftar gambar
const getImages = async (req, res, next) => {
  try {
    // Mengambil resource (gambar) dari Cloudinary
    const result = await cloudinary.api.resources({
      type: "upload", // Mengambil resource yang diunggah
      prefix: "mikroskop-portable-images/", // Filter berdasarkan folder di Cloudinary
      max_results: 10, // Jumlah maksimum hasil yang akan dikembalikan
      resource_type: "image", // Pastikan hanya mengambil gambar
      tags: true, // Sertakan tag yang terkait dengan gambar
      context: true, // Sertakan metadata konteks jika ada
      direction: "desc", // Urutkan dari yang terbaru
    });

    // Memformat data agar lebih mudah dikonsumsi oleh frontend
    const images = result.resources.map((resource) => ({
      id: resource.public_id,
      src: resource.secure_url,
      name:
        resource.filename || resource.public_id.split("/").pop().split(".")[0], // Coba ambil nama asli, fallback ke public_id
      date: new Date(resource.uploaded_at).toLocaleDateString("id-ID", {
        year: "numeric",
        month: "long",
        day: "numeric",
      }),
      tags: resource.tags || [],
      width: resource.width,
      height: resource.height,
      format: resource.format,
      // Anda bisa menambahkan properti lain dari objek `resource` jika diperlukan
    }));

    // Mengirim daftar gambar dalam format JSON ke frontend
    res.status(200).json(images);
  } catch (error) {
    console.error("Error in getImages controller:", error.message || error);
    // Meneruskan error ke middleware penanganan error global
    next(error);
  }
};

const uploadImage = async (req, res, next) => {
  try {
    // Multer menyimpan file yang diupload di req.file
    if (!req.file) {
      const error = new Error("No file uploaded.");
      error.statusCode = 400;
      throw error;
    }

    // Path sementara tempat Multer menyimpan file
    const filePath = req.file.path;

    // Upload file ke Cloudinary
    const result = await cloudinary.uploader.upload(filePath, {
      folder: "mikroskop-portable-images", // Folder tujuan di Cloudinary
      // Anda bisa menambahkan opsi lain seperti eager transformations, tags, dll.
    });

    // Hapus file sementara setelah berhasil diupload ke Cloudinary
    fs.unlinkSync(filePath);

    // Mengirim detail gambar yang diupload kembali ke frontend
    res.status(201).json({
      message: "Image uploaded successfully!",
      image: {
        id: result.public_id,
        src: result.secure_url,
        name: result.original_filename || req.file.originalname.split(".")[0],
        date: result.uploaded_at, // Tanggal upload dari Cloudinary
        width: result.width,
        height: result.height,
        format: result.format,
        tags: result.tags || [],
      },
    });
  } catch (error) {
    console.error("Error during image upload:", error.message || error);
    // Hapus file sementara jika terjadi error saat upload ke Cloudinary
    if (req.file && fs.existsSync(req.file.path)) {
      fs.unlinkSync(req.file.path);
    }
    next(error); // Meneruskan error ke middleware penanganan error global
  }
};

// Mengekspor fungsi controller
module.exports = {
  getImages,
  uploadImage,
};
