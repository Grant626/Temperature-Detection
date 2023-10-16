import cv2
import cv2
from seekcamera import SeekCamera, SeekFrame

# Initialize the SeekCamera
camera = SeekCamera()


def on_frame(_camera, camera_frame, _renderer):
    """Async callback fired whenever a new frame is available.

    Parameters
    ----------
    _camera: SeekCamera
        Reference to the camera for which the new frame is available.
    camera_frame: SeekFrame
        Reference to the class encapsulating the new frame.
    _renderer: Renderer
        User-defined data passed to the callback.
    """
    temperature_data = camera_frame.data
    # You can now process the temperature data as needed.

    # For example, print the temperature of a specific pixel.
    x = 100  # Replace with the x-coordinate of the pixel you want to query
    y = 100  # Replace with the y-coordinate of the pixel you want to query
    temperature_at_pixel = temperature_data[y][x]
    print(f"Temperature at pixel ({x}, {y}): {temperature_at_pixel}°C")


# Register the frame callback
camera.register_frame_available_callback(on_frame)

# Start the capture session
camera.capture_session_start()

# Create an OpenCV window to display the thermal image
cv2.namedWindow("Thermal Image", cv2.WINDOW_NORMAL)

while True:
    key = cv2.waitKey(1)
    if key == ord("q"):
        break

cv2.destroyAllWindows()