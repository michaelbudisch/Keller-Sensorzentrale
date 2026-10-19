# Home Assistant – Installation und Update

**Basis:** Wemos v1.9.6 + Dashboard/Panel v2.1.0. Die Dateinamen `heizoel-monitor-card.js` und `heizoel_dashboard_panel` sind aus Bestandskompatibilität bewusst erhalten; sichtbarer Titel „Kellersteuerung“.

## 1. Backup

`configuration.yaml`, bisherigen Inhalt von `packages/`, `www/heizoel-monitor-card.js`, `custom_components/heizoel_dashboard_panel/` und die aktuelle Lovelace-Karte sichern. Vorhandene reale Statistiken und Zähler nicht überschreiben.

## 2. Dateien kopieren

Alle Zielpfade beziehen sich **auf den Ordner, in dem deine tatsächlich aktive `configuration.yaml` liegt**; die Weboberfläche nennt diesen üblicherweise `/config`.

| Datei in diesem Repo | Ziel |
| --- | --- |
| `home-assistant/www/heizoel-monitor-card.js` | `/config/www/heizoel-monitor-card.js` |
| `home-assistant/custom_components/heizoel_dashboard_panel/` | `/config/custom_components/heizoel_dashboard_panel/` |
| `home-assistant/packages/keller_sensorzentrale.yaml` | `/config/packages/keller_sensorzentrale.yaml` |
| `home-assistant/packages/keller_verbrauch.yaml` | `/config/packages/keller_verbrauch.yaml` |

**Wichtig:** `.../homeassistant/config/packages/` ist auf manchem Dateisystem der falsche Ort, wenn `configuration.yaml` schon direkt in `.../homeassistant/` liegt. Nie nur aufgrund des WinSCP-Pfadnamens `/config` ein weiteres Verzeichnis erstellen.

## 3. `configuration.yaml` einbinden

Wenn `homeassistant:` bereits vorhanden ist, darunter nur **einmalig** eintragen:

```yaml
homeassistant:
  packages: !include_dir_named packages
```

Keine zweiten `homeassistant:`- oder `heizoel_dashboard_panel:`-Blöcke erzeugen. Die bisherigen Felder des Panels können weiterverwendet werden; ein Beispiel findest du unter [`home-assistant/`](../../home-assistant/configuration_BEISPIEL.yaml) (Auszug, keine vollständige Konfiguration).

**Entitäts-IDs prüfen:** In der Testinstallation heißt der rohe Abstand `sensor.heizungskeller_sensoren_abstand_geglattet` (tatsächlich **geglattet**, ohne „ae“). Wenn sich dein HA-Gerät anders nennt, musst du die Quelldaten und Panel-Zuordnung anpassen.

## 4. YAML prüfen und Neustart

HA-Konfigurationsprüfung ausführen, danach Home Assistant vollständig neu starten. Die Custom-Card wird unter `/local/heizoel-monitor-card.js` eingebunden. Panel bleibt unter `/heizoel-monitor` erreichbar. Browsercache bei alten JS-Ständen leeren.

## 5. Status und Verbrauch

- `sensor.heizungskeller_sensoren_abstand_geglattet` sollte Messwerte vom Wemos enthalten.
- `sensor.keller_tank_abstand_5min`, `sensor.keller_tank_einzeln_5min`, `sensor.keller_tank_gesamt_5min`, `sensor.keller_tank_fuellstand_5min` werden von HA berechnet. Kurzes `unknown` nach Neustart/bei fehlenden Daten ist möglich.
- `input_boolean.keller_verbrauch_aktiv` sollte nach Einlesen des Verbrauchspakets mit Zustand `off` existieren. Vor dauerhafter Nutzung echte Tankhöhe und mechanische Uhren vergleichen.
- `sensor.heizoel_verbrauch_heute`, `_woche`, `_monat`, `_jahr` benötigen Daten ab Aktivierung. Keine rückwirkende Historie.
- Temperaturdiagramme benötigen angeschlossene und zugeordnete DS18B20 sowie HA Recorder/Verlauf.

## 6. Troubleshooting

| Beobachtung | Prüfen |
| --- | --- |
| Helfer `input_boolean.keller_verbrauch_aktiv` fehlt | `packages/` unter **wirklichem** HA-Konfigurationsordner? YAML include korrekt? Neustart? |
| 5-Minuten-Abstand `unknown`, aber MQTT zeigt cm | `geglattet` vs. `geglaettet`, Filterquelle in YAML, Wemos eingeschaltet? |
| Literwerte fehlen, Abstand vorhanden | Abstand außerhalb 16-Punkte-Tankreferenz; noch nicht kalibriert? |
| Webseite nicht erreichbar, Relais via HA gehen | Webserver/Heap/WLAN untersuchen; MQTT funktioniert unabhängig vom HTTP-Server |
| Alle MQTT-Entities `unavailable` | Wemos, Broker, Discovery/Availability-Status kontrollieren |
| Temperaturlinien leer | DS18B20 vorhanden, ROM zugeordnet, Recorder eingeschaltet, historischer Zeitraum vorhanden? |

Für eine 30-Tage-Ansicht muss der Home-Assistant-Recorder auch entsprechend Daten behalten. Die vorhandenen Packages verändern die Recorder-Aufbewahrung nicht automatisch.
