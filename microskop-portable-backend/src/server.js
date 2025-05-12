// server.js
require("dotenv").config({
  path: require("path").resolve(__dirname, "../.env"),
});
const express = require("express");
const http = require("http");
const WebSocket = require("ws");
const cors = require("cors");
const path = require("path");
const imageRoutes = require("./routes/imageRoutes");

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server }); // Buat server WebSocket

const PORT = process.env.PORT || 3000;

app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

// Serve static files
app.use(express.static(path.join(__dirname, "public")));

// Simpan semua koneksi WebSocket
const clients = new Set();

// Event listener untuk koneksi WebSocket
wss.on("connection", (ws) => {
  console.log("Klien terhubung (Frontend atau ESP32)");
  clients.add(ws);

  // Menangani data yang diterima
  ws.on("message", (data) => {
    try {
      const stringData = data.toString();
      const messageData = JSON.parse(stringData);
      console.log("Pesan diterima:", messageData);

      // Broadcast ke semua klien yang terhubung
      clients.forEach((client) => {
        if (client !== ws && client.readyState === WebSocket.OPEN) {
          client.send(stringData);
        }
      });
    } catch (e) {
      console.error("Gagal parse JSON:", data, e);
    }
  });

  // Tangani pemutusan koneksi
  ws.on("close", () => {
    console.log("Seorang klien terputus");
    clients.delete(ws);
  });
});

// API routes
app.use("/api/images", imageRoutes);

// Serve frontend
app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "public", "index.html"));
});

// Mulai server
server.listen(PORT, () => {
  console.log(`Server backend berjalan di port ${PORT}`);
  console.log(`Akses frontend di http://localhost:${PORT}`);
  console.log(`Pastikan ESP32 Anda terhubung ke ws://192.168.130.45:${PORT}/`);
});
