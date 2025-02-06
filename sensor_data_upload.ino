#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <HardwareSerial.h>
#include <DHT.h>


//variables define 
///////////////////////////////////////////////////////////////////////////////////////////
#define DHTPIN 4       // Pin where the DHT11 is connected (e.g., GPIO4)
#define DHTTYPE DHT11  // DHT11 sensor type
DHT dht(DHTPIN, DHTTYPE);
#define FLAME_SENSOR_PIN 36 // Connect the flame sensor to analog pin A0
#define PIR_SENSOR_PIN 4
HardwareSerial gpsSerial(1);
const int bufferSize = 100;
char buffer[bufferSize];
int bufferIndex = 0;

long duration;
float distanceCm;
#include "time.h"

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Nabeel"
#define WIFI_PASSWORD "koyiloth"

// Insert Firebase project API Key
#define API_KEY "AIzaSyB0COdOmk52pzrnlsh5z2NWI9sApXpDqsQ"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "psych.121.matrix@gmail.com"
#define USER_PASSWORD "comsoc2025"

// Insert RTDB URL
#define DATABASE_URL "https://basictest-c7678-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String buildingPath = "/building_type";
String disasterPath = "disaster_type";
String latPath = "latitude";
String longPath = "longitude";
String motionPath = "motion_detected";
//String smokePath = "sensor_smoke";
String flamePath = "flame";
String tempPath = "sensor_temp";
String timepassedPath = "time_since_disaster";
String timePath = "/timestamp";

time_t disasterStartTime = 0;

//latitude and longitude
double lat = 0;
double longi = 0;

// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
FirebaseJson json;

const char* ntpServer = "pool.ntp.org";

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 10000;


//all methods used
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int calculateTimeSinceDisaster() {
  if (disasterStartTime == 0) {
    return 0; // No disaster has started
  }

  time_t currentTime = getTime();
  int elapsedSeconds = difftime(currentTime, disasterStartTime); // Elapsed time in seconds
  int elapsedHours = elapsedSeconds ; // Convert seconds to hours

  return elapsedHours;
}

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  int retryCount = 0;

  // Attempt to get time up to 5 times
  while (!getLocalTime(&timeinfo) && retryCount < 5) {
    Serial.println("Failed to obtain time, retrying...");
    retryCount++;
    delay(1000);
  }

  if (retryCount >= 5) {
    Serial.println("Failed to obtain time after retries");
    return 0;
  }

  time(&now); // Get the current time
  return now; // Return the timestamp
}

// Function to format a timestamp into "1stJan25_12:34:46" style
String formatTimestamp(time_t timestamp) {
  struct tm *timeinfo;
  timeinfo = localtime(&timestamp);

  char buffer[20]; // Buffer to hold the formatted string

  // Format the timestamp without the ordinal suffix
  strftime(buffer, sizeof(buffer), "%d%b%y_%H:%M:%S", timeinfo);

  // Convert the formatted string to a String object
  String formattedTimestamp = String(buffer);

  // Extract the day part (first 2 characters)
  String dayPart = formattedTimestamp.substring(0, 2);

  // Add the ordinal suffix to the day (e.g., 1st, 2nd, 3rd, 4th)
  int day = timeinfo->tm_mday;
  String suffix;
  if (day == 1 || day == 21 || day == 31) {
    suffix = "st";
  } else if (day == 2 || day == 22) {
    suffix = "nd";
  } else if (day == 3 || day == 23) {
    suffix = "rd";
  } else {
    suffix = "th";
  }

  // Replace the day part with the day + suffix
  formattedTimestamp.replace(dayPart, dayPart + suffix);

  return formattedTimestamp;
}

void processNMEASentence(const char* sentence) {
  // Check if it's a GGA sentence
  if (strncmp(sentence, "$GPGGA", 6) == 0) {


    // Split the sentence by commas
    char copy[100]; // Make a copy of the sentence for processing
    strncpy(copy, sentence, sizeof(copy));

    char* token = strtok(copy, ","); // Split by commas
    int fieldIndex = 0;
    double latitude = 0.0, longitude = 0.0;
    char latDirection = 'N', lonDirection = 'E';

    while (token != nullptr) {
      // Extract latitude (field 2) and longitude (field 4)
      if (fieldIndex == 2) {
        latitude = atof(token); // Convert to double
      } else if (fieldIndex == 3) {
        latDirection = token[0]; // N or S
      } else if (fieldIndex == 4) {
        longitude = atof(token); // Convert to double
      } else if (fieldIndex == 5) {
        lonDirection = token[0]; // E or W
      }

      token = strtok(nullptr, ","); // Move to the next field
      fieldIndex++;
    }

    // Adjust latitude and longitude based on direction
    if (latDirection == 'S') {
      latitude = -latitude;
    }
    if (lonDirection == 'W') {
      longitude = -longitude;
    }

    lat = latitude;
    longi = longitude;

    // Print latitude and longitude


  } 
}


////////////////////////////main//////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  // Initialize BME280 sensor
  initWiFi();
  configTime(19800, 0, "time.google.com");
  
  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
  disasterStartTime = getTime();
  // Start GPS module (adjust baud rate and pins as needed)
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);

  // Initialize DHT11 sensor
  dht.begin();

  // Flame sensor setup (optional, if needed)
  pinMode(FLAME_SENSOR_PIN, INPUT);
}

void loop() {
  while (gpsSerial.available()) {
    char c = gpsSerial.read(); // Read a single character

    // Check for the end of an NMEA sentence (marked by '\n')
    if (c == '\n') {
      buffer[bufferIndex] = '\0'; // Null-terminate the string
      processNMEASentence(buffer); // Process the complete sentence
      bufferIndex = 0; // Reset the buffer index
    } else if (bufferIndex < bufferSize - 1) {
      buffer[bufferIndex++] = c; // Add the character to the buffer
    }
  }

  float temperature = dht.readTemperature();
  int flameValue = analogRead(FLAME_SENSOR_PIN);
  int flame=flameValue/1023;

  int motion;
  int motionDetected = digitalRead(PIR_SENSOR_PIN); // Read the PIR sensor's digital output
  if (motionDetected == HIGH) { // Check if motion is detected
    motion=1;; // Print "1" if motion is detected
  } else {
    motion=0; // Print "0" if no motion is detected
  }
  
  // Send new readings to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // Get current timestamp
    timestamp = getTime();
    String formattedTime = formatTimestamp(timestamp); // Format the timestamp
    Serial.print("time: ");
    Serial.println(formattedTime);

    parentPath = databasePath + "/" + formattedTime;
    int timeSinceDisaster = calculateTimeSinceDisaster();

    json.set(buildingPath.c_str(), "2");
    json.set(disasterPath.c_str(), "3");
    json.set(latPath.c_str(), String(lat));
    json.set(longPath.c_str(), String(longi));
    //json.set(distancePath.c_str(), String(distanceCm));
    json.set(motionPath.c_str(), String(motion));
    json.set(tempPath.c_str(), String(temperature));
    json.set(flamePath.c_str(), String(flame));
    json.set(timepassedPath.c_str(), String(timeSinceDisaster));
    json.set(timePath, formattedTime);
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
  delay(1000);
}