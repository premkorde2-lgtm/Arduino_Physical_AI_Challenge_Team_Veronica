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


Repository Navigation & Directory Flow
Each directory contains a dedicated README.md explaining its specific implementation:

Getting Started Guide for New Developers
Follow these steps sequentially to bring up the full stack:

Step 1: Flashing Hardware
Wearable Watch: Follow the custom PCB flashing guide in firmware/watch/README.md to upload watch_main.ino via an external UART bridge.

Arduino UNO Q MCU: Flash uno_q_main.ino to the MCU layer following instructions in firmware/uno_q/README.md.

Step 2: Environment Configuration
Set up SSH access and VNC remote desktop on the Arduino UNO Q Debian layer using the steps detailed in scripts/README.md.

Step 3: Launching Video & Telemetry Servers
SSH into the UNO Q Linux environment and execute the video stream receiver:

Bash
python3 server/server.py
(See server/README.md for detailed dependencies).

Launch the Node.js web dashboard:

Bash
cd web
npm install
npm start
(See web/README.md for full endpoint configurations).

Step 4: Verify Deployment
Open your browser and navigate to http://localhost:3000 to inspect live video feeds, telemetry graphs, and emergency watch alerts.

System Gallery & Proofs
Visual documentation, hardware schematics, and terminal logs are available in the docs/images/ gallery.
