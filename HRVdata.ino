#include <math.h>


const int sensorPin = A0;                  // Analog pin connected to the sensor
const int threshold = 450;                 // Threshold value for beat detection
int sensorValue = 0;                          // Variable to store sensor readings
unsigned long lastBeatTime = 0;      // Time of the last detected beat
unsigned long currentTime = 0;       // Current time in milliseconds
float BPM = 0;                                  // Beats Per Minute
int beatCount = 0;                            // Counter for beats to calculate an average BPM
unsigned long bpmWindowStart = 0; // Time window start for BPM averaging
const int averagingWindow = 3000; // Averaging window in milliseconds (3 seconds)

// HRV variables
unsigned long rrIntervals[50];   // Array to store R-R intervals (up to 50)
int rrIndex = 0;                 // Index for storing R-R intervals
float RMSSD = 0;                 // Root Mean Square of Successive Differences (HRV)

// Setup function
void setup() {
  Serial.begin(9600);            // Start serial communication at 9600 baud
}

// Main loop
void loop() {
  sensorValue = analogRead(sensorPin);  // Read the analog value from the sensor
  currentTime = millis();              // Get the current time in milliseconds

  // Detect a beat when the value exceeds the threshold
  if (sensorValue > threshold) {
    if (currentTime - lastBeatTime > 300) {  // Minimum 300 ms between beats to avoid double-counting
      unsigned long timeBetweenBeats = currentTime - lastBeatTime; // Time between beats in ms
      BPM = 60000.0 / timeBetweenBeats;    // Calculate BPM (60000 ms in a minute)
      lastBeatTime = currentTime;          // Update the last beat time

      // Store R-R interval
      if (rrIndex < 50) {  // Store up to 50 R-R intervals
        rrIntervals[rrIndex] = timeBetweenBeats;
        rrIndex++;
      } else {
        // Shift R-R intervals if we have more than 50
        for (int i = 0; i < 49; i++) {
          rrIntervals[i] = rrIntervals[i + 1];
        }
        rrIntervals[49] = timeBetweenBeats;  // Add the latest R-R interval
      }

      // Calculate RMSSD if there are enough R-R intervals
      if (rrIndex > 1) {
        float sumOfSquares = 0;
        int rrCount = rrIndex < 50 ? rrIndex : 50;  // Use up to 50 intervals

        // Calculate the squared differences between successive R-R intervals
        for (int i = 1; i < rrCount; i++) {
          float diff = rrIntervals[i] - rrIntervals[i - 1];
          sumOfSquares += diff * diff;
        }

        // Calculate RMSSD
        RMSSD = sqrt(sumOfSquares / (rrCount - 1));
      }

      // Add beat to count and manage the averaging window
      beatCount++;
    }
  }

  // Print BPM and RMSSD every second
  if (currentTime - bpmWindowStart >= averagingWindow) {
    if (beatCount > 0) {
      Serial.print("BPM: ");
      Serial.println(BPM);  // Print the calculated BPM
      Serial.print("HRV (RMSSD): ");
      Serial.println(RMSSD);  // Print the calculated HRV (RMSSD)
    } else {
      Serial.println("No beats detected!");
    }
    bpmWindowStart = currentTime;  // Reset the averaging window
    beatCount = 0;                 // Reset the beat counter
  }
}
