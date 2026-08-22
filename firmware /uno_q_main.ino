#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// --- Hardware Pins ---
RF24 radio(A, B);                   // USE ACCORDING TO YOUR CONNECTION IN OUR CASE THEY ARE D9,10
const int MODE_PIN = p;              // in our case its 2 , GND = Transmitter Mode | HIGH = Receiver Mode

const int xPin = A0;                 // ADXL335 Analog Pins
const int yPin = A1;
const int zPin = A2;

const byte address[6] = "00001";     // Shared communication pipe address

// --- Shared Data Structure ---
struct DataPacket {
  int xVal;
  int yVal;
  int zVal;
};

DataPacket packet;

// Mode State
bool isTransmitter = false;

// Non-blocking Timing Variables (Transmitter Mode)
unsigned long previousSampleTime = 0;
unsigned long previousTxTime = 0;
const unsigned long SAMPLE_INTERVAL = 10; // Sample ADXL335 every 10ms
const unsigned long TX_INTERVAL = 50;     // Transmit RF every 50ms

// Filtering Accumulators
long xAccumulator = 0;
long yAccumulator = 0;
long zAccumulator = 0;
int sampleCount = 0;

void setup() {
  Serial.begin(9600);

  // Configure Mode Switch Pin with Internal Pullup
  pinMode(MODE_PIN, INPUT_PULLUP);
  
  // Read Pin 2: LOW = Transmitter, HIGH = Receiver
  isTransmitter = (digitalRead(MODE_PIN) == LOW);

  // Initialize Radio Hardware
  if (!radio.begin()) {
    Serial.println(F("nRF24L01 Hardware Error! Check Wiring."));
    while (1);
  }

  radio.setPALevel(RF24_PA_LOW);

  if (isTransmitter) {
    radio.openWritingPipe(address);
    radio.stopListening();           // Set as Transmitter
    Serial.println(F("Mode: TRANSMITTER | Reading ADXL335 & Sending RF..."));
  } else {
    radio.openReadingPipe(0, address);
    radio.startListening();          // Set as Receiver
    Serial.println(F("Mode: RECEIVER | Listening for incoming RF streams..."));
  }
}

void loop() {
  if (isTransmitter) {
    runTransmitterMode();
  } else {
    runReceiverMode();
  }
}

// -------------------------------------------------------------
// TRANSMITTER FUNCTION (Non-Blocking Sensor Reading + TX)
// -------------------------------------------------------------
void runTransmitterMode() {
  unsigned long currentMillis = millis();

  // Task 1: Sample ADXL335
  if (currentMillis - previousSampleTime >= SAMPLE_INTERVAL) {
    previousSampleTime = currentMillis;
    xAccumulator += analogRead(xPin);
    yAccumulator += analogRead(yPin);
    zAccumulator += analogRead(zPin);
    sampleCount++;
  }

  // Task 2: Transmit RF Data
  if (currentMillis - previousTxTime >= TX_INTERVAL) {
    previousTxTime = currentMillis;

    if (sampleCount > 0) {
      packet.xVal = xAccumulator / sampleCount;
      packet.yVal = yAccumulator / sampleCount;
      packet.zVal = zAccumulator / sampleCount;

      xAccumulator = 0;
      yAccumulator = 0;
      zAccumulator = 0;
      sampleCount = 0;
    }

    bool success = radio.write(&packet, sizeof(DataPacket));

    if (success) {
      Serial.print(F("TX -> X: ")); Serial.print(packet.xVal);
      Serial.print(F(" | Y: "));   Serial.print(packet.yVal);
      Serial.print(F(" | Z: "));   Serial.println(packet.zVal);
    } else {
      Serial.println(F("TX Failed (No Ack)"));
    }
  }
}

// -------------------------------------------------------------
// RECEIVER FUNCTION (Non-Blocking Incoming RF Check)
// -------------------------------------------------------------
void runReceiverMode() {
  if (radio.available()) {
    radio.read(&packet, sizeof(DataPacket));

    Serial.print(F("RX -> X: ")); Serial.print(packet.xVal);
    Serial.print(F(" | Y: "));   Serial.print(packet.yVal);
    Serial.print(F(" | Z: "));   Serial.println(packet.zVal);
  }
}
