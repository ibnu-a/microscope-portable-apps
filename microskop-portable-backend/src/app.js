import express from "express";
import cors from "cors";
import rateLimit from "express-rate-limit";
import dotenv from "dotenv";

dotenv.config();

const app = express();

// app.set('trust proxy', 1);

app.use(cors());
app.use(express.json());

const API_KEY = process.env.API_KEY;

if (!API_KEY) {
  console.error(
    "CRITICAL ERROR: API_KEY is not defined in environment variables!"
  );
  process.exit(1);
}

const authenticateApiKey = (req, res, next) => {
  const providedApiKey = req.headers["x-api-key"]; // Ambil API Key dari header X-API-Key

  if (!providedApiKey || providedApiKey !== API_KEY) {
    console.warn(
      `Unauthorized access attempt from IP: ${req.ip} with key: ${providedApiKey}`
    );
    return res.status(403).json({ message: "Forbidden: Invalid API Key" });
  }
  next(); // Lanjutkan ke middleware/route berikutnya
};

const apiLimiter = rateLimit({
  windowMs: 1 * 60 * 1000,
  max: 600,
  message: {
    status: 429,
    message: "Too many requests from this IP, please try again after 1 minute.",
  },
  standardHeaders: true,
  legacyHeaders: false,
});

app.post("/sensor-data", authenticateApiKey, apiLimiter, (req, res) => {
  const {
    zoom_level,
    pan_position,
    tilt_position,
    zoom_percentage,
    pan_percentage,
    tilt_percentage,
  } = req.body;

  if (
    typeof zoom_level === "number" &&
    typeof pan_position === "number" &&
    typeof tilt_position === "number"
  ) {
    const io = req.app.get("socketio");

    io.emit("potentiometerUpdate", {
      zoomLevel: zoom_level,
      panPosition: pan_position,
      tiltPosition: tilt_position,
      zoomPercentage: zoom_percentage,
      panPercentage: pan_percentage,
      tiltPercentage: tilt_percentage,
    });

    res.status(200).json({
      message: "Potentiometer data received and broadcasted successfully",
      receivedData: req.body,
    });
  } else {
    res.status(400).json({
      message:
        "Invalid potentiometer data format. Check zoom, pan, and tilt levels.",
    });
  }
});

app.post("/image-action", authenticateApiKey, apiLimiter, (req, res) => {
  const { action } = req.body;

  if (action === "previous" || action === "next") {
    const io = req.app.get("socketio");

    io.emit("imageNavigation", { action });

    res.status(200).json({
      message: `Image navigation action '${action}' received and broadcasted.`,
      receivedAction: action,
    });
  } else {
    res.status(400).json({
      message:
        'Invalid image navigation action. Expected "previous" or "next".',
    });
  }
});

app.get("/", (req, res) => {
  res.status(200).send("Microscope Backend is running!");
});

export default app;
