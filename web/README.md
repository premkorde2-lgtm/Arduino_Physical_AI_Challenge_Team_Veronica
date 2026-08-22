#  VERONICA Real-Time Web Dashboard

This directory contains the Node.js Express server, Socket.IO signaling layer, and client-side web interface for real-time video streaming, telemetry visualization, and system command interaction.

## System Architecture

```text
[ Arduino UNO Q / Python Server ]
               │
               │ WebSockets / Socket.IO
               ▼
      [ web/server.js (Node Express) ]
               │
               │ HTTP (Port 3000)
               ▼
   [ web/index.html + script.js ]
   (Live Telemetry & Feed Display)

📂 File Structure
server.js: Express HTTP server & Socket.IO signaling broker.

package.json: Node project metadata & dependency declarations (express, socket.io).

index.html: Responsive UI layout featuring video containers and telemetry display panels.

script.js: Frontend client logic handling WebSocket events, DOM rendering, and user controls.

🚀 Local Execution & Setup
1. Install Node Dependencies
Run the following command inside the web/ directory:

Bash
npm install
2. Start the Server
Launch the Express application:

Bash
npm start
3. Access the Interface
Open a web browser and navigate to:

Plaintext
http://localhost:3000
(Or http://Qip:3000 if connecting remotely across the local network).


5. Click the green **Commit changes...** button at the top right.

---

Now your `web/` directory is fully documented. Should we move on to drafting **`firmware/watch/watch_main.ino`** to complete the wearable unit code?
   
