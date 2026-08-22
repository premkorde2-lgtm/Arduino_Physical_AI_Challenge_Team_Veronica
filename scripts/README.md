## Arduino UNO Q Remote Setup Guide (SSH & VNC)

This guide provides a zero-to-connected walkthrough for configuring remote terminal access (SSH) and desktop interface access (VNC) on the Arduino UNO Q Debian environment.

> **Note:** Replace `Qip` in the commands below with your actual Arduino UNO Q IP address (e.g., `192.168.1.50`).

---

##  System Connection Architecture

```text
LAPTOP (Windows)
  │
  ├─► SSH Command ───────────────────────┐
  │                                      │
  │                                      ▼
  │                                Arduino UNO Q
  │                                      │
  │                                      ├─► Xorg :0 Display Engine
  │                                      │
  │                                      └─► x11vnc Service
  │                                            │
  │                                            │ TCP Port 5900
  ▼                                            ▼
RealVNC Viewer ◄───────────────────────── Wi-Fi / Hotspot Network
  │
  └─► Live Linux Desktop

🛠 Step-by-Step Setup
1. Connect Laptop → UNO Q via SSH
Open Windows Command Prompt (CMD) and execute:

DOS
ssh arduino@Qip
Enter your UNO Q password when prompted. Successful connection output:

Plaintext
arduino@prem:~$
2. Verify Desktop Display Engine
Check that the graphical desktop server is active on the UNO Q:

Bash
ps aux | grep -E 'Xorg|lightdm' | grep -v grep
Expected Output:

Plaintext
Xorg :0
3. Launch x11vnc Server
Execute the server startup command using the LightDM Xauthority file:

Bash
sudo x11vnc -display :0 -auth /var/run/lightdm/root/:0 -forever -shared -rfbport 5900
Important: Keep this SSH terminal session open. Closing it will terminate the VNC service.

Expected Startup Logs:

Plaintext
Using X display :0
Listening for VNC connections on TCP port 5900
4. Connect via RealVNC Viewer
Launch RealVNC Viewer on your laptop.

Enter the address:

Plaintext
Qip:5900
Click Connect.

## VNC Password Configuration & Troubleshooting
Option A: Configure Password Authentication
If RealVNC prompts for authentication, stop x11vnc in your terminal (Ctrl + C) and generate a password:

Bash
x11vnc -storepasswd
Enter a password (8 characters max). Relaunch the VNC server using:

Bash
sudo x11vnc -display :0 -auth /var/run/lightdm/root/:0 -rfbauth /home/arduino/.vnc/passwd -forever -shared -rfbport 5900
