# smart-wearable-for-disasters
# Survivor Prediction System - Firebase & AI Integration

## Overview
This project integrates ESP32 sensor data, Firebase Realtime Database, and an AI Model (Random Forest) to predict survivor probability in disaster scenarios. The system continuously fetches sensor data from Firebase, processes it through a trained ML model, and updates the database with the predicted probability.

## Project Structure
```md
📁 survivor_prediction_system
│── 📄 main.py                # Python script for Firebase & AI model integration
│── 📄 survivor_prediction_model.pkl  # Pre-trained AI model
│── 📄 basictest-c7678-firebase-adminsdk.json  # Firebase credentials
│── 📄 README.md              # Project documentation
```

## Hardware Components
- **ESP32** - Microcontroller for processing and wireless communication
- **Motion Sensor** - Detects movement to assess survivor activity
- **DHT11 Sensor** - Measures temperature and humidity
- **Smoke Sensor (MQ-2/MQ-135)** - Detects hazardous gases and smoke levels
- **GPS Module** - Provides real-time location tracking

## Software & ML Components
- **Firebase Realtime Database** - Stores survivor data and AI predictions
- **Python & Firebase Admin SDK** - Fetches sensor data and updates predictions
- **Machine Learning Model (Random Forest)** - Predicts survivor probability
- **NumPy & Joblib** - Handles data preprocessing and model execution

## Features
- Fetches real-time survivor data from Firebase  
- Uses AI (Random Forest) to predict survivor probability  
- Updates probability back into Firebase without overwriting data  
- Processes multiple survivor entries dynamically  
- Runs continuously and updates predictions every 10 seconds  

## Firebase Data Structure
The Firebase Realtime Database follows this structure:
```json
{
  "readings": {
    "entry1": {
      "latitude": 12.9716,
      "longitude": 77.5946,
      "building_type": 2,
      "disaster_type": 1,
      "time_since_disaster": 5,
      "sensor_temp": 36.5,
      "sensor_smoke": 150,
      "motion_detected": 1,
      "predicted_probability": 2
    }
  }
}
```

## Setup Instructions
### Install Dependencies
Ensure you have Python 3 installed, then run:
```bash
pip install firebase-admin joblib numpy
```

### Set Up Firebase
1. Go to [Firebase Console](https://console.firebase.google.com/)  
2. Create a project and enable Realtime Database.  
3. In Project Settings → Service Accounts, click Generate New Private Key.  
4. Download the JSON file and rename it to `basictest-c7678-firebase-adminsdk.json`.  

### Run the Survivor Prediction Script
```bash
python main.py
```

## Code Explanation
### Fetch Data from Firebase
The script retrieves all survivor records:
```python
ref = db.reference("/readings")
data = ref.get()
```

### Predict Survivor Probability
The AI model processes each survivor's data:
```python
X = np.array([[data.get('latitude', 0.0), data.get('longitude', 0.0), data.get('sensor_temp', 25.0)]])
prediction = model.predict(X)
```

### Store Results in Firebase
Each entry is updated with `predicted_probability`:
```python
db.reference(f"/readings/{entry_id}").update({"predicted_probability": result})
```

## Expected Output
When the script runs, it fetches real-time survivor data, predicts their survival probability, and updates Firebase. Example:
```bash
Raw Firebase Data: {"latitude": 12.9716, "longitude": 77.5946, "sensor_temp": 36.5}
Predicted Survivor Probability: 2
Updated Firebase entry: /readings/entry1
```

## Next Steps
- Integrate a front-end dashboard to display survivor predictions.  
- Enhance AI model with more training data for improved accuracy.  
- Optimize Firebase data structure for large-scale deployments.  

Developed for real-time disaster survivor tracking and rescue optimization.




