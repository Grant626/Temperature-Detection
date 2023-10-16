# seekcamera-fsc

The seekcamera-fsc application stores Flat Scene Correction (FSC) data to Seek cameras using the Seek SDK.

## Building

Please refer to the SDK C Programming Guide for details.

## Usage

To run the application, cal the application from the command-line.

```txt
$ seekcamera-fsc -h
Allowed options:
    -m : Discovery mode. Valid options: usb, spi, all (default: usb)
       : Required - No
    -t : Number of seconds to wait for the camera to warm up before storing the flat scene correction (default: 60)
       : Required - No
    -h : Displays this message
       : Required - No
```

### Expected output

Output from a successful run on a Raspberry Pi 4 is shown below.

```
$ root@raspberrypi4-64:~# seekcamera-fsc -m usb
seekcamera-fsc starting
settings
        1) mode (-m): usb
        2) warmup time (-t): 60
discovering cameras
camera connect: DE0D2DF10B19
flat scene correction action (valid options: delete, store, quit): store
confirm CID (DE0D2DF10B19): DE0D2DF10B19
starting capture session
warming up.. 0 seconds remaining
storing flat scene correction
flat scene correction store 100 % complete
stopping capture session
exiting...
camera disconnect: DE0D2DF10B19
done
```

### Mode (-m)

The discovery mode argument is optional; is it specified via the `-m` flag. It should
either be "usb", "spi", or "all". The default value is "usb". Discovery mode refers to the
protocol interface used to automatically discover the connected Seek devices.

Example usage:

```txt
# For both spi and usb
$ seekcamera-fsc -m all

# For spi only
$ seekcamera-fsc -m spi

# For usb only
$ seekcamera-fsc
$ seekcamera-fsc -m usb
```

### Warmup time (-t)

The warmup time argument is optional; it is specified via the `-t` flag.
It should be strictly greater than 0 and is specified in integer seconds.
The default value is 60 seconds.
Warmup time refers to the amount of time to allow the camera to warmup before storing a flat scene correction.

Example usage:

```txt
# Default 60 second warmup
$ seekcamera-fsc

# Custom 120 second warmup
$ seekcamera-fsc -t 120
```

### Help (-h)

The help argument is optional; it is specified via the `-h` flag.

Example usage:

```txt
$ seekcamera-fsc -h
```

## User controls

### delete
Deletes the FSC image from the camera. If no image was present in the camera, no action is taken.

### store
Perform the FSC process and store the FSC image into the camera.

### quit
Quits the application
