# Arduino_Physical_AI_Challenge_Team_Veronica
Autonomous physical AI system developed for the Arduino Physical AI Challenge by Team Veronica. Features embedded edge AI processing, real-time sensor integration, and motor control.

![Project Veronica Banner](<docs/images/Project Veronica.jpeg>)

##  System Overview

Team Veronica's Physical AI ecosystem integrates a custom wearable ESP32 watch, an Arduino UNO Q dual-domain bridge (MCU + Linux MPU), an asynchronous OpenCV video server, and a real-time Node.js dashboard into a unified physical intelligence stack.

```text
  +-----------------------+              +-----------------------+
  |  ESP32 Wearable Watch |              |    Arduino UNO Q      |
  |  - MPU-6050 Motion    |  Wi-Fi / UDP |  - MCU Real-Time Core |
  |  - GPS NMEA Stream    | -----------> |  - Debian Linux MPU   |
  |  - BLE Air Mouse      |  Telemetry   |  - OpenCV Server      |
  +-----------------------+              +-----------┬-----------+
                                                     │
                                     Socket.IO / Web │ Telemetry & Video
                                                     ▼
                                         +-----------------------+
                                         | Dynamic Web Dashboard |
                                         |  - Express / Node.js  |
                                         |  - WebRTC Video Stream|
                                         +-----------------------+

