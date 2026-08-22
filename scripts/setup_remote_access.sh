#!/bin/bash
# ==============================================================================
# Arduino UNO Q — Remote Access & Auto-Start Configuration Script
# Team Veronica — Arduino Physical AI Challenge
# ==============================================================================

set -e

echo "=== [1/4] Updating System Packages ==="
sudo apt update && sudo apt upgrade -y

echo "=== [2/4] Installing SSH & VNC Dependencies ==="
sudo apt install -y openssh-server wayvnc tigervnc-standalone-server python3-pip python3-venv

echo "=== [3/4] Enabling Remote Services ==="
sudo systemctl enable ssh
sudo systemctl start ssh

mkdir -p ~/.vnc
echo "VNC server dependencies installed. Set password using: vncpasswd"

echo "=== [4/4] Creating Systemd Auto-Start Service for server.py ==="
SERVICE_FILE="/etc/systemd/system/unoq_server.service"

sudo bash -c "cat <<EOF > $SERVICE_FILE
[Unit]
Description=Arduino UNO Q Backend WebRTC Server
After=network.target

[Service]
Type=simple
User=$USER
WorkingDirectory=$(pwd)/../server
ExecStart=/usr/bin/python3 $(pwd)/../server/server.py
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF"

sudo systemctl daemon-reload
sudo systemctl enable unoq_server.service

echo "=== Setup Complete! ==="
echo "1. Verify SSH status: sudo systemctl status ssh"
echo "2. Run 'vncpasswd' to configure VNC access password."
echo "3. Start the backend service manually: sudo systemctl start unoq_server"# Setup script for SSH & VNC configuration
