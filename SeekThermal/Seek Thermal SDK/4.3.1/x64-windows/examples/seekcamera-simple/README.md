# seekcamera-simple

The seekcamera-simple application discovers Seek cameras and logs thermography frames to CSV.

## Building

Please refer to the SDK C Programming Guide for details.

## Usage

To run the application, run the application from the command-line.

```txt
$ ./seekcamera-simple -h
Allowed options
        -m : Discovery mode. Valid options: usb, spi, all (default: usb)
           : Required - No
        -h : Displays this message
           : Required - No
```

## Expected output

Output from a successful run is shown below.

```txt
root@raspberrypi4-64:~# seekcamera-simple starting
settings
        1) mode (-m): usb
camera connect: DE0D2DF11A26
registered camera callback: DE0D2DF11A26
started capture session: DE0D2DF11A26
opened log file: DE0D2DF11A26 (thermography-DE0D2DF11A26.csv)
frame available: DE0D2DF11A26 (size: 200x150)
```

When a camera connects, a capture session will immediately start; each received frame will be logged to a
CSV file in the current directory. The CSV filename is specific to the camera and has the form
`thermography-[CID].csv" where CID refers to the camera's unique chip ID (CID).

### Mode (-m)

The discovery mode argument is optional; is it specified via the `-m` flag. It should
either be "usb", "spi", or "all". The default value is "usb". Discovery mode refers to the
protocol interface used to automatically discover the connected Seek devices.

Example usage:

```txt
# For both spi and usb
$ seekcamera-simple -m all

# For spi only
$ seekcamera-simple -m spi

# For usb only
$ seekcamera-simple
$ seekcamera-simple -m usb
```

### Help (-h)

The help argument is optional; it is specified via the `-h` flag.

Example usage:

```txt
$ seekcamera-simple -h
```
