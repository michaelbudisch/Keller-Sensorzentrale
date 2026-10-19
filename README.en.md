# Keller Sensorzentrale — Home Assistant sensor hub

A Wemos D1 mini / ESP8266 DIY sensor hub with **six logical DS18B20 slots (three 1-Wire buses)**, **two optional relay outputs**, an **ultrasonic tank distance input**, MQTT Discovery, a lightweight local setup UI, a custom Home Assistant panel, and history / fuel usage packages.

The repository is prepared for GitHub as a **prototype**, not a finished or certified product. The matching 3D-printed case is being prepared for a separate MakerWorld publication.

- German assembly guide: [docs/hardware/BAUANLEITUNG.md](docs/hardware/BAUANLEITUNG.md)
- Pin map and electrical notes: [docs/hardware/ANSCHLUSSPLAN.md](docs/hardware/ANSCHLUSSPLAN.md)
- Home Assistant install: [docs/home-assistant/INSTALLATION.md](docs/home-assistant/INSTALLATION.md)
- Arduino sketch: [firmware/KellerSensorzentrale/KellerSensorzentrale.ino](firmware/KellerSensorzentrale/KellerSensorzentrale.ino)
- Printing / MakerWorld preparation: [enclosure/README.md](enclosure/README.md)

**Safety:** 3.3 V GPIOs are not 5 V tolerant. Do not connect 5 V echo signals directly. The prototype and ultrasonic sensor are not certified for hazardous atmospheres or for installation into a heating-oil tank. No mains or heating safety control wiring is covered.

This public-source export contains *no user passwords*. Configure the three `#BITTE_AENDERN` values in the sketch locally before compiling. Source code and documentation are prepared under the [MIT License](LICENSE). Enclosure models may receive their own license when published later on MakerWorld. Verify rights to included photos and conduct a hardware/build review before making the repository public.
