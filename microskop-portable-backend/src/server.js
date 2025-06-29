import http from "http";
import { Server as SocketIOServer } from "socket.io";
import app from "./app.js";
import dotenv from "dotenv";

dotenv.config(); // Load environment variables from .env file

const PORT = process.env.PORT || 3000;
const server = http.createServer(app); // Create HTTP server from Express app
const io = new SocketIOServer(server, {
  cors: {
    origin: "*", // Allow all origins for development. Restrict this in production.
    methods: ["GET", "POST"],
  },
});

// Socket.IO event handling
io.on("connection", (socket) => {
  console.log("A client connected:", socket.id);

  socket.on("disconnect", () => {
    console.log("Client disconnected:", socket.id);
  });
});

// Make `io` instance accessible in `app.js`
app.set("socketio", io);

// Start the server
server.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
  console.log(`Socket.IO server running on port ${PORT}`);
});

export default io; // Export io if needed elsewhere, though usually it's passed via app.set
