# Temperature-Detection
Temperature detection system for power substation detection robot.

## Installation
### Ensure that the Seek Thermal SDK is installed:

Download and install the SDK from here:

https://developer.thermal.com/support/solutions/articles/48001240854-seek-thermal-sdk-v4-3

Note:
- Must login with your Seek Thermal account
- Follow the PDF instructions to install based on your system architecture

### Ensure that all required python dependencies are installed:
<ul>
  <li>seekcamera-python</li>
  <li>flask</li>
  <li>opencv-python</li>
  <li>imutils</li>
</ul>

```
pip install seekcamera-python flask opencv-python imutils
```

Double check that all dependacy imports are satisfied in the server file.

## Deployment
Navigate to the python server directory:
```
cd Python-Server 
```
Run the server with the command:
```
python webstreaming.py --ip 0.0.0.0 --port 8000
```
This will open a server instance at 127.0.0.1:8000 on your local machine

This will also open an instance to be accessed on any machine connected to the local network, check console log for the ip

## Background

This is part of the system designed for a robot to patrol a power substation and detect faulty transformers based on excessively hot temperatures.

The robot is meant to function autonomously and patrol a pre-determined course, stopping at each transformer and maneuvering so that the camera faces it.

This system will then scan and detect the temperature of the transformer in that moment, streaming the live feed and also saving the frame to the correspnding transformer on the webpage.

The system will update with the status of each transformer so that substation workers and efficiently monitor them and increase response time to incidents involving faulty transformers, and also saving enormous time/resources that would typically be used to manually check the transformers.
