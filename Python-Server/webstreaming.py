# import the necessary packages
from thermal_cam.thermal_detection import *
from imutils.video import VideoStream
from flask import Flask, Response, send_from_directory, render_template, request
import threading
import argparse
import datetime
import imutils
import time
import cv2
import os 
import json

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

@app.route("/favicon.ico")
def favicon():
	return send_from_directory(os.path.join(app.root_path, 'static'),'favicon.ico',mimetype='image/vnd.microsof.icon')

@app.route("/")
def index():
    # return the rendered template
    return render_template("index.html")

@app.route("/nodes")
def nodes():
    return render_template("nodes.html")

renderer = Renderer()

def detect_temperature(framerate=30):
    
    time.sleep(5)

    global outputFrame, lock

    # Create a context structure responsible for managing all connected USB cameras.
    # Cameras with other IO types can be managed by using a bitwise or of the
    # SeekCameraIOType enum cases.
    with SeekCameraManager(SeekCameraIOType.USB) as manager:
      # Start listening for events.
      manager.register_event_callback(on_event, renderer)

      while True:
          # Wait a maximum of 150ms for each frame to be received.
          # A condition variable is used to synchronize the access to the renderer;
          # it will be notified by the user defined frame available callback thread.
          with renderer.frame_condition:
            # testing limit of framerate, ~15 framerate here
            # if renderer.frame_condition.wait_for(time.sleep(frame_wait)):
                if renderer.frame_condition.wait(150.0 / 1000.0):
                  img = renderer.frame.data
                  (height, width, _) = img.shape
                  img = imutils.resize(img, width=400)

                  # Resize the rendering window.
                  if renderer.first_frame:
                      renderer.first_frame = False
                      
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
        
def generateFrame(node):
    global outputFrame, lock
    with lock:
        # check if the output frame is available, otherwise skip
        # the iteration of the loop
        if outputFrame is None:
            print("Couldn't get output frame")
            return "Error getting output frame"

        cv2.imwrite('./static/node_%s.jpg' % node, outputFrame)
        
        # img.save('./static/node_%s.jpg' % node)
        return "Saving image for node_%s" % node
  
               
### Routes for thermal data

# json RESTful api get/post requests
@app.route('/node_temp', methods = ['GET', 'POST'])
def node_temp():
    node = request.args.get('node')
    data = json.load(open('./static/node_info.json'))
    if request.method == 'GET':
        return data['node%s' % node]['temp']
    if request.method == 'POST':
        data['node%s' % node]['temp'] = getMinMax(renderer)

@app.route('/node_status', methods = ['GET', 'POST'])
def node_status():
    node = request.args.get('node')
    data = json.load(open('./static/node_info.json'))
    if request.method == 'GET':
        return data['node%s' % node]['status']
    if request.method == 'POST':
        # Method to check status of node based on new frame temp with desired threshold
        data['node%s' % node]['status'] = 'Clear'

@app.route('/node_checked', methods = ['GET', 'POST'])
def node_checked():
    node = request.args.get('node')
    data = json.load(open('./static/node_info.json'))
    if request.method == 'GET':
        return data['node%s' % node]['checked']
    if request.method == 'POST':
        now = datetime.datetime.now()
        data['node%s' % node]['checked'] = "%02d.%02d.%04d / %02d:%02d" % (now.day, now.month, now.year, now.hour, now.minute)


# routes for saving frames
@app.route('/save_frame', methods = ['POST'])
def save_frame():
    node = request.args.get('node')
    # Push current time, frame temp and status to json
    return generateFrame(node)


# routes for handling the live feed
@app.route("/video_feed")
def video_feed():
    # return the response generated along with the specific media
    # type (mime type)
    return Response(generate(), mimetype="multipart/x-mixed-replace; boundary=frame")

@app.route("/live_temp")
def live_temp():
    # return the highest temp of current frame
    return getMinMax(renderer)

@app.route("/live_checking")
def live_checking():
    # return which node the robot is checking 
    # Possibly store global variable and change upon node update
    node = 1
    return "Node: {}".format(str(node))

@app.route("/live_status")
def live_status():
    # return current status of robot
    # Possibly check if any node is not functioning properly
    return "All clear"

@app.route("/live_time")
def live_time():
    # return current time
    now = datetime.datetime.now()
    return "%02d.%02d.%04d / %02d:%02d" % (now.day, now.month, now.year, now.hour, now.minute)


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