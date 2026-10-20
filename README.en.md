# Keller Sensorzentrale — Home Assistant sensor hub

A Wemos D1 mini / ESP8266 DIY sensor hub with **six logical DS18B20 slots (three 1-Wire buses)**, **two optional relay outputs**, an **ultrasonic tank distance input**, MQTT Discovery, a lightweight local setup UI, a custom Home Assistant panel (v2.3.2: separate heating and oil sections, heating-cycle estimation with shaded activity intervals and temperature trend history), and history / fuel usage packages.

The repository is prepared for GitHub as a **prototype**, not a finished or certified product. The matching 3D-printed enclosure has been [published on MakerWorld](https://makerworld.com/en/models/3464324-wemos-d1-gehause-fur-ds18b20-und-relais#profileId-3953596).

- Current development and test status (German): [PROJECT_STATUS.md](PROJECT_STATUS.md)
- German assembly guide: [docs/hardware/BAUANLEITUNG.md](docs/hardware/BAUANLEITUNG.md)
- Pin map and electrical notes: [docs/hardware/ANSCHLUSSPLAN.md](docs/hardware/ANSCHLUSSPLAN.md)
- Home Assistant install: [docs/home-assistant/INSTALLATION.md](docs/home-assistant/INSTALLATION.md)
- Arduino sketch: [firmware/KellerSensorzentrale/KellerSensorzentrale.ino](firmware/KellerSensorzentrale/KellerSensorzentrale.ino)
- 3D-printed enclosure: [MakerWorld model](https://makerworld.com/en/models/3464324-wemos-d1-gehause-fur-ds18b20-und-relais#profileId-3953596) · [enclosure documentation](enclosure/README.md)

**Safety:** 3.3 V GPIOs are not 5 V tolerant. Do not connect 5 V echo signals directly. The prototype and ultrasonic sensor are not certified for hazardous atmospheres or for installation into a heating-oil tank. No mains or heating safety control wiring is covered.

This public-source export contains *no user passwords*. Configure the three `#BITTE_AENDERN` values in the sketch locally before compiling. Source code and documentation are prepared under the [MIT License](LICENSE). Any licensing of the 3D model is determined by the MakerWorld listing, separately from the source-code MIT license. Verify rights to included photos and conduct a hardware/build review before making the repository public.
