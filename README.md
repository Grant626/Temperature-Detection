# Temperature-Detection
Temperature detection system for power substation detection robot.

# Installation
Ensure that the Seek Thermal SDK and all required python dependencies are installed:

-seekcamera-python
-flask
-opencv-python
-imutils

# Deployment
Run the server with the command:

python webstreaming.py --ip 0.0.0.0 --port 8000

This will open a server instance at 127.0.0.1:8000 on your local machine

This will also open an instance to be accessed on any machine connected to the local network, check console log for the ip
