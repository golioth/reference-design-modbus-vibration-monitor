# Golioth Modbus Reference Design

The Modbus Reference Design demonstrates how to interface with a
[Banner Sure Cross®
QM30VT2](https://www.bannerengineering.com/us/en/products/part.806276.html)
sensor via the Modbus protocol and send vibration and temperature
measurements to the cloud using the Golioth IoT platform.

Sensor values are uploaded to the cloud and stored in Golioth's LightDB
Stream database. The sensor sampling frequency and other sensor
parameters are remotely configurable via the Golioth Settings service.

Business use cases and hardware build details are available on [the
Modbus Vibration Monitor Project
Page](https://projects.golioth.io/reference-designs/modbus-vibration-monitor/).

This repository contains the firmware source code and [pre-built release
firmware
images](https://github.com/golioth/reference-design-modbus-vibration-monitor/releases).

## Supported Hardware

- Nordic nRF9160-DK
- Golioth Aludel Elixir
- Golioth Aludel Mini

### Additional Sensors/Components

- Texas Instruments THVD1426 RS-485 transceiver
- Banner QM30VT2 Banner Modbus temperature & vibration sensor

## Golioth Features

This app implements:

  - [Device Settings
    Service](https://docs.golioth.io/firmware/golioth-firmware-sdk/device-settings-service)
  - [Remote Procedure Call
    (RPC)](https://docs.golioth.io/firmware/golioth-firmware-sdk/remote-procedure-call)
  - [Stream
    Client](https://docs.golioth.io/firmware/golioth-firmware-sdk/stream-client)
  - [LightDB State
    Client](https://docs.golioth.io/firmware/golioth-firmware-sdk/light-db-state/)
  - [Over-the-Air (OTA) Firmware
    Upgrade](https://docs.golioth.io/firmware/golioth-firmware-sdk/firmware-upgrade/firmware-upgrade)
  - [Backend
    Logging](https://docs.golioth.io/device-management/logging/)

### Settings Service

The following settings should be set in [the Device Settings menu of the
Golioth Console](https://console.golioth.io/device-settings).

  - `LOOP_DELAY_S`
    Adjusts the delay between sensor readings. Set to an integer value
    (seconds).

    Default value is `60` seconds.

### Remote Procedure Call (RPC) Service

The following RPCs can be initiated in the Remote Procedure Call tab of
each device in the [Golioth Console](https://console.golioth.io).

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

### Time-Series Stream data

Sensor data is periodically sent to the following `sensor/*` paths of
the LightDB Stream service:

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
  - `sensor/z_axis/acceleration/kurtosis`: X-Axis Kurtosis
  - `sensor/z_axis/acceleration/peak`: Z-Axis Peak Acceleration (G)
  - `sensor/z_axis/acceleration/rms`: Z-Axis RMS Acceleration (G)
  - `sensor/z_axis/velocity/peak/frequency`: Z-Axis Peak Velocity
    Component Frequency (Hz)
  - `sensor/z_axis/velocity/peak/in_per_sec`: Z-Axis Peak Velocity
    (in/sec)
  - `sensor/z_axis/velocity/peak/mm_per_sec`: Z-Axis Peak Velocity
    (mm/sec)
  - `sensor/z_axis/velocity/rms/in_per_sec`: Z-Axis RMS Velocity
    (in/sec)
  - `sensor/z_axis/velocity/rms/mm_per_sec`: Z-Axis RMS Velocity
    (mm/sec)

``` json
{
  "sensor": {
    "temperature": {
      "celcius": 21.96,
      "farenheight": 71.53
    },
    "x_axis": {
      "acceleration": {
        "crest_factor": 12.258,
        "high_frequency_rms": 0.007,
        "kurtosis": 25.063,
        "peak": 0.09,
        "rms": 0.007
      },
      "velocity": {
        "peak": {
          "frequency": 12.2,
          "mm_per_sec": 0.19
        },
        "rms": {
          "mm_per_sec": 0.134
        }
      }
    },
    "z_axis": {
      "acceleration": {
        "crest_factor": 3.986,
        "high_frequency_rms": 0.019,
        "kurtosis": 3.231,
        "peak": 0.078,
        "rms": 0.248
      },
      "velocity": {
        "peak": {
          "frequency": 9.7,
          "mm_per_sec": 25.453
        },
        "rms": {
          "mm_per_sec": 17.998
        }
      }
    }
  }
}
```

If your board includes a battery, voltage and level readings
will be sent to the `battery` path.

> [!NOTE]
> Your Golioth project must have a Pipeline enabled to receive this
> data. See the [Add Pipeline to Golioth](#add-pipeline-to-golioth)
> section below.

### Stateful Data (LightDB State)

The concept of Digital Twin is demonstrated with the LightDB State
`example_int0` and `example_int1` variables that are subpaths of the
`desired` and `state` paths.

  - `desired` values may be changed from the cloud side. The device will
    recognize these, validate them for \[0..65535\] bounding, and then
    reset these values to `-1`
  - `state` values will be updated by the device to reflect the device's
    actual stored value. The cloud may read the `state` endpoints to
    determine device status. In this arrangement, only the device
    should ever write to the `state` endpoints.

``` json
{
  "desired": {
    "example_int0": -1,
    "example_int1": -1
  },
  "state": {
    "example_int0": 0,
    "example_int1": 1
  }
}
```

By default the state values will be `0` and `1`. Try updating the
`desired` values and observe how the device updates its state.

### OTA Firmware Update

This application includes the ability to perform Over-the-Air (OTA)
firmware updates. To do so, you need a binary compiled with a different
version number than what is currently running on the device.

> [!NOTE]
> If a newer release is available than what your device is currently
> running, you may download the pre-compiled binary that ends in
> `_update.bin` and use it in step 2 below.

1. Update the version number in the `VERSION` file and perform a
   pristine (important) build to incorporate the version change.
2. Upload the `build/app/zephyr/zephyr.signed.bin` file as a Package for
   your Golioth project.

   - Use `main` as the package name.
   - Use the same version number from step 1.

3. Create a Cohort and add your device to it.
4. Create a Deployment for your Cohort using the package name and
   version number from step 2.
5. Devices in your Cohort will automatically upgrade to the most
   recently deployed firmware.

Visit [the Golioth Docs OTA Firmware Upgrade
page](https://docs.golioth.io/firmware/golioth-firmware-sdk/firmware-upgrade/firmware-upgrade)
for more info.

## Add Pipeline to Golioth

Golioth uses [Pipelines](https://docs.golioth.io/data-routing) to route
stream data. This gives you flexibility to change your data routing
without requiring updated device firmware.

Whenever sending stream data, you must enable a pipeline in your Golioth
project to configure how that data is handled. Add the contents of
`pipelines/json-to-lightdb.yml` as a new pipeline as follows (note that
this is the default pipeline for new projects and may already be
present):

1.  Navigate to your project on the Golioth web console.
2.  Select `Pipelines` from the left sidebar and click the `Create`
    button.
3.  Give your new pipeline a name and paste the pipeline configuration
    into the editor.
4.  Click the toggle in the bottom right to enable the pipeline and
    then click `Create`.

All data streamed to Golioth in JSON format will now be routed to
LightDB Stream and may be viewed using the web console. You may change
this behavior at any time without updating firmware simply by editing
this pipeline entry.

## Local set up

> [!IMPORTANT]
> Do not clone this repo using git. Zephyr's `west` meta tool should be
> used to set up your local workspace.

### Install the Python virtual environment (recommended)

``` shell
cd ~
mkdir golioth-reference-design-modbus-vibration-monitor
python -m venv golioth-reference-design-modbus-vibration-monitor/.venv
source golioth-reference-design-modbus-vibration-monitor/.venv/bin/activate
pip install wheel west ecdsa
```

### Use `west` to initialize and install

``` shell
cd ~/golioth-reference-design-modbus-vibration-monitor
west init -m git@github.com:golioth/reference-design-modbus-vibration-monitor.git .
west update
west zephyr-export
pip install -r deps/zephyr/scripts/requirements.txt
```

## Building the application

Build the Zephyr sample application for the [Nordic nRF9160
DK](https://www.nordicsemi.com/Products/Development-hardware/nrf9160-dk)
(`nrf9160dk_nrf9160_ns`) from the top level of your project. After a
successful build you will see a new `build` directory. Note that any
changes (and git commits) to the project itself will be inside the `app`
folder. The `build` and `deps` directories being one level higher
prevents the repo from cataloging all of the changes to the dependencies
and the build (so no `.gitignore` is needed).

Prior to building, update `VERSION` file to reflect the firmware version
number you want to assign to this build. Then run the following commands
to build and program the firmware onto the device.

> [!WARNING]
> You must perform a pristine build (use `-p` or remove the `build`
> directory) after changing the firmware version number in the `VERSION`
> file for the change to take effect.

``` text
$ (.venv) west build -p -b nrf9160dk/nrf9160/ns --sysbuild app
$ (.venv) west flash
```

Configure PSK-ID and PSK using the device shell based on your Golioth
credentials and reboot:

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
    includes the board definitions for the Golioth Aludel-Elixir
  - [libostentus](https://github.com/golioth/libostentus) is a helper
    library for controlling the Ostentus ePaper faceplate
  - [zephyr-network-info](https://github.com/golioth/zephyr-network-info)
    is a helper library for querying, formatting, and returning network
    connection information via Zephyr log or Golioth RPC

## Have Questions?

Please get in touch with Golioth engineers by starting a new thread on
the [Golioth Forum](https://forum.golioth.io/).
