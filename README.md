# Golioth Modbus Reference Design

This repository contains the firmware source code and [pre-built release
firmware
images](https://github.com/golioth/reference-design-modbus-vibration-monitor/releases)
for the Golioth Modbus Vibration Monitor reference design.

The full project details are available on the [Modbus Vibration Monitor
Project
Page](https://projects.golioth.io/reference-designs/modbus-vibration-monitor),
including follow-along guides for building an IoT Modbus Vibration
Monitor yourself using widely available off-the-shelf development
boards.

We call this **Follow-Along Hardware**, and we think it's one of the
quickest and easiest ways to get started building an IoT
proof-of-concept with Golioth. In the follow-along guides, you will
learn how to assemble the hardware, flash a pre-built firmware image
onto the device, and connect to the Golioth cloud in minutes.

Once you have completed a follow-along guide for one of our supported
hardware platforms, the instructions below will walk you through how to
build and configure the firmware yourself.

## Supported Hardware

This firmware can be built for a variety of supported hardware
platforms.

> In Zephyr, each of these different hardware variants is given a unique
> "board" identifier, which is used by the build system to generate
> firmware for that variant.
>
> When building firmware using the instructions below, make sure to use
> the correct Zephyr board identifier that corresponds to your
> follow-along hardware platform.

| Hardware                                                     | Zephyr Board           | Follow-Along Guide                                                                                                       |
| ------------------------------------------------------------ | ---------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| ![image](images/modbus_vibration_monitor_fah_nrf9160_dk.jpg) | `nrf9160dk_nrf9160_ns` | [nRF9160 DK Follow-Along Guide](https://projects.golioth.io/reference-designs/modbus-vibration-monitor/guide-nrf9160-dk) |

**Follow-Along Hardware**

| Hardware                                                               | Zephyr Board                     | Project Page                                                                                                    |
| ---------------------------------------------------------------------- | -------------------------------- | --------------------------------------------------------------------------------------------------------------- |
| ![image](images/modbus_vibration_monitor_aludel_mini_v1_photo_top.jpg) | `aludel_mini_v1_sparkfun9160_ns` | [Modbus Vibration Monitor Project Page](https://projects.golioth.io/reference-designs/modbus-vibration-monitor) |
| ![image](images/modbus_vibration_monitor_aludel_mini_v1_photo_top.jpg) | `aludel_elixir_ns`               |                                                                                                                 |

**Custom Golioth Hardware**

## Firmware Overview

This reference design firmware demonstrates how to interface with a
[Banner Sure Cross®
QM30VT2](https://www.bannerengineering.com/us/en/products/part.806276.html)
sensor via the Modbus protocol and send vibration and temperature
measurements to the cloud using the Golioth IoT platform.

Sensor values are uploaded to the LightDB stream database in the Golioth
Cloud. The sensor sampling frequency and other sensor parameters are
remotely configurable via the Golioth Settings service.

### Supported Golioth Zephyr SDK Features

This firmware implements the following features from the Golioth Zephyr
SDK:

  - [Device Settings
    Service](https://docs.golioth.io/firmware/zephyr-device-sdk/device-settings-service)
  - [LightDB State
    Client](https://docs.golioth.io/firmware/zephyr-device-sdk/light-db/)
  - [LightDB Stream
    Client](https://docs.golioth.io/firmware/zephyr-device-sdk/light-db-stream/)
  - [Logging
    Client](https://docs.golioth.io/firmware/zephyr-device-sdk/logging/)
  - [Over-the-Air (OTA) Firmware
    Upgrade](https://docs.golioth.io/firmware/device-sdk/firmware-upgrade)
  - [Remote Procedure Call
    (RPC)](https://docs.golioth.io/firmware/zephyr-device-sdk/remote-procedure-call)

#### Device Settings Service

The following settings can be set in the Device Settings menu of the
[Golioth Console](https://console.golioth.io).

  - `LOOP_DELAY_S`
    Adjusts the delay between sensor readings. Set to an integer value
    (seconds).

    Default value is `60` seconds.

#### LightDB Stream Service

Sensor data is periodically sent to the following `sensor/*` endpoints
of the LightDB Stream service:

  - `sensor/temperature/celcius`: Temperature (°C)
  - `sensor/temperature/farenheight`: Temperature (°F)
  - `sensor/x_axis/acceleration/crest_factor`: X-Axis Crest Factor
  - `sensor/x_axis/acceleration/high_frequency_rms`: X-Axis
    High-Frequency RMS Acceleration (G)
  - `sensor/x_axis/acceleration/kurtosis`: X-Axis Kurtosis
  - `sensor/x_axis/acceleration/peak`: X-Axis Peak Acceleration (G)
  - `sensor/x_axis/acceleration/rms`: X-Axis RMS Acceleration (G)
  - `sensor/x_axis/velocity/peak/frequency`: X-Axis Peak Velocity
    Component Frequency (Hz)
  - `sensor/x_axis/velocity/peak/in_per_sec`: X-Axis Peak Velocity
    (in/sec)
  - `sensor/x_axis/velocity/peak/mm_per_sec`: X-Axis Peak Velocity
    (mm/sec)
  - `sensor/x_axis/velocity/rms/in_per_sec`: X-Axis RMS Velocity
    (in/sec)
  - `sensor/x_axis/velocity/rms/mm_per_sec`: X-Axis RMS Velocity
    (mm/sec)
  - `sensor/x_axis/acceleration/crest_factor`: X-Axis Crest Factor
  - `sensor/x_axis/acceleration/high_frequency_rms`: X-Axis
    High-Frequency RMS Acceleration (G)
  - `sensor/z_axis//acceleration/kurtosis`: X-Axis Kurtosis
  - `sensor/z_axis//acceleration/peak`: Z-Axis Peak Acceleration (G)
  - `sensor/z_axis//acceleration/rms`: Z-Axis RMS Acceleration (G)
  - `sensor/z_axis//velocity/peak/frequency`: Z-Axis Peak Velocity
    Component Frequency (Hz)
  - `sensor/z_axis//velocity/peak/in_per_sec`: Z-Axis Peak Velocity
    (in/sec)
  - `sensor/z_axis//velocity/peak/mm_per_sec`: Z-Axis Peak Velocity
    (mm/sec)
  - `sensor/z_axis//velocity/rms/in_per_sec`: Z-Axis RMS Velocity
    (in/sec)
  - `sensor/z_axis//velocity/rms/mm_per_sec`: Z-Axis RMS Velocity
    (mm/sec)

On hardware platforms with support for battery monitoring, battery
voltage and level readings are periodically sent to the following
`battery/*` endpoints:

  - `battery/batt_v`: Battery Voltage (V)
  - `battery/batt_lvl`: Battery Level (%)

#### LightDB State Service

The concept of Digital Twin is demonstrated with the LightDB State
`example_int0` and `example_int1` variables that are members of the
`desired` and `state` endpoints.

  - `desired` values may be changed from the cloud side. The device will
    recognize these, validate them for \[0..65535\] bounding, and then
    reset these endpoints to `-1`
  - `state` values will be updated by the device whenever a valid value
    is received from the `desired` endpoints. The cloud may read the
    `state` endpoints to determine device status, but only the device
    should ever write to the `state` endpoints.

#### Remote Procedure Call (RPC) Service

The following RPCs can be initiated in the Remote Procedure Call menu of
the [Golioth Console](https://console.golioth.io).

  - `get_network_info`
    Query and return network information.

  - `reboot`
    Reboot the system.

  - `set_log_level`
    Set the log level.

    The method takes a single parameter which can be one of the
    following integer values:

      - `0`: `LOG_LEVEL_NONE`
      - `1`: `LOG_LEVEL_ERR`
      - `2`: `LOG_LEVEL_WRN`
      - `3`: `LOG_LEVEL_INF`
      - `4`: `LOG_LEVEL_DBG`

## Building the firmware

The firmware build instructions below assume you have already set up a
Zephyr development environment and have some basic familiarity with
building firmware using the Zephyr Real Time Operating System (RTOS).

If you're brand new to building firmware with Zephyr, you will need to
follow the [Zephyr Getting Started
Guide](https://docs.zephyrproject.org/latest/develop/getting_started/)
to install the Zephyr SDK and related dependencies.

We also provide free online [Developer
Training](https://training.golioth.io) for Zephyr at:

<https://training.golioth.io/docs/zephyr-training>

> Do not clone this repo using git. Zephyr's `west` meta-tool should be
> used to set up your local workspace.

### Create a Python virtual environment (recommended)

``` shell
cd ~
mkdir golioth-reference-design-modbus-vibration-monitor
python -m venv golioth-reference-design-modbus-vibration-monitor/.venv
source golioth-reference-design-modbus-vibration-monitor/.venv/bin/activate
```

### Install `west` meta-tool

``` shell
pip install wheel west
```

### Use `west` to initialize the workspace and install dependencies

``` shell
cd ~/golioth-reference-design-modbus-vibration-monitor
west init -m git@github.com:golioth/reference-design-modbus-vibration-monitor.git .
west update
west zephyr-export
pip install -r deps/zephyr/scripts/requirements.txt
```

### Build the firmware

Build the Zephyr firmware from the top-level workspace of your project.
After a successful build you will see a new `build/` directory.

Note that this git repository was cloned into the `app` folder, so any
changes you make to the application itself should be committed inside
this repository. The `build` and `deps` directories in the root of the
workspace are managed outside of this git repository by the `west`
meta-tool.

Prior to building, update `CONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION` in the
`prj.conf` file to reflect the firmware version number you want to
assign to this build.

> When running the commands below, make sure to replace the placeholder
> `<your_zephyr_board_id>` with the actual Zephyr board from the table
> above that matches your follow-along hardware.

``` text
$ (.venv) west build -p -b <your_zephyr_board_id> app
```

For example, to build firmware for the [Nordic nRF9160
DK](https://www.nordicsemi.com/Products/Development-hardware/nrf9160-dk)-based
follow-along hardware:

``` text
$ (.venv) west build -p -b nrf9160dk_nrf9160_ns app
```

### Flash the firmware

``` text
$ (.venv) west flash
```

### Provision the device

In order for the device to securely authenticate with the Golioth Cloud,
we need to provision the device with a pre-shared key (PSK). This key
will persist across reboots and only needs to be set once after the
device firmware has been programmed. In addition, flashing new firmware
images with `west flash` should not erase these stored settings unless
the entire device flash is erased.

Configure the PSK-ID and PSK using the device UART shell and reboot the
device:

``` text
uart:~$ settings set golioth/psk-id <my-psk-id@my-project>
uart:~$ settings set golioth/psk <my-psk>
uart:~$ kernel reboot cold
```

## External Libraries

The following code libraries are installed by default. If you are not
using the custom hardware to which they apply, you can safely remove
these repositories from `west.yml` and remove the includes/function
calls from the C code.

  - [golioth-zephyr-boards](https://github.com/golioth/golioth-zephyr-boards)
    includes the board definitions for the Golioth Aludel-Mini
  - [libostentus](https://github.com/golioth/libostentus) is a helper
    library for controlling the Ostentus ePaper faceplate
  - [zephyr-network-info](https://github.com/golioth/zephyr-network-info)
    is a helper library for querying, formatting, and returning network
    connection information via Zephyr log or Golioth RPC

## Pulling in updates from the Reference Design Template

This reference design was forked from the [Reference Design
Template](https://github.com/golioth/reference-design-template) repo. We
recommend the following workflow to pull in future changes:

  - Setup
      - Create a `template` remote based on the Reference Design
        Template repository
  - Merge in template changes
      - Fetch template changes and tags
      - Merge template release tag into your `main` (or other branch)
      - Resolve merge conflicts (if any) and commit to your repository

<!-- end list -->

``` shell
# Setup
git remote add template https://github.com/golioth/reference-design-template.git
git fetch template --tags

# Merge in template changes
git fetch template --tags
git checkout your_local_branch
git merge template_v1.0.0

# Resolve merge conflicts if necessary
git add resolved_files
git commit
```
