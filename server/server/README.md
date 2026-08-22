#  VERONICA Q UDP Video Stream Server

This directory contains `server.py`, an asynchronous UDP receiver that ingests high-framerate JPEG video frames from remote nodes (such as wearable cameras or edge modules) and displays them on the Arduino UNO Q desktop display buffer over VNC (`:0`).

---

##  Pipeline Architecture

```text
[ Remote Camera / Watch Node ]
              │
              │  UDP Datagrams (JPEG Bytes)
              ▼
    [ Port 5001 on UNO Q ]
              │
              ▼
        [ server.py ]
              │
              ├─► Socket Buffer Allocation (4MB)
              ├─► NumPy Array Conversion
              └─► OpenCV Frame Decoding
                      │
                      ▼
        [ VNC Desktop Window Display ]

## Execution Steps
1. Install System & Python Dependencies
Ensure OpenCV and NumPy are installed on the UNO Q Debian MPU layer:

Bash
sudo apt update
sudo apt install -y python3-opencv python3-numpy
2. Run the UDP Server
Start the receiver inside your SSH/VNC session:

Bash
python3 server/server.py
3. Key Configurations
Port: 5001 (UDP)

Buffer: 4MB (SO_RCVBUF) to eliminate frame drop during high-speed streaming.

Exit: Press q while focused on the OpenCV window or send Ctrl + C in the terminal.


4. Click **Commit changes...**.

---

Once committed, navigate to **`web/index.html`** or **`firmware/watch/watch_main.ino`** to continue building the repository!
