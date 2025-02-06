from flask import Flask, render_template, jsonify
import firebase_admin
from firebase_admin import credentials, db
import json

# Initialize Flask app
app = Flask(__name__)

# Firebase setup
# Use your Firebase JSON key
cred = credentials.Certificate(
    "basictest-c7678-firebase-adminsdk-fbsvc-4e4e6cea52.json")
firebase_admin.initialize_app(cred, {
    "databaseURL": "https://basictest-c7678-default-rtdb.asia-southeast1.firebasedatabase.app/"
})

# Route to fetch survivor data from Firebase


@app.route("/api/survivors")
def get_survivor_data():
    ref = db.reference("/readings")
    data = ref.get()
    return jsonify(data if data else {})

# Route to render the frontend


@app.route("/")
def index():
    return render_template("index.html")


if __name__ == "__main__":
    app.run(debug=True)
