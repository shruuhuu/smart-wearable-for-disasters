import firebase_admin
from firebase_admin import credentials, db
import joblib
import numpy as np
import time

# Load Firebase credentials
cred = credentials.Certificate(
    "basictest-c7678-firebase-adminsdk-fbsvc-4e4e6cea52.json")
firebase_admin.initialize_app(cred, {
    "databaseURL": "https://basictest-c7678-default-rtdb.asia-southeast1.firebasedatabase.app/"
})

# Load trained AI model
model = joblib.load("survivor_prediction_model.pkl")

# Function to fetch all readings from Firebase


def fetch_firebase_data():
    ref = db.reference("/test")  # Ensure this path matches Firebase
    data = ref.get()

    print(f"Raw Firebase Data: {data}")  # Debugging: Print fetched data

    if isinstance(data, dict):  # Ensure data is a valid dictionary
        print(f"Available entries: {data.keys()}")
        return data
    else:
        print("Invalid or No data found in Firebase!")
        return None

# Function to predict survivor probability for each row


def predict_survivor():
    data = fetch_firebase_data()

    if data is not None:
        for entry_id, entry_data in data.items():
            # Use .get() to avoid KeyError and provide default values
            X = np.array([[
                entry_data.get('latitude', 0.0),  # Default to 0 if missing
                entry_data.get('longitude', 0.0),
                entry_data.get('building_type', 1),  # Default = 1 (Concrete)
                entry_data.get('disaster_type', 1),  # Default = 1 (Earthquake)
                entry_data.get('time_since_disaster', 0),
                entry_data.get('sensor_temp', 25.0),  # Default = 25°C
                entry_data.get('sensor_smoke', 0),
                entry_data.get('motion_detected', 0)
            ]])

            # Ensure the data is numeric (Prevent crashes)
            X = np.nan_to_num(X)

            # AI Model Prediction
            prediction = model.predict(X)

            # Mapping AI Output
            survivor_labels = {0: "Low", 1: "Medium", 2: "High"}
            result = survivor_labels.get(prediction[0], "Unknown")

            print(f"Predicted Survivor Probability for {entry_id}: {result}")

            # ✅ Update each row with predicted probability
            db.reference(
                f"/test/{entry_id}").update({"predicted_probability": result})

    else:
        print("Skipping prediction due to missing data.")


# Run survivor prediction every 10 seconds
while True:
    predict_survivor()
    time.sleep(10)
