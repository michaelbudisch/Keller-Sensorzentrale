# Changelog

## GitHub-Projektpaket 0.2.2 — 2026-10-23

- Lovelace-Dashboard **v1.0.2**: Wochen- und Monatsverbrauch zeigen jetzt direkt die in der echten Home-Assistant-Installation bestätigten Entitäten `sensor.heizoel_verbrauch_kalenderwoche` und `sensor.heizoel_verbrauch_kalendermonat` an. Die vorläufige Markdown-Suche aus v1.0.1 entfällt.
- Die täglichen Verbrauchsbalken (`change`) und die kumulierte Verbrauchsstatistik (`state`) bleiben an `sensor.heizoel_verbrauch_gesamt` gebunden. Der separat vorhandene Altzähler `sensor.heizoel_verbrauch_kumuliert` wird nicht für die Auswertung verwendet.
- Keine Änderung an Tank- oder Heiztaktberechnung, Zählerständen, Wemos-Firmware oder MQTT.


## GitHub-Projektpaket 0.2.1 — 2026-10-23

- Lovelace Verbrauchsseite v1.0.1: Tagesverbrauch über tägliche Änderung (`change`) des kumulierten Sensors (`total_increasing`) als Balkendiagramm; Gesamtverbrauch über Statistiktyp `state`.
- Wochen-/Monatssensoren werden bei abweichenden HA-IDs über eine dynamische native Markdown-Anzeige erkannt; bei fehlender Entität erscheint eine klare Diagnose statt einer fehlerhaften Tile. Bestehende Sensorwerte, Helfer und Wemos-Firmware bleiben unverändert.
- Live-Test der beiden fehlenden Utility-Meter-Entitäten steht aus.


## GitHub-Projektpaket 0.2.0 — 2026-10-22

- Neues vollständig **natives Lovelace-Dashboard** `home-assistant/Kellersteuerung_Lovelace.yaml`: 7 Ansichten, Standardkarten ohne HACS und ohne selbst programmiertes Frontend.
- Heizkreis: übersichtliche 6-Stunden-Rohhistorie sowie zwei getrennte `statistics-graph`-Karten für Temperatur (°C) und Spreizung (K), mit gekoppeltem `energy-date-selection`-Kalender.
- Tank / Verbrauch / Heiztakte / sechs DS18B20-Slots / Relais / Diagnose zusammengefasst, auf Basis der bekannten HA-Entitäts-IDs.
- Relais-Kacheln öffnen zunächst die Detailansicht statt direkt umzuschalten. Mess-/Verbrauchszähler, Recorder, Wemos-Firmware und alte Lovelace-Karten bleiben unverändert.
- Import per HA-Dashboard-Editor oder als YAML-Datei in `lovelace.dashboards` dokumentiert. Live-Test in der Nutzerinstallation steht noch aus.


## GitHub-Projektpaket 0.1.10 — 2026-10-21

- Seitenpanel **v2.4.0**: Im Heizkreis steht künftig ausschließlich **heute** im Standarddiagramm statt einer überfüllten 7-/30-Tage-Grafik.
- **Anklickbare Tagesdetails:** großer Kalenderdialog mit Datumsauswahl, Tag zurück/vor, Heute, ganzer Tag/6-Stunden-Ausschnitte und Filter für Vor-/Rücklauf bzw. Spreizung. Schließen auch per Escape.
- Tagesbezogene Recorder-Abfragen mit korrekten lokalen Tagesgrenzen und zeitlich begrenztem Browsercache; keine Datenspeicherung oder Berechnung auf dem Wemos.
- Tank-/Ölverbrauchsverlauf, Sensor-Slot-Verläufe, Heiztakt-Package v1.0.1 und Firmware v1.9.6 bleiben unverändert.


## GitHub-Projektpaket 0.1.9 — 2026-10-20

- **Kellersteuerungs-Panel v2.3.2:** Home-Assistant-Statistik-IDs mit `rucklauf`, `maximum` und `minimum` als feste, geprüfte Varianten berücksichtigt; keine Zuordnung über unscharfe Namen.
- Heiztakt-Analyse jetzt in drei klar getrennten Bereichen: **Taktung** (4 Kennzahlen), **Lauf- und Pausenzeiten** (10 Kennzahlen) und **Temperaturstatistik · 24 Stunden** (6 Kennzahlen).
- Anzeige-Updates reagieren auf alle aufgelösten Statistikentitäten. Fehlende Messwerte bleiben als Strich sichtbar. Dokumentation ergänzt um drei tatsächlich bestätigte Entitäts-IDs und den Hinweis auf unvollständige 24-h-Abdeckung.
- **Keine Änderung am Wemos (v1.9.6), HA-Heiztakt-Package (v1.0.1), Tank- und Ölverbrauchspaketen oder Lovelace-Karte.**


## GitHub-Projektpaket 0.1.8 — 2026-10-20

- Heiztakt-Package **v1.0.1**: erste, unbestätigte Pause nicht mehr aus altem Zeitstempel berechnen; Gültigkeitsmarker werden bei Neustart und Umschalten zurückgesetzt, vorhandene Statistiken bleiben erhalten.
- Seitenpanel **v2.3.1**: Statistik-Entitäten mit und ohne `keller_`-Präfix erkennen und Anzeige bei neuen 24h-Werten aktualisieren.
- Dokumentation zur gezielten Entfernung eines zuvor gespeicherten falschen Pausenwerts ergänzt; Firmware und Ölauswertung unverändert.


## GitHub-Projektpaket 0.1.7 — 2026-10-20

- **Kellersteuerung Seitenpanel v2.3.0:** Ein eigener Reiter **Heizung & Takte** trennt Heizkreis und Taktanalyse von **Tank & Verlauf** und **Ölverbrauch**; Startseite mit allgemeiner Sensor-/Relaisübersicht. Der sechs-Slot-Temperaturbereich bleibt separat.
- Neues optionales Home-Assistant-Package **`keller_heiztakte.yaml` v1.0.0**: 2-Punkt-Hysterese (über 8,5 K/unter 5,5 K), Entprellung, abgeleiteter Aktivzustand, Starts/Zeiten, zuletzt/Ø/Minimum/Maximum erkannter Zyklen, 24-h-Temperatur- und Spreizungsstatistik sowie vorsichtige Taktbewertung.
- Markierung erkannter Aktivintervalle in der Heizkreisgrafik; History-Cache für Heizkurven auf 60 s reduziert.
- Neue HA-Einrichtungs- und Kalibrierdokumentation. Messwerte nur Näherung der Brenneraktivität; keine Änderungen am Wemos, MQTT, den bestehenden Tank-Packages oder der Lovelace-Karte.


## GitHub-Projektpaket 0.1.6 — 2026-10-20

- Home-Assistant-Seitenpanel **v2.2.0** zeigt Vorlauf, Rücklauf und Spreizung (Kelvin) mit frei wählbaren Sensor-Entitäten.
- Gemeinsamer Heizkreisverlauf mit linker °C- und rechter K-Achse und 24-h-/7-/30-Tage-Zeiträumen; bisheriger Sechs-Slot-Verlauf bleibt erhalten.
- Verwendet optional einen bestehenden HA-Spreizungs-Helfer, sonst zeitlich abgeglichene Recorder-Werte der Temperaturfühler; keine neuen HA-Packages nötig.
- Python-Registrierung und Manifest auf v2.2.0, mit neuer Cache-Busting-URL; Anleitung und Projektstatus ergänzt.
- Keine Änderung an Wemos v1.9.6, HA-Packages v2.1.0 oder Lovelace-Karte v2.1.1.


## GitHub-Projektpaket 0.1.5 — 2026-10-19

- **Lovelace-Karte v2.1.1**: Eine alte Karten-YAML-Angabe `entity:` kann nicht mehr die aktuelle Zuordnung `entities.total` überschreiben; Altwert nur noch als Fallback, wenn kein moderner Gesamtbestandssensor vorhanden ist.
- Anleitung für den Austausch von `www/heizoel-monitor-card.js`, Cache-Busting `?v=211` und die explizite Entitätszuordnung ergänzt.
- Wemos-Firmware v1.9.6, HA-Packages und eigenständiges Seitenleistenpanel v2.1.0 unverändert.


## GitHub-Projektpaket 0.1.4 — 2026-10-19

- `PROJECT_STATUS.md` als verbindlichen Überblick über zuletzt berichteten Hardware-/Softwarestand, Pinbelegung, Home-Assistant-Entitäten, offene Probleme und Update-Workflow angelegt.
- README Deutsch/Englisch mit der Statusdatei verlinkt und einen veralteten MakerWorld-Platzhalter in der deutschen README korrigiert.
- Keine funktionalen Änderungen: Firmware weiterhin v1.9.6, Home Assistant weiterhin v2.1.0.


## GitHub-Projektpaket 0.1.3 — 2026-10-19

- Das veröffentlichte 3D-Druck-Gehäuse mit MakerWorld-Modell und Druckprofil in den deutschen und englischen Projektdokumenten verlinkt.
- Veraltete „bald verfügbar“-Hinweise zum Gehäuse aktualisiert.
- Firmware v1.9.6 und Home Assistant v2.1.0 unverändert.


## GitHub-Projektpaket 0.1.2 — 2026-10-19

- Elf eigene, auf GitHub für die Dokumentation verkleinerte Fotos/Screenshots ergänzt; EXIF-Metadaten entfernt.
- Bildergalerie mit Gehäuse, Rohrhalterung, Einbau, Sensorkappe, CAD und Home Assistant eingefügt.
- MakerWorld-Veröffentlichungstexte in Deutsch und Englisch vorbereitet; noch keine Modelldateien.
- Firmware und Home-Assistant-Integration unverändert.


## GitHub-Projektpaket 0.1.1 — 2026-10-19

- Öffentliches GitHub-Projekt mit vorbereiteter MIT-Lizenz, Beitragshinweisen und Sicherheitsmeldungen.
- Deutsche und englische README-Texte auf Lizenz und Veröffentlichung aktualisiert.
- MakerWorld-Bereich bleibt ohne Modelle oder Gehäusebilder als Platzhalter bestehen.
- Kein Funktionsupdate der Firmware (weiterhin v1.9.6) oder des HA-Pakets (v2.1.0).

## GitHub-Projektpaket 0.1.0 — 2026-10-19

- Vollständige Verzeichnisstruktur aus Firmware v1.9.6 und HA-Dashboard-Paket v2.1.0 erzeugt.
- Im Arduino-Sketch eingebettete Zugangsdaten **ersetzt durch Kompilier-Platzhalter**.
- Bestätigten HA-Quellnamen `abstand_geglattet` in YAML, JS, Panel und Beispielen vereinheitlicht.
- Bauanleitung, BOM, Anschlussplan und reduzierte Fotos der Prototyp-Platine ergänzt.
- MakerWorld-/3D-Gehäuse-Vorbereitung ergänzt, 3MF/STL noch ausstehend.
- Separate Lizenzwahl: derzeit keine neue Lizenzbehauptung für vorhandene Dateien.

## Enthaltene Firmware-Historie

- v1.8.5 — 16-Punkte-Höhen/Liter-Kennlinie und reorganisierte Weboberfläche.
- v1.9.2 — sechs logische DS18B20-Slots, MQTT-Relaissteuerung optional.
- v1.9.3 — frei einstellbarer Anzeigename.
- v1.9.4 — Relais ohne Seitenneuladen, Statusdiagnostik.
- v1.9.5 — HA-Lite, Langzeitverläufe in Home Assistant ausgelagert.
- v1.9.6 — JavaScript-Statusanzeige korrigiert.
- HA v2.1.0 — Verbrauchs-/Nachtank-Schätzung als optionales Package.
