# MQTT RGB Light Strips

A WiFi Connected RGB LED Strip using MQTT. Read the full blog post at
[selfhostedhome.com](https://selfhostedhome.com/wifi-connected-rgb-led-strip/).

A second blogpost details the effects for syncing up the LEDs to music. This is also available at [selfhostedhome.com](https://selfhostedhome.com/wifi-rgb-led-strip-music-visualization/).

![Rainbow Lights](images/rgb-rainbow.gif)

## Hardware

Please refer to the blog post for more information. Essentially this is using
[WS2811 LED strips](https://amzn.to/2Cg3rxm) controlled by a [NodeMCU](https://amzn.to/2NI5s7t). For music based effects I use the [MSGEQ7](https://amzn.to/2CDbqEZ) for audio processing..

Check out the `schematic/` folder for more information.

## Software

### Embedded

This repository uses [PlatformIO](https://platformio.org/) for building the
embedded software for the NodeMCU. Check out their installation docs for more
information on how to get started with PlatformIO.

The code here supports:
* Control over MQTT topics
* Color Fill
* Synchronize LEDs to Music
* Different Effects
    * rainbow
    * rainbow\_with\_glitter
    * confetti
    * sinelon
    * bpm
    * juggle
    * candycane
    * party\_colors
    * rotate\_party\_colors
* Music Based Effects
    * music\_rainbow
    * music\_rgb
    * music\_rotate\_party\_colors
    * music\_cycle_
* Over the Air Updates

To get started rename the `src/config_template.h` file to `src/config.h`.
Afterwards update the MQTT topics, WiFi information and LED strip information
to match your setup.

### Home Assistant

Refer to the Home Assistant yaml snippet in the `homeassistant` directory for
how to set up MQTT lights in Home Assistant.
