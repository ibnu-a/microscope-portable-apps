// src/firebase/config.js
import { initializeApp } from "firebase/app";
import { getDatabase } from "firebase/database";

const firebaseConfig = {
  apiKey: "AIzaSyBPioVPNcm68am6nWOW2bo3g0u9xywC_lQ",
  authDomain: "microscope-apps.firebaseapp.com",
  databaseURL: "https://microscope-apps-default-rtdb.firebaseio.com",
  projectId: "microscope-apps",
  storageBucket: "microscope-apps.firebasestorage.app",
  messagingSenderId: "591517764346",
  appId: "1:591517764346:web:f89b037fe0b7b8cd75d743",
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);

// Initialize Realtime Database and get a reference to the service
export const database = getDatabase(app);

export default app;
