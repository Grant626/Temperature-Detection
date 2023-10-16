from threading import Condition

import cv2
import seekcamera
from seekcamera import *


# SN: 0C10Z0YGWHB7
# CID: DED257D1100C

camera = seekcamera.SeekCamera()
print(camera.chipid)
frame = SeekFrame()


camera.capture_session_start(SeekCameraFrameFormat.COLOR_ARGB8888)
print(camera.serial_number)

