# Elektrischer Anschlussplan (Signalnetzliste)

Der Plan gibt **elektrische Verbindungen**, nicht die sichtbare Links-rechts-Reihenfolge eines bestimmten Steckers, wieder. Die Steckerkontakte am eigenen Aufbau mit dem Multimeter bestimmen und beschriften. D-Pin-Bezeichnungen beziehen sich auf den **Wemos D1 mini / ESP8266** – nicht auf einen ESP32.

## 1. DS18B20: drei unabhängige 1-Wire-Busse

```text
                 +-- SLOT 1: DATA
D1 / GPIO5 -------+-- SLOT 2: DATA
                 |
3V3 -- R1 5k -----+         R1 = Pull-up nach 3,3 V

                 +-- SLOT 3: DATA
D2 / GPIO4 -------+-- SLOT 4: DATA
                 |
3V3 -- R2 5k -----+

                 +-- SLOT 5: DATA
D5 / GPIO14 ------+-- SLOT 6: DATA
                 |
3V3 -- R3 5k -----+

Wemos 3V3 ------------------- alle 6 × VCC / 3V3
Wemos GND ------------------- alle 6 × GND
```

Es ist **nur ein Pull-up je Bus**, nicht je Buchse nötig. Slot 1 und 2 teilen denselben GPIO und werden anhand der **ROM-Adresse** des Sensors unterschieden, entsprechend Slot 3/4 und 5/6. Die drei DATA-Busse dürfen untereinander nicht verbunden werden.

## 2. 4-Pin-Tankanschluss (elektrisch)

```text
Tank VCC  --------------------- 5V-Versorgung (Herstellerangaben prüfen)
Tank GND  --------------------- GND (gemeinsam mit Wemos)
Tank TX / ECHO ---- R4 5k ----o---- Wemos D6 / GPIO12
                             |
                             R5 6,8k
                             |
                            GND
Tank RX / TRIG --------------- nicht belegt im Autopuls-Modus
```

Bei 5,0 V HIGH-Spannung ergibt R4=5 kΩ / R5=6,8 kΩ rechnerisch **2,88 V** am D6. Der Sensor muss einen zum vorhandenen Sketch passenden Impuls-/ECHO-Ausgang bieten; serielle UART-Ausgabe wäre **nicht** dasselbe. Nicht alle Module mit TX/RX-Aufdruck senden automatisch ECHO-Pulse. Vor der Verdrahtung die konkrete Sensorvariante verifizieren.

**Prototyp-Abweichung:** Drei 5-kΩ-Widerstände als oberer 5k und unterer 10k ergeben nominell 3,33 V (ohne Toleranzreserve), *kein* allgemein sicherer Bauvorschlag. Ein einzelner Serienwiderstand ist kein zuverlässiger 5→3,3-V-Pegelwandler.

## 3. 4-Pin-Relais-Anschluss (elektrisch)

```text
5V-Versorgung ------------- Relais-Modul VCC (nur wenn für 5V vorgesehen)
Wemos GND ---------------- Relais-Modul GND
D7 / GPIO13 -------------- Relais-Modul IN1
D0 / GPIO16 -------------- Relais-Modul IN2
```

Voreinstellung der Firmware: **active LOW** (`RELAIS_AKTIV_LOW = true`). Viele Relais-Boards schalten bei LOW, aber nicht alle; außerdem ist 3,3-V-Logik an manchen 5-V-Relaisboards nicht garantiert. Nur Module mit passenden Pegeln verwenden. Wemos-D0 (GPIO16) hat technische Besonderheiten und ist nicht beliebig durch einen anderen Pin ersetzbar, ohne die Firmware zu ändern.

## 4. Messungen vor dem ersten Einstecken

Bei ausgeschalteter Spannungsversorgung mit dem Multimeter auf Kurzschluss zwischen 3V3/GND und 5V/GND prüfen. Danach im unbelasteten Niederspannungs-Aufbau 3V3 und 5V mit Bezug auf GND messen. Bei allen Sensorbuchsen kontrollieren, ob VCC wirklich 3,3 V ist; am Tank und Relais die tatsächlich beabsichtigte Spannung. Erst dann Fühler oder Relaismodule anstecken. TX/ECHO-Eingang D6 darf nie direkt 5 V sehen.

**Keine Netzspannung, keine Brenner-/Heizungs-Sicherheitskreise und keine Installation unzulässiger Elektronik in einer Heizöltank-/Explosionsschutz-Zone.**
