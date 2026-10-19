# Projektstatus – Keller Sensorzentrale

**Stand:** 19.10.2026 · **GitHub-Dokumentationsstand:** v0.1.4 · **Status:** im eigenen Aufbau erprobter Prototyp, kein zertifiziertes Fertigprodukt.

> **Startpunkt für neue Chats und Beiträge:** Vor Änderungen diese Datei und die jeweils aktuellen GitHub-Quelldateien lesen. Der Code im Branch `main` ist die öffentliche Quellbasis; die genaue Konfiguration des bereits installierten Geräts kann davon abweichen. Für Live-Einstellungen und Probleme stets aktuelle Screenshots, Diagnosedaten oder Konfigurationsauszüge heranziehen. Keine Passwörter oder privaten Zugangsdaten ins Repository übernehmen.

## 1. Projektüberblick

Ein Wemos D1 mini (ESP8266) liest einen Ultraschall-Tanksensor und bis zu sechs DS18B20-Temperatursensoren aus, steuert zwei Relais und veröffentlicht die Werte über MQTT an Home Assistant. Im Wemos bleiben Messung, kurzer Median, 16-Punkte-Tankkennlinie, Sensorzuordnung, Relais, Einstellungen und Diagnose. Home Assistant übernimmt zeitliche Glättung, Verlauf, Anzeige und die optional aktivierbare Heizöl-Verbrauchs-/Nachtank-Auswertung.

| Bereich | Öffentlicher Stand | Praxiserfahrung / Grenzen |
| --- | --- | --- |
| ESP8266-Firmware | **v1.9.6 (HA-Lite / Statusfix)** | Eingebauter Wemos übermittelt Tankwerte und MQTT-Status; GitHub-Sketch ist eine von Zugangsdaten bereinigte Vorlage, kein binär identisches Abbild des laufenden Geräts. |
| Home Assistant | **v2.1.0 (Dashboard, Panel, Packages)** | Tankabstand, geglättete Liter und Gesamtbestand erschienen in HA; Schalten der Relais über MQTT/HA wurde beobachtet. |
| Tankmessung | **Aktiv, ein Sensor** | Bei der letzten Rückmeldung funktionierten Distanzmessung und Literanzeige. Beide mechanischen Tankuhren zeigten ungefähr den gleichen Stand; exakte Messtoleranz nicht bestimmt. |
| Tankhochrechnung | **Zwei gleichartige Tanks** | Gemessen wird ein Tank; der zweite wird aus gleichem angenommenem Füllstand hochgerechnet. Im HA-Package ist die aktuelle Einzelkapazität **720 L** für den Prozentwert fest hinterlegt; bei anderen Tanks anpassen. |
| Temperaturslots | **Sechs Buchsen/Slots vorgesehen** | Auf der Platine verdrahtet, aber zur letzten Rückmeldung waren **keine angeschlossenen DS18B20-Messwerte** bestätigt. Anzeige und Verlauf daher noch leer. |
| Relais | **Zwei Kanäle** | Status/Steuerung aus HA möglich, lokale Belastbarkeit, Versorgung und dauerhafter Betrieb noch nicht abschließend qualifiziert. Keine Netzspannung/Sicherheitsfunktionen. |
| Verbrauchsstatistik | **Optionales HA-Package vorhanden** | Helfer `input_boolean.keller_verbrauch_aktiv` existiert. Die tatsächliche spätere Aktivierung und belastbare Tages-/Wochen-/Monats-/Jahreswerte sind **nicht abschließend bestätigt**. |
| 3D-Gehäuse | **Veröffentlicht** | [MakerWorld-Modell + Druckprofil](https://makerworld.com/en/models/3464324-wemos-d1-gehause-fur-ds18b20-und-relais#profileId-3953596), [Fotos und Montage](docs/galerie/README.md). Die 3MF/STL-Dateien liegen auf MakerWorld, nicht im GitHub-Repository. |

**Zuletzt gezeigte Beispielwerte (19.10.2026, keine Live-Werte):** Wemos ca. **23,2 cm** Tankabstand und **1.366,8 L** hochgerechneter Bestand. Home Assistant wegen 5-Minuten-Glättung zeitlich versetzt ca. **1.353,8 L / 94,0 %**; früher auch ca. 1.358,8 L / 94,4 %. Solche Momentaufnahmen dürfen nicht als statische Konfigurationswerte oder gesicherte Füllmengen übernommen werden.

## 2. Hardware und GPIO-Belegung

| Anschluss | Wemos D1 mini | Verwendung |
| --- | --- | --- |
| Slot 1 + 2 | **D1 / GPIO5** | 1-Wire-Datenbus A, ein Pull-up (ca. 5 kΩ) an 3,3 V |
| Slot 3 + 4 | **D2 / GPIO4** | 1-Wire-Datenbus B, ein Pull-up (ca. 5 kΩ) an 3,3 V |
| Slot 5 + 6 | **D5 / GPIO14** | 1-Wire-Datenbus C, ein Pull-up (ca. 5 kΩ) an 3,3 V |
| Tank ECHO/TX | **D6 / GPIO12** | Abstandspulse, **mit** geeigneter 5-V-auf-3,3-V-Pegelanpassung |
| Relais 1 | **D7 / GPIO13** | Schaltausgang, im Sketch active LOW voreingestellt |
| Relais 2 | **D0 / GPIO16** | Schaltausgang, im Sketch active LOW voreingestellt |

- **Sechs 3-Pin-Stecker:** jeweils **3V3, DATA, GND**; zwei pro 1-Wire-Bus. Physische Pinreihenfolge keinesfalls aus Fotos ableiten.
- **Zwei 4-Pin-Stecker:** Tank (5V/GND/TX-ECHO/RX-TRIG; RX derzeit frei) und Relais (5V/GND/IN1/IN2). Beide Anschlüsse elektrisch unterscheiden und prüfen.
- **Achtung Pegel:** Der verwendete Prototyp wurde mit drei 5-kΩ-Widerständen als Spannungsteiler aufgebaut (5 kΩ oben / 10 kΩ unten); das liefert nominell **3,33 V** und wenig Reserve. Die [öffentliche Nachbauanleitung](docs/hardware/ANSCHLUSSPLAN.md) beschreibt eine konservativere Pegelanpassung. Kein 5-V-Signal direkt an den GPIO!
- Hardwareanleitungen: [Bauanleitung](docs/hardware/BAUANLEITUNG.md) · [Netzliste](docs/hardware/ANSCHLUSSPLAN.md) · [SVG-Prinzipbild](docs/hardware/anschlussplan.svg) · [Stückliste](docs/hardware/STUECKLISTE.md).

## 3. Software, Datenfluss und Kompatibilität

**Wemos:**
- [`firmware/KellerSensorzentrale/KellerSensorzentrale.ino`](firmware/KellerSensorzentrale/KellerSensorzentrale.ino) – öffentliches v1.9.6-Quelltemplate, drei `#BITTE_AENDERN`-Platzhalter, ESP8266-Boardpaket, WiFiManager, PubSubClient, OneWire und DallasTemperature.
- EEPROM speichert Kalibrierung/Zuordnungen und Geräteeinstellungen. Vor Änderungen oder OTA die lokale Konfiguration sichern.
- Die abgespeckte Weboberfläche dient der Konfiguration und Diagnose; MQTT ist der primäre Home-Assistant-Datenweg.

**Home Assistant:**
- [Installationsanleitung](docs/home-assistant/INSTALLATION.md) und [Dateien](home-assistant/).
- Bestehende Namen aus Kompatibilitätsgründen beibehalten: `www/heizoel-monitor-card.js`, `custom_components/heizoel_dashboard_panel`, Panel-Pfad `/heizoel-monitor`; sichtbarer Titel **Kellersteuerung**.
- HA-Packages liegen im Ordner **`packages/` unmittelbar neben der aktiven `configuration.yaml`**. Bei der realen Installation führte ein irrtümliches zusätzliches `config/`-Unterverzeichnis dazu, dass Helfer zunächst fehlten.
- Bestätigte MQTT-Rohentität: **`sensor.heizungskeller_sensoren_abstand_geglattet`**, ausdrücklich **`geglattet`**, nicht `geglaettet`. Dies war zuvor ein echter Zuordnungsfehler.
- HA-Filter/Template-Entitäten: `sensor.keller_tank_abstand_5min`, `sensor.keller_tank_einzeln_5min`, `sensor.keller_tank_gesamt_5min`, `sensor.keller_tank_fuellstand_5min`.
- Verbraucher-Helfer: `input_boolean.keller_verbrauch_aktiv`. Der Verbrauchsalgorithmus enthält Schwellenwerte und benötigt die Prüfung anhand realer Tage, längerer Ausfälle und Nachtank-Vorgänge. Nicht eigenmächtig Null setzen oder Statistiken zurücksetzen.

## 4. Bekannte Grenzen und noch offene Prüfungen

1. **Webserver-Stabilität:** Historisch war die Weboberfläche zeitweise unerreichbar, während MQTT/HA-Relais weiter funktionierten. Nach HA-Lite ca. **27–29 KB freier Heap** in gezeigten Momentaufnahmen, aber **keine belastbare Langzeit-Entwarnung**. Bei neuen Ausfällen Heap-Minimum, größter Block, Fragmentierung, Neustartursache, Laufzeit sowie WLAN/MQTT erfassen.
2. **Langzeittest und Zuverlässigkeit:** Keine systematisch dokumentierten Wiederanlauf-, Sensorabbruch-, WiFi/MQTT-Ausfall- und Relaisschalt-Lasttests.
3. **Temperatur:** Noch reale DS18B20 je Bus anstecken, ROM-Adressen Slots zuordnen, MQTT/HA-Entitäten und Verläufe testen. Für 30-Tage-Grafiken Recorder-Aufbewahrung prüfen.
4. **Tankkalibrierung:** Gegen die mechanischen Uhren plausibilisiert, aber Verbrauchsschwellwerte, Toleranzen und Tankgleichlauf über Zeit sowie Füllstand bei verschiedenen Pegeln noch zu prüfen.
5. **Firmware-Build:** Die öffentliche v1.9.6-Vorlage wurde beim Projekt-Export **nicht als vollständiger reproduzierbarer ESP8266-Build bzw. Hardware-Test** verifiziert. Die drei persönlichen Zugangsdaten gehören niemals in Git.
6. **Schaltplan:** Der GitHub-Plan ist eine elektrische Prinzipskizze, kein fertig geprüfter PCB-/Gerber-Entwurf. Steckerausrichtung, Pegel, 3,3-V-Tauglichkeit und Stromversorgung bei einem Nachbau erneut prüfen.
7. **Sicherheit:** Keine Netzspannung oder sicherheitsrelevante Heizungssteuerung über den DIY-Prototyp. Tanksensor und Gehäuse sind nicht als zugelassene Einrichtungen für Tankinnenraum oder explosionsgefährdete Bereiche nachgewiesen. Fachgerechte Montage und Zulassungen eigenständig prüfen.

## 5. Aufgabenliste (nächste sinnvolle Arbeiten)

- [ ] Temperaturfühler anschließen, ROM-Zuordnung und Verlauf mit echtem Messverlauf prüfen.
- [ ] Webserver- und MQTT-Stabilität mehrere Tage beobachten, Diagnose erfassen und ggf. HTTP/Heap gezielt optimieren.
- [ ] Reproduzierbare Arduino-Buildumgebung mit exakten Bibliotheksversionen und Compile-Test einrichten.
- [ ] Home-Assistant-Verbrauchslogik mit echten Messreihen validieren, Nachtank-Erkennung testen und ggf. dokumentierte Referenzierung ergänzen.
- [ ] Hardware-Bauanleitung gegen finale Platine und Gehäusemaße prüfen; Fotos sind kein eindeutiger Stromlaufplan.
- [ ] Releaselogik/Changelog und Sicherheitsprüfung bei jedem neuen Softwarestand beibehalten.
- [x] Projekt auf GitHub veröffentlicht; MakerWorld-Gehäuse verlinkt.

## 6. Vereinbarter Workflow für weitere Entwicklungs-Chats

1. **GitHub zuerst:** Vor jeder Codearbeit den aktuellen Branch `main`, diese Datei, README, `CHANGELOG.md` und betroffene Quelldateien live einlesen.
2. **Nicht von Alt-Chats übernehmen:** Frühere Chat-ZIPs, Versionen und vermutete GPIO-/Entity-Namen gelten nicht automatisch als gültig. Abweichungen zur laufenden Installation explizit klären.
3. **Änderungen klein und nachvollziehbar:** Firmware vX.Y.Z, HA-Version und Repo-Dokumentationsversion **getrennt** führen; keine künstlichen Firmware-Upgrades bei reinen README-/Foto-Änderungen.
4. **Testen statt versprechen:** Syntax-/YAML-Prüfungen, möglichst ESP8266-Kompilierung, Änderungsvergleich und Auswirkungen auf MQTT Discovery, EEPROM, HA-Entity-IDs, Kalibrierung und Relaisstartzustand prüfen. Nicht durchgeführte Hardwaretests benennen.
5. **Veröffentlichen:** Dokumentation, `CHANGELOG.md`, Versionsübersicht und diese Datei synchron aktualisieren; GitHub-Commit/Link nennen. Zugangsdaten und lokale IP-Adressen niemals veröffentlichen.
6. **Bei lokaler Installation:** Vor dem Firmware-Update aktuelle Einstellungen sichern, vor Home-Assistant-Änderungen die aktiven YAML-Dateien und Zählerstände sichern; keine bestehenden Zähler unbemerkt zurücksetzen.

## 7. Wichtigste Links

- [GitHub-Repository](https://github.com/michaelbudisch/Keller-Sensorzentrale)
- [Softwareinstallation](docs/home-assistant/INSTALLATION.md)
- [Elektrische Bauanleitung](docs/hardware/BAUANLEITUNG.md)
- [Projektbilder](docs/galerie/README.md)
- [MakerWorld-Gehäuse](https://makerworld.com/en/models/3464324-wemos-d1-gehause-fur-ds18b20-und-relais#profileId-3953596)

---

*Diese Datei beschreibt den zuletzt **berichteten** Entwicklungs- und Teststand; sie ist kein automatischer Live-Status des Wemos oder von Home Assistant.*
