# Entwicklungsstatus und offene Punkte

## Aus dem bisherigen Aufbau bestätigt

- Wemos D1 mini mit v1.9.6 meldete im lokalen Status Werte, WLAN und MQTT.
- Tankabstand und kurzer Median wurden im HA-Dashboard erfasst.
- HA-Package `keller_sensorzentrale.yaml` wurde geladen und nach Korrektur der ID angezeigt.
- Relais wurden testweise aus Home Assistant angesprochen; keine Langzeitzuverlässigkeit nachgewiesen.
- Mechanische Anzeigen zweier gleichartiger Tanks zeigten unter den Testbedingungen einen ungefähr übereinstimmenden Stand.

## Noch nicht unabhängig geprüft

- Kompilierung des öffentlich sanitisierten Sketches mit frischer Arduino-Bibliotheksumgebung.
- Langzeitbetrieb, Ausfall-/Wiederanlaufverhalten, Versorgung und WLAN/MQTT unter Fehlerbedingungen.
- Pinorientierung **aller JST-Buchsen auf allen Platinenrevisionen** (nicht nur Signalzuordnung).
- Temperaturmessung aller sechs Slots mit real angeschlossenen Fühlern.
- Nachtank-Erkennung und Verbrauchszähler über mehrere reale Heizperioden.
- Erreichbarkeit/Zuverlässigkeit des lokalen HTTP-Webservers unter Last.
- Sicherheit/Eignung von Ultraschallsensor, Gehäuse und Relais für Einbauort und Umweltbedingungen.

## Entscheidungen und Kompatibilität

- 3 DS18B20-Busse; DS18B20-Slots werden über 64-Bit-Adressen zugeordnet, die jeweils zwei Buchsen eines Busses sind nicht einzeln vom GPIO unterscheidbar.
- Tankberechnung der Firmware und der erwartete Home-Assistant-Glättungswert sind *unterschiedliche* Signalverarbeitungsstufen.
- Home-Assistant-Panel-Domain und JS-Dateiname enthalten absichtlich weiter `heizoel` für Bestandsinstallationen.
- Bekannte HA-Quellen-ID (2026-10-19): `sensor.heizungskeller_sensoren_abstand_geglattet` (nicht `geglaettet`).
- Bestehende GPIO-Pinbelegung bleibt gegenüber v1.9.6 unangetastet.
