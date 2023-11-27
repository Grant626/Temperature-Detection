# seekcamera-fw-update

The seekcamera-fw-update application updates the firmware on Seek cameras using the Seek SDK.

## Building

Please refer to the SDK C Programming Guide for details.

## Usage

To run the application, call the application from the command-line.

```txt
$ seekcamera-fw-update -h
Allowed options:
    -m : Discovery mode. Valid options: usb, spi, all (default: usb)
       : Required - No
    -p : Path to the file containing the firmware image
       : Required - Yes
    -h : Displays this message
       : Required - No
```

### Expected output

Output from a successfull run is shown below.

```txt
root@raspberrypi4-64:~# seekcamera-fw-update -m usb -p [FW_IMAGE.bin]
seekcamera-fw-update starting
settings
        1) mode (-m): usb
        2) path (-p): [FW_IMAGE.bin]
camera connect: DE0D2DF11A26
update firmware (y/n): y
confirm CID to overwrite current firmware (DE0D2DF11A26): DE0D2DF11A26
firmware update 100 percent complete: DE0D2DF11A26
firmware update finished successfully: DE0D2DF11A26
exiting...
camera disconnect: DE0D2DF11A26
done
```

### Mode (-m)

The discovery mode argument is optional; is it specified via the `-m` flag.
It should either be "usb", "spi", or "all".
The default value is "usb". Discovery mode refers to the protocol interface used to automatically discover the connected Seek devices.

Example usage:

```txt
# For both spi and usb
$ seekcamera-fw-update -p [FW_IMAGE.bin] -m all

# For spi only
$ seekcamera-fw-update -p [FW_IMAGE.bin] -m spi

# For usb only
$ seekcamera-fw-update -p [FW_IMAGE.bin]
$ seekcamera-fw-update -p [FW_IMAGE.bin] -m usb
```

### Path (-p)

The path argument is required; it is specified via the `-p` flag.
It should be an absolute path to the file containing the firmware image.

Example usage:

```txt
$ seekcamera-fw-update -p [FW_IMAGE.bin]
```

### Help (-h)

The help argument is optional; it is specified via the `-h` flag.

Example usage:

```txt
$ seekcamera-fw-update -h
```
