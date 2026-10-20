# Heizkreis – Temperaturbasierte Taktanalyse (Home Assistant)

**Komponente:** `packages/keller_heiztakte.yaml` v1.0.1 · **Seitenpanel:** v2.3.2 · **Firmware:** unverändert v1.9.6.

## Warum eine eigene Auswertung?

Die Heizungs-Vorlauf- und Rücklauftemperaturen auf Slot 1 und Slot 2 zeigen wiederkehrende schnelle Anstiege des Vorlaufs sowie Temperaturdifferenzen (Spreizung). Daraus lassen sich **ungefähre Temperatur-Aktivphasen** ableiten. Ein echter Brennerstatus oder ein Pumpensignal wurde nicht angeschlossen – der Status und seine Laufzeiten sind daher **keine exakten Brennerlaufzeiten**, keine Betriebssicherheitsfunktion und keine Heizungsfehlerdiagnose.

Die bisherige Kellersteuerung behält ihre Rollen:
- **Übersicht:** Relais und sechs allgemeine Temperatur-Slots.
- **Heizung & Takte:** Vorlauf, Rücklauf, Spreizung, Heizkreis-Verlauf und Taktanalyse.
- **Temperatur-Slots:** sechs frei benennbare Sensoren und deren Verlauf.
- **Tank & Verlauf:** Tankbestand und Tankhistorie.
- **Ölverbrauch:** separat berechnete Ölverbrauchsschätzung.

## Installation

1. Die bestehenden Dateien und Helfer in HA sichern. **Nicht** vorhandene Ölverbrauchszähler löschen, umbenennen oder zurücksetzen.
2. `home-assistant/packages/keller_heiztakte.yaml` nach **`/config/packages/keller_heiztakte.yaml`** kopieren; `/config` meint das Verzeichnis der tatsächlich aktiven `configuration.yaml`. `homeassistant: packages: !include_dir_named packages` muss bereits eingerichtet sein.
3. Paneldateien `home-assistant/custom_components/heizoel_dashboard_panel/{__init__.py,manifest.json,frontend/heizoel-panel.js}` nach `/config/custom_components/heizoel_dashboard_panel/` aktualisieren. Das bestehende Panel nicht doppelt registrieren.
4. **Entwicklerwerkzeuge → YAML → Konfiguration prüfen**, anschließend Home Assistant vollständig neu starten. Panel neu öffnen oder Strg+F5. Im Footer steht v2.3.2; das Python-Modul verwendet Cache-Busting `?v=2_3_2`.
5. **Heizung & Takte** öffnen: Im Block Heiztakt-Analyse **„Auswertung aktivieren“** klicken oder `input_boolean.keller_heizkreis_auswertung_aktiv` einschalten. **Ab diesem Zeitpunkt** werden neu erkannte Aktivphasen gezählt; frühere historische Peaks führen nicht zu genauen alten Starts.
6. Mindestens 24–72 Stunden messen und Schwellwerte / Mindestzeiten mit den Kurven abgleichen, bevor du die Taktbewertung interpretierst.

Die Vor- und Rücklaufsensoren sind in diesem Package **fest auf** `sensor.heizungskeller_sensoren_slot_1` und `sensor.heizungskeller_sensoren_slot_2` eingestellt. Die Panel-Zuordnung unter **Einstellungen** betrifft nur die Darstellung. Für andere Anlagen bitte **auch im YAML-Package** die beiden Quell-IDs ändern.

## Erkennungslogik

- Temperatursensor: `sensor.keller_heizkreis_spreizung` = Vorlauf minus Rücklauf in Kelvin; bei fehlenden Eingangswerten unavailable.
- Threshold-Sensor: `binary_sensor.keller_heizkreis_schwelle` mit `upper: 7` und `hysteresis: 1.5`. Schaltet bei ansteigender Spreizung **über 8,5 K EIN**, bei fallender **unter 5,5 K AUS**.
- Entprellte Erkennung: `binary_sensor.keller_heizkreis_aktiv` mit `delay_on: 00:00:30`, `delay_off: 00:00:45`. Nur wenn Freigabe und beide Temperaturen vorhanden sind.
- Bei echten Zustandswechseln **off → on** zählt `counter.keller_heizkreis_starts_gesamt` einen neu erkannten Start. Nur vollständige **on → off**-Phasen werden zur mittleren Laufzeit addiert, vollständige **off → on**-Pausen zur mittleren Pausenzeit.
- Datenlücken/Neustarts werden **nicht** als eindeutig abgeschlossene Aktivphasen gezählt. Der Zähler wird von Home Assistant über Neustarts wiederhergestellt; einzelne Phasen rund um Neustarts können fehlen. Kurzzeitige Zeitberechnungen werden auf 24 h gedeckelt.
- Die Einschätzung „wenige/mehrere/viele Zyklen“ basiert auf der Anzahl erkannter Aktivphasen der letzten 60 Minuten (Grenzen 5/8), ist **kein normierter Fehler- oder Grenzwert**. Bei Anlagen mit anderer Temperaturdynamik sind die Werte neu zu prüfen.

**Wichtig:** Ein langes temperaturbedingtes „Aktiv“-Intervall kann weiterbestehen, obwohl der Brenner bereits abgeschaltet hat. Die erfassten Zeiten sind auf die Erkennungsregel bezogen.

## Die neuen Entitäten

| Home Assistant | Funktion |
| --- | --- |
| `input_boolean.keller_heizkreis_auswertung_aktiv` | Taktanalyse ein/aus |
| `binary_sensor.keller_heizkreis_aktiv` | geschätzte Aktivphase |
| `sensor.keller_heizkreis_spreizung` | Differenz in K |
| `sensor.keller_heizkreis_starts_heute`, `sensor.keller_heizkreis_starts_woche` | gezählte neue Aktivphasen seit Aktivierung im jeweiligen Kalenderzeitraum |
| `sensor.keller_heizkreis_starts_letzte_stunde`, `sensor.keller_heizkreis_starts_24h` | via Recorder erkannte on-Zeiträume im gleitenden Fenster; **können eine am Fensteranfang bereits aktive Phase mitzählen** |
| `sensor.keller_heizkreis_laufzeit_heute`, `sensor.keller_heizkreis_pause_heute` | erfasste Aktiv-/Pausenzeit des aktuellen Tages in Stunden |
| `sensor.keller_heizkreis_laufzeit_durchschnitt`, `sensor.keller_heizkreis_pause_durchschnitt` | Mittelwert vollständig erfasster Intervalle (seit Start der Erfassung) |
| `input_number.keller_heizkreis_{laufzeit,pause}_{letzte,min,max}` | Dauer letzter, kürzester, längster vollständig erfasster Intervalle in Minuten |
| `sensor.keller_heizkreis_{vorlauf,ruecklauf,spreizung}_mittel_24h` | statistische Stichprobenmittel der letzten 24 h |
| `sensor.keller_heizkreis_vorlauf_max_24h`, `sensor.keller_heizkreis_ruecklauf_min_24h`, `sensor.keller_heizkreis_spreizung_max_24h` | Extremwerte über 24 h |
| `sensor.keller_heizkreis_taktbewertung` | rein beschreibende Häufigkeits-Kategorie |

**Hinweis zur Recorder-Aufbewahrung:** Das neue Package aktiviert weder Recorder noch verlängert es seine Aufbewahrungsdauer. Für 30-Tage-Diagramme muss die Historie ausreichend lange gespeichert werden. **Übliche Minuten-/Stundenwerte werden nicht nachträglich rekonstruiert, wenn Home Assistant währenddessen nicht lief.**

## Live-Test-Korrektur v1.0.1 / Panel v2.3.1

Die erste Version der Taktanalyse (v1.0.0) konnte aus einem alten `input_datetime.keller_heizkreis_pause_beginn` eine falsche erste Pause berechnen (im Test: **1.137,7 Minuten**). Ab v1.0.1 sorgen zwei beim HA-Neustart und beim Ein-/Ausschalten zurückgesetzte Gültigkeitsmarker dafür, dass Pausendauern erst nach einer vollständig beobachteten Aktivphase berücksichtigt werden. Vor der ersten gültigen Phase wird keine Pausenstatistik berechnet.

Die bereits erfasste falsche Pause wird **nicht automatisch gelöscht**. Falls du sie gezielt entfernen möchtest: Nach einem Backup in **Home Assistant → Entwicklerwerkzeuge → Aktionen** die Aktion `input_number.set_value` mit folgenden Parametern ausführen:

```yaml
action: input_number.set_value
target:
  entity_id:
    - input_number.keller_heizkreis_pause_letzte
    - input_number.keller_heizkreis_pause_min
    - input_number.keller_heizkreis_pause_max
    - input_number.keller_heizkreis_pause_summe
    - input_number.keller_heizkreis_pause_anzahl
data:
  value: 0
```

Das betrifft nur die fünf Pausen-Helfer, **nicht** die Aktivzeit-/Start-Zähler oder die Ölverbrauchsauswertung. Bereits aufgezeichnete Recorder-Verläufe werden nicht gelöscht. Nur zurücksetzen, wenn du die bisherige Pausenstatistik tatsächlich verwerfen willst.

**24h-Sensoren leer (bis Panel v2.3.1):** Die Statistik-Sensoren werden aus ihrem **Anzeigenamen**, nicht aus ihrer `unique_id`, angelegt. Die bisherigen Versionen erkannten nicht alle Varianten: `ruecklauf` wird zu `rucklauf`, `max` zu `maximum`, `min` zu `minimum`. Panel **v2.3.2** unterstützt sowohl die im Live-Test nachgewiesenen IDs als auch ältere Alias-Varianten. Prüfe bei weiter leeren Feldern in HA unter Entwicklerwerkzeuge → Zustände, ob die numerischen Eingangsdaten ankommen. Der HA-Recorder sollte für längere Zeiträume aktiviert sein.

## Bestätigte Entitäts-IDs und neue Gruppierung (Panel v2.3.2)

Bei der realen Installation wurden am **20.10.2026** diese IDs und Zustände bestätigt:

| HA-Entität | Beobachteter Wert |
| --- | ---: |
| `sensor.heizkreis_vorlauf_maximum_24h` | 51,3 °C |
| `sensor.heizkreis_rucklauf_minimum_24h` | 23,6 °C |
| `sensor.heizkreis_rucklauf_mittel_24h` | 34,7 °C |

Weitere erwartete (nach YAML-Namen abgeleitete, noch nicht im Screenshot bestätigte) IDs:
`sensor.heizkreis_vorlauf_mittel_24h`,
`sensor.heizkreis_spreizung_mittel_24h`,
`sensor.heizkreis_spreizung_maximum_24h`.

**Panel v2.3.2** kennt die exakt bestätigten IDs und sinnvolle Alternativen und zeigt die Werte in drei Blöcken an: **Taktung** (Starts), **Lauf- und Pausenzeiten**, **Temperaturstatistik (24 Stunden)**. Fehlende/nicht numerische Daten bleiben als Strich sichtbar. Der Rest des Heizkreisverlaufs, der Tank, das Relais und die Ölverbrauchslogik bleiben unverändert.

**Hinweis zum Statistikzeitraum:** Ein `age_coverage_ratio` von beispielsweise 0,2 bedeutet nur rund 20 % Abdeckung des 24-h-Zeitfensters. Es handelt sich dann noch nicht um ein vollständiges Tagesmaximum oder -mittel, auch wenn die Kachel bereits Daten zeigt. Die Werte werden aussagekräftiger, wenn über einen ganzen Tag aufgezeichnet wurde.

## Test-Checkliste und Troubleshooting

- [ ] `sensor.keller_heizkreis_spreizung` entspricht ungefähr Vorlauf − Rücklauf.
- [ ] Die berechnete Aktivphase wird erst bei mindestens 8,5 K + Mindestdauer aktiviert und bei unter 5,5 K + Mindestdauer deaktiviert.
- [ ] Zähler steigen höchstens einmal pro abgeschlossener AUS→EIN-Erkennung; Test niemals durch absichtliches Schalten realer Heizungskomponenten erzwingen.
- [ ] 24h-Statistik und Recorder führen die Kurven korrekt; schraffierte Aktivphasen stimmen zeitlich ungefähr überein.
- [ ] Vergleiche die Erkennung mit **vorhandenen** offiziellen Brennerlaufzeitzählern, sofern verfügbar, statt aus den Sensorwerten Exaktheit abzuleiten.
- [ ] Beim HA-Neustart werden keine erfundenen Starts addiert; letzte unvollständige Phase kann fehlen.
- [ ] Nach 2–3 Tagen prüfen, ob Spreizungs- und Zeitgrenzen für die konkrete Anlage sinnvoll gewählt sind.

**Fehlende Werte:** Package tatsächlich im richtigen Ordner? Geräte vorab online? Freigabe aktiviert? In HA die Entitäts-IDs durch Suche `keller_heizkreis` überprüfen; HA kann bei bereits existierenden IDs einen Suffix vergeben. Alte Template-Helfer `sensor.heizung_spreizung` dürfen neben dem neuen Package-Sensor bestehen; beide Messwerte müssen aber nicht gleich verzögert sein.

**Datensicherheit:** Das Package ändert keine Relaiszustände, keine Heizungsparameter und keine vorhandene Heizöl-Verbrauchsrechnung. Alle Änderungen sind auf Home Assistant beschränkt.
