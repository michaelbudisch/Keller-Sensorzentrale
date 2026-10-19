# MakerWorld – Veröffentlichungstexte für Keller Sensorzentrale

> Vor der Veröffentlichung: eigene 3MF/STL-Dateien hochladen, Druckprofil testen, Maße und Lizenz wählen, MakerWorld-URL später im GitHub-Projekt ergänzen. Die hier genannten Eigenschaften stammen aus den Fotos des Prototyps; es werden keine unbestätigten Abmessungen oder Druckprofile angegeben.

## Deutsch

### Titel
Keller Sensorzentrale – 3D-Druck-Gehäuse für Wemos D1 mini, Sensorboard & 2-Kanal-Relais

### Kurzbeschreibung
Modulares DIY-Gehäuse für einen ESP8266-Heizungskeller-Monitor mit Sensoranschlüssen, Relais, Kabelausgängen und optionalem Rohrhalter. Firmware und Home Assistant auf GitHub.

### Beschreibung
Dieses Gehäuse habe ich für meine selbst entwickelte **Keller Sensorzentrale** entworfen. Im Inneren finden ein **Wemos D1 mini (ESP8266)**, meine Lochraster-Sensorplatine und ein **2-Kanal-Relaismodul** Platz. Die Sensorzentrale misst unter anderem den Füllstand meiner Heizöltanks und stellt Messwerte und Relaiszustände über MQTT für **Home Assistant** bereit.

Die Bilder zeigen meinen funktionsfähigen Prototypen im Gehäuse sowie eine Montage an einem Rohr mithilfe einer separat gedruckten Halterung und Kabelbindern.

**Gehäusemerkmale**
- Getrennte Bereiche für Sensorplatine und Relaismodul
- Öffnungen für Spannungsversorgung, Steckverbinder und Kabel
- Passender Deckel mit Haltestrukturen
- Zugang zu den Anschlüssen ohne komplette Demontage des Elektronikaufbaus
- Separat gezeigte Rohrhalterung für meinen eigenen Montageort

**Was du zusätzlich benötigst**
Elektronik und Verkabelung sind nicht Bestandteil des Druckmodells. Die Abmessungen der eigenen Platine, die Lage der Stecker und die verwendeten Module müssen zum Gehäuse passen. Bitte vor dem Druck die Modellansichten und Maße kontrollieren.

**Firmware, Schaltplan und Home Assistant**
Das gesamte Open-Source-Projekt mit Firmware, Bauanleitung und Home-Assistant-Dashboard findest du hier:
https://github.com/michaelbudisch/Keller-Sensorzentrale

**Hinweis zur Sicherheit**
DIY-Prototyp, kein geprüftes Elektrogehäuse. Verwende es nur für fachgerecht aufgebaute, ungefährliche Kleinspannungsanwendungen. Keine Netzspannung im gedruckten Gehäuse und keine sicherheitsrelevanten Heizungssteuerungen. Eingriffe an einem Heizöltank oder seiner Abdichtung müssen fachgerecht und entsprechend den örtlichen Vorschriften erfolgen.

### Schlagwörter
ESP8266, Wemos D1 mini, Home Assistant, MQTT, Sensorgehäuse, Elektronikgehäuse, DIY, Heizungskeller, 2-Kanal-Relais, Rohrhalter, 3D-Druck

## English

### Title
Basement Sensor Hub – 3D Printed Enclosure for Wemos D1 mini, Sensor Board & Dual Relay

### Short description
A custom DIY enclosure for an ESP8266 sensor hub with sensor connectors, relay module, accessible cable ports and an optional pipe-mount bracket. Firmware and Home Assistant files on GitHub.

### Description
I designed this enclosure for my own **Basement Sensor Hub** project. It holds a **Wemos D1 mini (ESP8266)**, my custom perfboard sensor breakout and a **two-channel relay module**. The controller can monitor heating-oil tank level, publish sensor readings through MQTT and integrate with **Home Assistant**.

The photos show my working prototype, the internal component arrangement and a separately printed bracket used to mount the enclosure to a pipe with cable ties.

**Features**
- Dedicated space for the sensor board and two-channel relay module
- Openings for power, connectors and cables
- Matching lid with retention features
- Accessible connections for maintenance
- Optional pipe bracket shown in the photos

**Before printing**
Electronics and wiring are not included in the 3D model. Check your board dimensions, connector positions and exact modules against the uploaded model before printing. The design is made for my particular prototype.

**Firmware, wiring and Home Assistant**
Get the open-source controller firmware, wiring documentation and Home Assistant integration here:
https://github.com/michaelbudisch/Keller-Sensorzentrale

**Safety**
This is an experimental DIY design, not a certified electrical enclosure. Use it only for properly assembled low-voltage projects, not mains wiring or safety-critical heating control. Modifications to oil tanks and seals require qualified installation and compliance with local regulations.

### Tags
ESP8266, Wemos D1 mini, Home Assistant, MQTT, sensor enclosure, electronics box, DIY, basement, dual relay, pipe mount, 3D printing
