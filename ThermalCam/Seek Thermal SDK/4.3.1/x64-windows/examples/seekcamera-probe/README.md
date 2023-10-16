# seekcamera-probe

The seekcamera-probe application probes the system for connected cameras using the Seek SDK.
It supports multiple cameras.

## Building

Please refer to the SDK C Programming Guide for details.

## Usage

To run the application, call the application from the command-line.

```txt
$ seekcamera-probe -h
Allowed options:
	-m : Discovery mode. Valid options: usb, spi, all (default: usb)
	   : Required - No
	-d : Probe duration in seconds. Valid options: > 0 (default: 1)
	   : Required - No
	-h : Displays this message
	   : Required - No
```

### Expected output

Output from a successfull run is shown below.

```txt
$ ./seekcamera-probe -m usb
seekcamera-probe starting
settings:
        1) mode (-m):          usb
        2) duration (-d):      1
+-------------------------------------------------------------------------+
| Seek Thermal SDK: 4.0.0.0                                               |
+-------------------------------------------------------------------------+
| CID          | SN           | CPN      | FW          | IO               |
+-------------------------------------------------------------------------+
| DE0D2DF11A26 | 261AZ0PBVH87 | 3509     | 16.8.2.5    | USB              |
|              |              |          |             | BUS: 1           |
|              |              |          |             | PORTS: 1         |
+-------------------------------------------------------------------------+
```

### Mode (-m)

The discovery mode argument is optional; is it specified via the `-m` flag. It should
either be "usb", "spi", or "all". The default value is "all". Discovery mode refers to the
protocol interface used to automatically discover the connected Seek devices.

Example usage:

```txt
# For both spi and usb
$ seekcamera-probe -m all

# For spi only
$ seekcamera-probe -m spi

# For usb only
$ seekcamera-probe
$ seekcamera-probe -m usb
```

### Probe duration (-d)

The probe duration is optional; it is specified via the `-d` flag.
It should be strictly greater than zero (>0) and has full-seconds granularity.

Example usage:

```txt
# For the default probe duration
$ seekcamera-probe

# For a non-default probe duration of 5s
$ seekcamera-probe -d 5
```

### Help (-h)

The help argument is optional; it is specified via the `-h` flag.

Example usage:

```txt
$ seekcamera-probe -h
```
