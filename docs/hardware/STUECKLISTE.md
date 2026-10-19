# Stückliste – Version mit sechs Sensorbuchsen

| Position | Anzahl | Details / Hinweise |
| --- | ---: | --- |
| Mikrocontroller | 1 | Wemos D1 mini / kompatibler ESP8266 mit beschrifteten D-Pins |
| Lochrasterplatine | 1 | ausreichend für D1 mini, acht Stecker und freie Lötfläche; Abmessungen nach Gehäuse |
| 3-Pin-Buchsen | 6 | passende Steckerserie und Rastermaß bestimmen; nicht nach Foto blind identifizieren |
| 4-Pin-Buchsen | 2 | eine für Tank-Signal, eine für Relais-Modul |
| Pull-ups R1–R3 | 3 × 5 kΩ | je **ein** Pull-up 3V3→DATA für D1, D2, D5 (4,7 kΩ ebenfalls üblich) |
| Tank-Pegelanpassung | 2 | empfohlen: **5 kΩ oben + 6,8 kΩ unten** als Spannungsteiler für 5-V-TX/ECHO; siehe Anschlussplan |
| Temperaturfühler | 0–6 | **DS18B20** (3,3-V-Versorgung, passende 3-adrige Ausführung) |
| Tankgeber | optional | SR04M-2-Variante, Ausgangssignal, Betriebsspannung und Einbauzulassung **vorher prüfen** |
| Relaismodul | optional | 2 Kanäle, Steuersignale **3,3-V-kompatibel**, Versorgung passend; nur Kleinspannungs-Testlasten |
| Verbindungskabel | nach Bedarf | isolierte Litzen / Steckkabel, Schrumpfschlauch, ggf. Stiftleisten/Sockel |
| Schutz/Gehäuse | 1 | 3D-Gehäuse noch einzupflegen; mechanisch entlastete Anschlüsse und geschützte Rückseite |
| Messmittel | 1 | Multimeter für Polaritäts-, Spannungs- und Durchgangsprüfungen |

**Achtung:** Die bestehende Prototyp-Platine hatte für den Tankausgang drei 5-kΩ-Widerstände im Verhältnis 5k:(5k+5k). Das ergibt bei 5 V rechnerisch 3,33 V und lässt keine sichere Toleranzreserve für den 3,3-V-GPIO. Für **neue Nachbauten** ist das deshalb *nicht* die empfohlene Stückliste.
