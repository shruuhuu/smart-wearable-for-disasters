import React, { useEffect, useState } from "react";
import { MapContainer, TileLayer, Marker, Popup } from "react-leaflet";
import "leaflet/dist/leaflet.css";
import { initializeApp } from "firebase/app";
import { getDatabase, ref, onValue } from "firebase/database";
import { Card, CardContent } from "@/components/ui/card";
import { Badge } from "@/components/ui/badge";

// Firebase Config (Replace with your credentials)
const firebaseConfig = {
  apiKey: "AIzaSyB0COdOmk52pzrnlsh5z2NWI9sApXpDqsQ",
  authDomain: "basictest-c7678.firebaseapp.com",
  databaseURL:
    "https://basictest-c7678-default-rtdb.asia-southeast1.firebasedatabase.app",
  projectId: "basictest-c7678",
  storageBucket: "basictest-c7678.firebasestorage.app",
  messagingSenderId: "404211896599",
  appId: "1:404211896599:web:ffc19cda6fb7a8bb072752",
};

const app = initializeApp(firebaseConfig);
const database = getDatabase(app);

export default function SurvivorDashboard() {
  const [survivorData, setSurvivorData] = useState({});

  useEffect(() => {
    const dbRef = ref(database, "/test");
    const unsubscribe = onValue(dbRef, (snapshot) => {
      setSurvivorData(snapshot.val() || {});
    });
    return () => unsubscribe();
  }, []);

  return (
    <div className="p-4">
      <h1 className="text-2xl font-bold mb-4">Survivor Prediction Dashboard</h1>
      <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
        {Object.entries(survivorData).map(([key, data]) => (
          <Card key={key}>
            <CardContent>
              <h2 className="text-xl font-semibold">Survivor ID: {key}</h2>
              <p>Latitude: {data.latitude}</p>
              <p>Longitude: {data.longitude}</p>
              <p>Building Type: {data.building_type}</p>
              <p>Disaster Type: {data.disaster_type}</p>
              <p>Time Since Disaster: {data.time_since_disaster} hours</p>
              <p>Temperature: {data.sensor_temp}°C</p>
              <p>Smoke Level: {data.sensor_smoke}</p>
              <p>Motion Detected: {data.motion_detected ? "Yes" : "No"}</p>
              <Badge
                className={
                  data.predicted_probability === "High"
                    ? "bg-red-500"
                    : data.predicted_probability === "Medium"
                    ? "bg-yellow-500"
                    : "bg-green-500"
                }
              >
                {data.predicted_probability || "Unknown"}
              </Badge>
            </CardContent>
          </Card>
        ))}
      </div>

      <div className="mt-4">
        <h2 className="text-xl font-semibold">Survivor Locations</h2>
        <MapContainer
          center={[0, 0]}
          zoom={2}
          style={{ height: "400px", width: "100%" }}
        >
          <TileLayer url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png" />
          {Object.entries(survivorData).map(
            ([key, data]) =>
              data.latitude &&
              data.longitude && (
                <Marker key={key} position={[data.latitude, data.longitude]}>
                  <Popup>Survivor ID: {key}</Popup>
                </Marker>
              )
          )}
        </MapContainer>
      </div>
    </div>
  );
}
