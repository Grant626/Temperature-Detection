# import the necessary packages
from thermal_cam.thermal_detection import *
from imutils.video import VideoStream
from flask import Response
from flask import Flask
from flask import render_template
import threading
import argparse
import datetime
import imutils
import time
import cv2

# initialize the output frame and a lock used to ensure thread-safe
# exchanges of the output frames (useful when multiple browsers/tabs
# are viewing the stream)
outputFrame = None
lock = threading.Lock()

# initialize a flask object
app = Flask(__name__)

# initialize the video stream and allow the camera sensor to
# warmup
# vs = VideoStream(usePiCamera=1).start()
vs = VideoStream(src=0).start()
time.sleep(2.0)


@app.route("/")
def index():
    # return the rendered template
    return render_template("index.html")


def detect_temperature(framerate):
  global outputFrame, lock
  
  frame_wait = 1 / framerate
  # Create a context structure responsible for managing all connected USB cameras.
  # Cameras with other IO types can be managed by using a bitwise or of the
  # SeekCameraIOType enum cases.
  with SeekCameraManager(SeekCameraIOType.USB) as manager:
      # Start listening for events.
      renderer = Renderer()
      manager.register_event_callback(on_event, renderer)

      while True:
          # Wait a maximum of 150ms for each frame to be received.
          # A condition variable is used to synchronize the access to the renderer;
          # it will be notified by the user defined frame available callback thread.
          with renderer.frame_condition:
              # testing limit of framerate, ~15 framerate here
            if renderer.frame_condition.wait_for(time.sleep(frame_wait)):
               if renderer.frame_condition.wait(150.0 / 1000.0):
                  img = renderer.frame.data
                  (height, width, _) = img.shape
                  img = imutils.resize(img, width=400)

                  # Resize the rendering window.
                  if renderer.first_frame:
                      renderer.first_frame = False

                  getMinMax(img, renderer)
                  
                  if renderer.max > 30:
                      img = cv2.putText(
                          img,
                          "Temp over 30C !!!",
                          (110, 110),
                          cv2.FONT_HERSHEY_PLAIN,
                          .8,
                          (255, 255,255),
                          1
                      )

                  # final frame to be sent is here
                  with lock:
                    outputFrame = img.copy()

def generate():
    # grab global references to the output frame and lock variables
    global outputFrame, lock
    
    # loop over frames from the output stream
    while True:
        # wait until the lock is acquired
        with lock:
            # check if the output frame is available, otherwise skip
            # the iteration of the loop
            if outputFrame is None:
                continue

            # encode the frame in JPEG format
            (flag, encodedImage) = cv2.imencode(".jpg", outputFrame)

            # ensure the frame was successfully encoded
            if not flag:
                continue

        # yield the output frame in the byte format
        yield (
            b"--frame\r\n"
            b"Content-Type: image/jpeg\r\n\r\n" + bytearray(encodedImage) + b"\r\n"
        )


@app.route("/video_feed")
def video_feed():
    # return the response generated along with the specific media
    # type (mime type)
    return Response(generate(), mimetype="multipart/x-mixed-replace; boundary=frame")


# check to see if this is the main thread of execution
if __name__ == "__main__":
    # construct the argument parser and parse command line arguments
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "-i", "--ip", type=str, required=True, help="ip address of the device"
    )
    ap.add_argument(
        "-o",
        "--port",
        type=int,
        required=True,
        help="ephemeral port number of the server (1024 to 65535)",
    )
    ap.add_argument(
        "-f",
        "--framerate",
        type=int,
        default=15,
        help="# framerate to stream at",
    )
    args = vars(ap.parse_args())

    # start a thread that will perform temperature detection
    t = threading.Thread(target=detect_temperature, args=(
    	args["framerate"],))
    t.daemon = True
    t.start()

    # start the flask app
    app.run(
        host=args["ip"],
        port=args["port"],
        debug=True,
        threaded=True,
        use_reloader=False,
    )
# release the video stream pointer
vs.stop()