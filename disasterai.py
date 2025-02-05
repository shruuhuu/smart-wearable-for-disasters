import time
from firebase_admin import credentials, db
import firebase_admin
import pandas as pd
import numpy as np
import random
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score
import joblib

df = pd.read_csv("lastminutedata.csv")

# Features & Labels
X = df[['latitude', 'longitude', 'building_type', 'disaster_type', 'time_since_disaster',
        'sensor_temp', 'sensor_smoke', 'motion_detected']]
y = df['survivor_zone']  # Now categorical (0,1,2)

# Train-test split (80% training, 20% testing)
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42)

# Train classification model
model = RandomForestClassifier(n_estimators=100, random_state=42)
model.fit(X_train, y_train)

# Model Evaluation
y_pred = model.predict(X_test)
accuracy = accuracy_score(y_test, y_pred)
print(f"Model Accuracy: {accuracy * 100:.3f}%")

# Save model for real-time use
joblib.dump(model, "survivor_prediction_model.pkl")
print("Model Trained and Saved Successfully!")
