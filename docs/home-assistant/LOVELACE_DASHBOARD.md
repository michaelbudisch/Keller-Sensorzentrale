# Kellersteuerung – neues natives Lovelace-Dashboard

**Quell-YAML:** [Kellersteuerung_Lovelace.yaml](../../home-assistant/Kellersteuerung_Lovelace.yaml)

Das Dashboard benötigt ausschließlich Standardkarten von Home Assistant, kein HACS und keine eigene `.js`-Ressource. Es umfasst die sieben Bereiche **Übersicht, Heizkreis, Heiztakte, Temperaturen, Öltank, Verbrauch und System**. Es verändert weder den Wemos noch die bestehenden Pakete oder Messwert-Historien.

## Einrichtung A – Dashboard in der Oberfläche anlegen (empfohlen)

1. Unter **Einstellungen → Dashboards** ein leeres Dashboard **Kellersteuerung** anlegen.
2. Dashboard öffnen, **Bearbeiten**, im Menü **Rohkonfigurationseditor / Raw configuration editor** öffnen.
3. Den **gesamten Inhalt** der Datei [Kellersteuerung_Lovelace.yaml](../../home-assistant/Kellersteuerung_Lovelace.yaml) einfügen, bestehende leere Konfiguration ersetzen, speichern.
4. Alle sieben Tabs überprüfen. Andere Dashboards, das bisherige Panel und die alte Custom-Karte bleiben bestehen. Du brauchst dafür keine Änderung an `configuration.yaml`.

## Einrichtung B – Versionierte YAML-Datei

Die Datei ins Verzeichnis der aktiven `configuration.yaml` kopieren (im dokumentierten Aufbau meist `/usr/share/hassio/homeassistant/`). Besteht `lovelace:` bereits, nur den `dashboards:`-Unterblock ergänzen:

```yaml
lovelace:
  dashboards:
    keller-lovelace:
      mode: yaml
      filename: Kellersteuerung_Lovelace.yaml
      title: Kellersteuerung
      icon: mdi:home-thermometer
      show_in_sidebar: true
```

Keine zweite `lovelace:`-Konfiguration anlegen, bestehende Ressourcen-Einstellungen beibehalten. Konfiguration prüfen, bei Änderung an `configuration.yaml` HA neu starten. Bei späteren YAML-Änderungen im Dashboardmenü **Refresh / Aktualisieren**.

## Warum der Heizkreis jetzt besser ablesbar ist

- Der native **history-graph** zeigt kompakte **6 Stunden** Vorlauf/Rücklauf bzw. Spreizung, ohne 24 Stunden stark zu verdichten.
- Die Karte **energy-date-selection** steuert die beiden **statistics-graph**-Karten auf derselben Heizkreis-Seite. Per Kalender sind einzelne Tage und größere Zeiträume wählbar.
- Beide Einheiten werden in **separaten Graphen** dargestellt (°C und K). Diese Statistiken sind für längere Zeiträume zusammengefasst, statt jeden Rohmesspunkt zu zeigen.
- Über den Knopf **Exakte Messpunkte in der Home-Assistant-Historie** öffnet sich das normale **History**-Panel, wo Datum und Sensoren auswählbar sind. Die vollständige Auflösung hängt von den Recorder-Einstellungen ab.

## Hinweise

- **Alle IDs** sind aus den vorhandenen HA-Paketen und den bereits bestätigten Entitätslisten des Projekts abgeleitet. Benennungen einzelner Entities in deinem HA können abweichen. Bei `Entität nicht gefunden` die entsprechende YAML-Entity ersetzen.
- **6 Temperatur-Slots**: nicht belegte Slots können `unavailable` zeigen.
- **Heiztakt-Statistik**: Temperaturbasiert; kein direkt gemessener Brennerstatus. Bereits gespeicherte frühere Pausen-Ausreißer werden nicht automatisch gelöscht.
- **Tank**: ein gemessener Tank, zweiter Tank hochgerechnet. Mechanische Anzeige zur Plausibilitätskontrolle verwenden.
- **Relais**: Im Dashboard öffnen sie zunächst die Detailansicht und lassen sich nicht versehentlich direkt über die Kachel umschalten. Keine sicherheitsrelevante Heizungssteuerung darüber.
- **Ohne Aufzeichnung keine Historie**: HA Recorder benötigt, Standard-Speicherdauer der detaillierten Historie häufig ca. 10 Tage. Langzeitstatistiken mit geeigneten Zustandsklassen werden länger gehalten.
- **Kein Wemos-Update** nötig und keine neuen Packages.

## Checkliste für ersten Live-Test

- [ ] Alle sieben Tabs sichtbar
- [ ] Vorlauf, Rücklauf und Spreizung zeigen plausible Werte
- [ ] Datumswähler aktualisiert beide Statistikgrafiken
- [ ] Historienknopf öffnet die HA-Verlaufsansicht
- [ ] Tankfüllstand/Liter korrekt und Warnsensoren nicht vertauscht
- [ ] Relais-Kacheln schalten nicht unmittelbar
- [ ] Heiztakt- und Ölverbrauchsstatistik werden unverändert angezeigt

Siehe Home Assistant Dokumentation: [Sections](https://www.home-assistant.io/dashboards/sections/), [Statistics graph](https://www.home-assistant.io/dashboards/statistics-graph/), [Date selection](https://www.home-assistant.io/dashboards/energy/), [History](https://www.home-assistant.io/integrations/history/).
