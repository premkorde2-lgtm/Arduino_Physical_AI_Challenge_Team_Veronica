#!/usr/bin/env python3
"""
VERONICA Q UDP Video Receiver
Team Veronica — Arduino Physical AI Challenge

Receives JPEG-encoded UDP video streams over port 5001,
decodes the binary payload using OpenCV, and displays the feed.
"""

import cv2
import numpy as np
import signal
import socket
import sys

# Network Configuration
UDP_IP = "0.0.0.0"
UDP_PORT = 5001
BUFFER_SIZE = 65535
SOCKET_BUF_SIZE = 4 * 1024 * 1024  # 4MB OS receive buffer

def setup_socket() -> socket.socket:
    """Initialize and configure the UDP listening socket."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, SOCKET_BUF_SIZE)
    sock.bind((UDP_IP, UDP_PORT))
    return sock

def main():
    sock = setup_socket()

    print("==========================================")
    print("      VERONICA Q UDP VIDEO RECEIVER       ")
    print("==========================================")
    print(f"[*] Listening Interface : {UDP_IP}")
    print(f"[*] UDP Stream Port   : {UDP_PORT}")
    print("[*] Waiting for incoming frames...")
    print("==========================================\n")

    def signal_handler(sig, frame):
        print("\n[!] Shutting down server gracefully...")
        sock.close()
        cv2.destroyAllWindows()
        sys.exit(0)

    signal.signal(signal.SIGINT, signal_handler)

    while True:
        try:
            # Receive UDP packet payload
            data, addr = sock.recvfrom(BUFFER_SIZE)

            # Decode buffer -> uint8 array -> BGR OpenCV Frame
            np_data = np.frombuffer(data, dtype=np.uint8)
            frame = cv2.imdecode(np_data, cv2.IMREAD_COLOR)

            if frame is None:
                print("[!] Warning: Received corrupted or invalid JPEG payload.")
                continue

            h, w, c = frame.shape
            print(f"[+] Frame Received: {w}x{h} ({c} channels) from {addr[0]}:{addr[1]}")

            # Display feed over Xorg / VNC session
            cv2.imshow("VERONICA - Q UDP FEED", frame)

            # Exit stream on 'q' press
            if cv2.waitKey(1) & 0xFF == ord("q"):
                print("[*] User quit stream requested.")
                break

        except Exception as err:
            print(f"[!] Frame processing exception: {err}")

    sock.close()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
