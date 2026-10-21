# Home Assistant – Installation und Update

**Basis:** Wemos v1.9.6 + Home-Assistant-Tank-Packages v2.1.0 + Heiztakt-Package v1.0.1 (optional) + Seitenpanel v2.4.0 + Lovelace-Karte v2.1.1. Die Dateinamen `heizoel-monitor-card.js` und `heizoel_dashboard_panel` sind aus Bestandskompatibilität bewusst erhalten; sichtbarer Titel „Kellersteuerung“.

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

## Lovelace-Karte: alte Anzeige oder falscher Gesamtbestand

Die **Lovelace-Karte** `custom:heizoel-monitor-card` ist nicht dasselbe wie das eigenständige **Seitenleistenpanel** `/heizoel-monitor`. Wenn die Seitenleiste „Kellersteuerung“ funktioniert, die Lovelace-Karte aber noch „Heizölmonitor“ mit leeren Literwerten zeigt, ist häufig die alte Datei oder die alte Karten-YAML aktiv.

1. **Backup** der bisherigen Karten-YAML und `/config/www/heizoel-monitor-card.js`.
2. Aktuelle **`home-assistant/www/heizoel-monitor-card.js` (v2.1.1)** nach `/config/www/heizoel-monitor-card.js` kopieren.
3. Unter **Einstellungen → Dashboards → Ressourcen** die bereits existierende JS-Modul-Ressource auf **`/local/heizoel-monitor-card.js?v=211`** ändern. **Keine zweite Ressource mit gleicher Datei hinzufügen.**
4. Betroffene Lovelace-Karte bearbeiten → Code-Editor: Einen alten Eintrag `entity:` entfernen und stattdessen `entities:` mit `total: sensor.keller_tank_gesamt_5min`, `single: sensor.keller_tank_einzeln_5min`, `percent: sensor.keller_tank_fuellstand_5min` verwenden. [Vollständiges Kartenbeispiel](../../home-assistant/Dashboard_Karte_Beispiel.yaml).
5. Dashboard neu laden (`Strg+F5`) oder im privaten Fenster öffnen. Solange die alte Oberfläche erscheint, ist noch die alte JS-Version aktiv.

**Kein Wemos-Flash und keine HA-Package-Änderung nötig.** Bestehende Verbrauchsstatistiken nicht zurücksetzen.

## 4. YAML prüfen und Neustart

HA-Konfigurationsprüfung ausführen, danach Home Assistant vollständig neu starten. Die Custom-Card wird unter `/local/heizoel-monitor-card.js` eingebunden. Panel bleibt unter `/heizoel-monitor` erreichbar. Browsercache bei alten JS-Ständen leeren.

## Heizkreis-Anzeige im Seitenpanel v2.4.0

Im Seitenleistenpanel `/heizoel-monitor` werden nun **Heizung Vorlauf** (Slot 1), **Heizung Rücklauf** (Slot 2) und **Spreizung = Vorlauf − Rücklauf** (K) angezeigt. Ein eigenes Heizkreis-Diagramm verwendet zwei Skalen: links Temperaturen (°C), rechts die Spreizung (K). Zeiträume: 24 h, 7 Tage, 30 Tage. Die normalen sechs Slots und der bisherige Verlauf bleiben sichtbar.

**Zuordnung:** Im Panel-Reiter **Einstellungen** kannst du „Heizkreis Vorlauf“ und „Heizkreis Rücklauf“ auf beliebige zwei verschiedene Temperatur-Entitäten legen. Ein zusätzlicher Verlaufssensor `sensor.heizung_spreizung` ist optional. Falls er fehlt oder noch keine Recorder-Historie hat, schätzt das Diagramm die Differenz aus den beiden vorhandenen Temperatur-Zeitreihen; bei fehlenden oder auseinanderliegenden Daten zeigt es keine erfundenen Werte. Der **aktuelle** Differenzwert wird direkt aus den beiden gültigen Live-Temperaturen berechnet und benötigt keinen Helfer.

**Globale Zuordnung** (im **bereits bestehenden** `heizoel_dashboard_panel:`-Block unter `configuration.yaml`, nicht doppelt anlegen):

```yaml
heizoel_dashboard_panel:
  sidebar_title: Kellersteuerung
  sidebar_icon: mdi:home-thermometer
  url_path: heizoel-monitor
  entities:
    flow: sensor.heizungskeller_sensoren_slot_1
    return: sensor.heizungskeller_sensoren_slot_2
    spread: sensor.heizung_spreizung  # optional, wenn Helfer vorhanden
```

**Upgrade ohne Wemos-Flash:** Backup des Ordners `/config/custom_components/heizoel_dashboard_panel/` erstellen. Aus GitHub den vollständigen Ordner `home-assistant/custom_components/heizoel_dashboard_panel/` übernehmen (`__init__.py`, `manifest.json`, `frontend/heizoel-panel.js`). HA-Konfiguration prüfen, Home Assistant vollständig neu starten und Browser mit **Strg+F5** neu laden. Die Registrierung lädt das neue JavaScript mit `?v=2_4_0`, damit der alte Browsercache nicht weiter verwendet wird. Die Lovelace-Karte ist davon unabhängig.

## Eigenständiger Bereich „Heizung & Takte“ (Panel v2.4.0)

**Alles für die Heizung ist im neuen Reiter „Heizung & Takte“ gebündelt.** Die Startseite bleibt für allgemeine Sensor-Slots und Relais. „Tank & Verlauf“ und „Ölverbrauch“ enthalten ausschließlich die Heizöldaten. Das generische Temperatur-Slot-Diagramm bleibt im Reiter „Temperatur-Slots“.

Zusätzliche Statistikfunktionen erfordern das optionale Paket **`home-assistant/packages/keller_heiztakte.yaml`**. Lege die Datei neben `keller_sensorzentrale.yaml` und `keller_verbrauch.yaml` in den Ordner `packages/` der **aktiven** `configuration.yaml`. Füge keine zweite `homeassistant:`- oder `packages:`-Konfiguration hinzu.

Nach einer Konfigurationsprüfung **Home Assistant vollständig neu starten** und im neuen Heizungsreiter die Schaltfläche **„Auswertung aktivieren“** benutzen (Helfer `input_boolean.keller_heizkreis_auswertung_aktiv`). **Es werden keine alten Starts rückwirkend gezählt.** Die Trendgrafik der Temperaturen funktioniert auch ohne aktivierte Taktanalyse, aber markierte Aktivphasen und die Zähler benötigen das neue Paket.

Der Reiter zeigt: aktueller Zustand, erkannte Starts heute/Woche, Aktivphasen in den letzten 60 Minuten/24 Stunden, Aktiv-/Pausenzeit heute, mittlere/letzte/kürzeste/längste Aktiv- und Pausendauern, 24-h-Mitteltemperaturen sowie Vorlaufmaximum, Rücklaufminimum und höchste Spreizung. Der Verlauf markiert bestätigte Temperatur-Aktivphasen dezent.

**Wichtig:** Die Messung erfolgt anhand der Temperaturspreizung; das ist **kein echter Brennerkontakt**. Die erste Erkennung beginnt bei 8,5 K und endet unter 5,5 K; mindestens 30 s aktiv und 45 s inaktiv. Schwellen und Zeitfilter sind im Package dokumentiert und sollten anhand mehrerer Tage nachvollzogen werden. Ausfälle und fehlende Messwerte werden nicht als gültige Takte nachgezählt.

Package v1.0.1 verhindert die falsche erste Pausenzeit. Das Panel v2.3.2 unterstützt die bestätigten Statistik-IDs mit `rucklauf`, `maximum` und `minimum` und gruppiert die Anzeige. Bereits fehlerhafte Pausenwerte können [gezielt korrigiert werden](HEIZTAKTE.md#live-test-korrektur-v101--panel-v231).

Eine genaue Anleitung, Entitätsliste, Grenzen und Kalibrierhinweise enthält **[HEIZTAKTE.md](HEIZTAKTE.md)**. Es sind **keine Firmware- oder Relaisänderungen** erforderlich.

## Tagesverlauf mit Detailansicht (Panel v2.4.0)

Die Heizkreisgrafik zeigt **nur den aktuellen Kalendertag** ab 00:00 Uhr in der **lokalen Browser-Zeitzone**. Die 7-/30-Tage-Buttons im Heizkreis sind entfallen; die anderen Verläufe (Öltank und sechs allgemeine Temperaturslots) bleiben unverändert.

Über den Knopf **„Tagesdetails öffnen“** beziehungsweise einen Klick auf die Grafik öffnet sich eine vergrößerte Ansicht mit:

- **Kalenderauswahl** und den Schaltflächen **Vortag**, **Nächster Tag**, **Heute**.
- **Ganzer Tag** oder einzeln **00–06**, **06–12**, **12–18**, **18–24 Uhr**.
- **Alle Kurven**, **Vorlauf/Rücklauf** ohne Spreizung oder **nur Spreizung**.
- Angepasster linker °C- und rechter K-Achse, soweit die ausgewählten Kurven dafür Daten haben.
- Schließen-Knopf und **Esc**-Taste zum Zurückkehren.

Die Abfrage verwendet den Home-Assistant-Recorder für den **konkret ausgewählten Tag**. Heute wird höchstens 60 Sekunden im Browser zwischengespeichert; alte Tage werden aus Gründen der Geschwindigkeit länger im aktuellen Browser-Tab gehalten (maximal 16 Tagesdatensätze). Es werden keine Historien lokal dauerhaft gespeichert.

**Hinweis:** Frühere Tage sind nur verfügbar, wenn Home Assistant sie noch im Recorder aufbewahrt. Bei fehlenden Daten zeigt das Panel einen Hinweis. Kalendertage werden von Mitternacht zu Mitternacht abgefragt, einschließlich der üblichen Sommer-/Winterzeit-Umstellung.

**Update:** Gegenüber v2.3.2 nur die Dateien \`frontend/heizoel-panel.js\`, \`__init__.py\` und \`manifest.json\` des Seitenpanels ersetzen. Keine Änderung am Wemos, der vorhandenen Heiztakt-Auswertung v1.0.1, den Tank-Packages oder an der Lovelace-Karte. Home Assistant vollständig neu starten, danach **Strg+F5**.

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
| Heiztakt-Werte fehlen | Neues `keller_heiztakte.yaml`-Package installiert, HA neugestartet, `input_boolean.keller_heizkreis_auswertung_aktiv` eingeschaltet? Quellen Slot 1/2 gültig? |

Für eine 30-Tage-Ansicht muss der Home-Assistant-Recorder auch entsprechend Daten behalten. Die vorhandenen Packages verändern die Recorder-Aufbewahrung nicht automatisch.
