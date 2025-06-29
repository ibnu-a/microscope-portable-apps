import express from "express";
import cors from "cors";

const app = express();

// Middleware
app.use(cors()); // Enable CORS for all routes
app.use(express.json()); // To parse JSON bodies from incoming requests

// Route for ESP32 to send potentiometer data
app.post("/sensor-data", (req, res) => {
  const {
    zoom_level,
    pan_position,
    tilt_position, // PROPERTI BARU
    zoom_percentage,
    pan_percentage,
    tilt_percentage, // PROPERTI BARU
  } = req.body;

  // Basic validation
  if (
    typeof zoom_level === "number" &&
    typeof pan_position === "number" &&
    typeof tilt_position === "number" // Validasi properti baru
  ) {
    const io = req.app.get("socketio");

    io.emit("potentiometerUpdate", {
      zoomLevel: zoom_level,
      panPosition: pan_position,
      tiltPosition: tilt_position, // Teruskan properti baru ke frontend
      zoomPercentage: zoom_percentage,
      panPercentage: pan_percentage,
      tiltPercentage: tilt_percentage, // Teruskan properti baru ke frontend
    });

    res
      .status(200)
      .json({
        message: "Potentiometer data received and broadcasted successfully",
        receivedData: req.body,
      });
  } else {
    res
      .status(400)
      .json({
        message:
          "Invalid potentiometer data format. Check zoom, pan, and tilt levels.",
      });
  }
});

// ROUTE BARU UNTUK AKSI NAVIGASI GAMBAR DARI ENCODER
app.post("/image-action", (req, res) => {
  const { action } = req.body; // Diharapkan payload: { "action": "next" } atau { "action": "previous" }

  if (action === "previous" || action === "next") {
    const io = req.app.get("socketio");

    // Emit event baru ke frontend untuk aksi navigasi gambar
    io.emit("imageNavigation", { action }); // Event baru: 'imageNavigation'

    res
      .status(200)
      .json({
        message: `Image navigation action '${action}' received and broadcasted.`,
        receivedAction: action,
      });
  } else {
    res
      .status(400)
      .json({
        message:
          'Invalid image navigation action. Expected "previous" or "next".',
      });
  }
});

// Basic root route for testing
app.get("/", (req, res) => {
  res.status(200).send("Microscope Backend is running!");
});

export default app;
