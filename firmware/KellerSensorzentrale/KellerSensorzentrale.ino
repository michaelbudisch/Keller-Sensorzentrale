/*
  Sensorzentrale v1.9.6 HA-Lite (Statusfix) – v1.8.5 Tankkennlinie + 6 Slots / MQTT-Relais
  Arduino IDE | Board: LOLIN(WEMOS) D1 R2 & mini
  Zusaetzliche Bibliotheken: WiFiManager, PubSubClient, OneWire, DallasTemperature

  Grundlage: funktionierende v1.4 mit selbsttaetigen Pulsen per Interrupt.
  v1.6: Tankanzahl, Gesamtmenge und MQTT/Home Assistant.
  v1.7: Passwortschutz, ArduinoOTA und Web-Firmwareupdate.
  v1.8.2: Stabiler WiFiManager-Boot und WLAN-Wechsel per Heimnetz.
  v1.8.3: Getrennte Web-Reiter; Startseite nur Daten, keine Einstellformulare.
  v1.8.4: LED-WLAN-Diagnose auch waehrend blockierendem WiFiManager.
  v1.8.5: Eigene Hoehe/Liter-Tabelle per Weboberflaeche, ohne feste Tanktyp-Tabelle.
  v1.9.1: DS18B20 und Relais, Mehrpunkt-Kennlinie erhalten.
  v1.9.2: 6 neutrale DS18B20-Slots (2 pro D1/D2/D5), schaltbare MQTT-Relaisfreigabe;
  v1.9.3: Frei waehlbarer Anzeigename fuer Website und MQTT-/HA-Geraeteanzeige;
  v1.9.4: Relais ohne Seitenwechsel schalten, Schaltbegrenzung und Statusdiagnose;
  v1.9.5: HA-Lite: Langzeitpuffer und Diagramme ausgelagert zu Home Assistant;
  v1.9.6: Fehlendes Statusanzeige-Element im Web-Header wiederhergestellt;
  kurze Medianauswertung bleibt vor Ort. MQTT-IDs und EEPROM unveraendert.
  stabile Home-Assistant-IDs und sichere Relais-AUS-Reaktion bei MQTT-Ausfall.
  fuer Home Assistant. Genau ein Sensor bleibt Messgrundlage.
  Keine Sensor-/Tankinstallation: nur Funktions- und Softwaretest.

  Nur fuer einen externen Funktionstest. Der Bastelsensor ist nicht fuer
  den Einbau in einen Heizoeltank freigegeben/nachgewiesen.

  Fuer den Tischtest: 5V und GND zur Sensorplatine; Signalausgang TX/ECHO
  ueber geeignete 5V->3,3V-Pegelanpassung zu D6.
  D5/RX ist in diesem Messmodus kein notwendiger Triggerausgang.
  3V3 am Wemos ist KEIN 5V-toleranter Signaleingang.

  WLAN-Portal: Heizoel-Setup-<chip-id> unter 192.168.4.1
  Dashboard: IP aus dem Heimnetz (kein Port-Forwarding!)
  Web-Update: /firmware, ArduinoOTA: 8266, beide mit Admin-Passwort.
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h> // Bibliothek von Nick O'Leary
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
#include <Ticker.h>  // Mit dem ESP8266-Boardpaket enthalten, keine Zusatzbibliothek
#include <math.h>
#include <string.h>
#include <stddef.h>

// ------------------ Einstellungen ---------------------------------
// === VOR DEM ERSTEN HOCHLADEN LOKALE ZUGANGSDATEN SETZEN ===
// Diese drei Werte sind fest im Sketch kodiert, NICHT im EEPROM gespeichert.
// Das alte Firmware-Update verlangt beim Hochladen die ALTEN Zugangsdaten;
// nach der Installation gelten ausschliesslich die hier eingegebenen Werte.
// Nur die Vorlage ohne echte Zugangsdaten veroeffentlichen.
constexpr char SETUP_AP_PASSWORT[] = "#BITTE_AENDERN";
constexpr char WEB_BENUTZER[]      = "#BITTE_AENDERN";
constexpr char ADMIN_PASSWORT[]    = "#BITTE_AENDERN";
static_assert(SETUP_AP_PASSWORT[0] != '#',
              "Hotspot-Passwort eintragen!");
static_assert(WEB_BENUTZER[0] != '#',
              "Web-Benutzername eintragen!");
static_assert(ADMIN_PASSWORT[0] != '#',
              "Admin-/OTA-Passwort eintragen!");
static_assert(sizeof(SETUP_AP_PASSWORT) >= 9,
              "Hotspot-Passwort braucht mindestens 8 Zeichen!");
// Dasselbe Admin-Passwort gilt fuer Weboberflaeche, Browser-Update und ArduinoOTA.
// Die drei Werte koennen bei Bedarf unterschiedlich sein (SSID-Benutzername getrennt).
const char* VERSION = "1.9.6";
const uint8_t ECHO_PIN = D6; // ECHO/TX des Ultraschallsensors (nur 3,3-V-kompatibles Signal!)
const uint8_t TEMP_PIN_A = D1; // 1-Wire-Bus fuer Slot 1 + 2
const uint8_t TEMP_PIN_B = D2; // 1-Wire-Bus fuer Slot 3 + 4
const uint8_t TEMP_PIN_C = D5; // 1-Wire-Bus fuer Slot 5 + 6
const uint8_t RELAIS_PINS[2] = {D7, D0};
const bool RELAIS_AKTIV_LOW = true; // an Relaismodul anpassen
// MQTT-Relaissteuerung wird per Weboberflaeche bewusst freigegeben.
// Zuerst nur spannungsfreie Tischtests mit dem Relaismodul.

// Sensorabtastung: Jeder vollstaendige Echo-Impuls (KEIN fester Poll-Timer).
// Nur fuer die zeitliche Aufloesung des DIAGRAMMS:
const uint32_t STANDARD_INTERVALL_MS = 1000;
const uint32_t MIN_INTERVALL_MS = 1000;
const uint32_t MAX_INTERVALL_MS = 60000;
const float MIN_ABSTAND_CM = 20.0f;       // Hersteller/Variante pruefen
const float MAX_ABSTAND_CM = 450.0f;
// Nur nach anhaltendem WLAN-Ausfall neu starten; danach nutzt WiFiManager
// die bestehende, bewaehrte Einrichtungsroutine beim Booten.
const uint32_t WLAN_AUSFALL_NEUSTART_MS = 180000UL; // 3 Minuten
const uint32_t KEIN_SIGNAL_MS = 5000;    // Nach 5 s ohne Echo als offline markieren
const float SPRUNG_GRENZE_CM = 35.0f;    // Groessere Einzelspruenge vorerst verwerfen
const float NEUER_ABSTAND_TOLERANZ_CM = 15.0f;
const uint8_t NEUER_ABSTAND_BESTAETIGUNGEN = 3;
// ------------------------------------------------------------------

// Die ISR ist bewusst kurz: Pulsdauer nur in einen Ringpuffer legen.
// IRAM-Attribut ist fuer ESP8266-GPIO-Interrupts erforderlich.
const uint8_t ISR_QUEUE_N = 64; // 2er-Potenz fuer effizienten Ringpuffer
volatile uint32_t isrBreiteUs[ISR_QUEUE_N];
volatile uint32_t isrEndeUs[ISR_QUEUE_N];
volatile uint8_t isrSchreiben = 0;
volatile uint8_t isrLesen = 0;
volatile uint32_t isrVerworfen = 0;
volatile uint32_t isrStartUs = 0;
volatile bool isrPulsAktiv = false;

void ICACHE_RAM_ATTR echoWechselISR() {
  const uint32_t jetztUs = micros();
  if (digitalRead(ECHO_PIN) == HIGH) {
    isrStartUs = jetztUs;
    isrPulsAktiv = true;
  } else if (isrPulsAktiv) {
    isrPulsAktiv = false;
    const uint8_t naechster = (isrSchreiben + 1) & (ISR_QUEUE_N - 1);
    if (naechster == isrLesen) {
      isrVerworfen++;
    } else {
      isrBreiteUs[isrSchreiben] = jetztUs - isrStartUs;
      isrEndeUs[isrSchreiben] = jetztUs;
      isrSchreiben = naechster;
    }
  }
}

bool naechstenPulsLesen(uint32_t &breiteUs, uint32_t &endeUs) {
  noInterrupts();
  if (isrLesen == isrSchreiben) {
    interrupts();
    return false;
  }
  breiteUs = isrBreiteUs[isrLesen];
  endeUs = isrEndeUs[isrLesen];
  isrLesen = (isrLesen + 1) & (ISR_QUEUE_N - 1);
  interrupts();
  return true;
}

uint32_t verworfenePufferImpulse() {
  noInterrupts();
  const uint32_t anzahl = isrVerworfen;
  interrupts();
  return anzahl;
}

ESP8266WebServer webserver(80);
ESP8266HTTPUpdateServer httpUpdater;
bool otaWirdAusgefuehrt = false;
WiFiClient mqttTransport;
PubSubClient mqtt(mqttTransport);
String mqttBasis, mqttOnlineTopic;
uint32_t mqttLetzterVersuchMs = 0;
uint32_t mqttLetzteWerteMs = 0;
bool mqttNeuKonfigurieren = false;
bool mqttWarVerbunden = false;
WiFiManager wifiManager;

// ---------- Temperaturen: eindeutige DS18B20-Adressen dauerhaft speichern ------
OneWire owA(TEMP_PIN_A), owB(TEMP_PIN_B), owC(TEMP_PIN_C);
DallasTemperature dsA(&owA), dsB(&owB), dsC(&owC);
DallasTemperature* const dsBus[3] = {&dsA, &dsB, &dsC};
constexpr uint8_t TEMP_SLOT_ANZAHL = 6;
const uint8_t TEMP_BUS_FUER_STECKER[TEMP_SLOT_ANZAHL] = {0, 0, 1, 1, 2, 2};
// Slots sind logische ROM-Zuordnungen, keine automatisch erkennbaren Buchsen.
const size_t TEMP_EEPROM_OFFSET = 448; // v1.8.5 Tanktabelle bleibt unangetastet
const uint32_t TEMP_MAP_MAGIC_ALT = 0x544D5031UL; // v1.9.1 mit 5 Slots
const uint32_t TEMP_MAP_MAGIC = 0x544D5032UL; // v1.9.2 mit 6 Slots
struct TemperaturZuordnung { uint32_t magic; uint8_t addressen[TEMP_SLOT_ANZAHL][8]; uint32_t pruefsumme; };
struct AlteTemperaturZuordnung { uint32_t magic; uint8_t addressen[5][8]; uint32_t pruefsumme; };
TemperaturZuordnung tempZuordnung;
float temperaturC[TEMP_SLOT_ANZAHL] = {NAN, NAN, NAN, NAN, NAN, NAN};
uint32_t temperaturLetzterErfolgMs[TEMP_SLOT_ANZAHL] = {0,0,0,0,0,0};
// 8 freie EEPROM-Bytes nach dem Sensor-Mapping (bleibt unter 512).
const size_t RELAIS_EEPROM_OFFSET = 504;
const uint32_t RELAIS_CONFIG_MAGIC = 0x524C5932UL;
struct RelaisKonfiguration { uint32_t magic; uint32_t mqttFreigabe; };
RelaisKonfiguration relaisKonfiguration = {RELAIS_CONFIG_MAGIC, 0};
bool mqttRelayFreigabe = false;
uint32_t tempScanMs = 0, tempAnfrageMs = 0;
bool tempKonversionLaeuft = false;

// ---------- Relais: inaktiver Ausgang ist die sichere Einschalt-Vorgabe ------
bool relaisAn[2] = {false, false};
bool relaisStatusMussSenden = false;
// Mechanische Relais nicht durch schnelle EIN-AUS-EIN-Folgen belasten.
// Ausschalten ist immer erlaubt (auch bei MQTT-Ausfall / Sicherheitsabschaltung).
const uint32_t RELAIS_MIN_PAUSE_MS = 900UL;
uint32_t relaisLetzteAenderungMs[2] = {0, 0};
uint32_t relaisSchaltAnzahl[2] = {0, 0};
void relaisSchalten(uint8_t nummer, bool an) {
  if (nummer >= 2) return;
  if (relaisAn[nummer] != an) {
    relaisStatusMussSenden = true;
    relaisLetzteAenderungMs[nummer] = millis();
    relaisSchaltAnzahl[nummer]++;
  }
  relaisAn[nummer] = an;
  digitalWrite(RELAIS_PINS[nummer], (an == RELAIS_AKTIV_LOW) ? LOW : HIGH);
}

// Alle Benutzerbefehle gehen hierdurch, inklusive MQTT.
// Ein AUS-Befehl wird niemals durch die Pause verzoegert.
bool relaisSchaltbefehl(uint8_t nummer, bool an) {
  if (nummer >= 2) return false;
  if (relaisAn[nummer] == an) return true;
  if (an && relaisLetzteAenderungMs[nummer] != 0 &&
      (uint32_t)(millis() - relaisLetzteAenderungMs[nummer]) < RELAIS_MIN_PAUSE_MS) return false;
  relaisSchalten(nummer, an);
  return true;
}

// Board-LED des D1 Mini liegt an GPIO2/D4 und ist LOW-aktiv.
// Der Ticker pulst die LED auch dann, wenn WiFiManager.autoConnect()
// blockierend auf Netzwerk/Einrichtung wartet. KEIN zweiter AP/Webserver.
enum LedWlanStatus : uint8_t { LED_SUCHT=0, LED_ONLINE=1, LED_SETUP=2, LED_UPDATE=3 };
volatile uint8_t ledWlanStatus = LED_SUCHT;
volatile uint8_t ledTickZaehler = 0;
volatile uint8_t ledTestTakte = 0;
Ticker wlanLedTicker;

void ledTickerTick() {
  const uint8_t takt = ledTickZaehler++;
  bool ledAn = false;
  if (ledTestTakte > 0) {
    ledAn = (ledTestTakte % 2) == 0;
    ledTestTakte--;
  } else if (ledWlanStatus == LED_ONLINE) {
    ledAn = true;                          // Dauersignal: verbunden
  } else if (ledWlanStatus == LED_SETUP) {
    const uint8_t phase = takt % 16;
    ledAn = (phase == 0 || phase == 2);   // Doppelblitz alle 1,6 Sekunden
  } else if (ledWlanStatus == LED_UPDATE) {
    ledAn = (takt % 2) == 0;             // Schnelles Blinken waehrend OTA
  } else {
    ledAn = (takt % 10) < 5;            // 0,5 s an / 0,5 s aus: WLAN suchen
  }
  digitalWrite(LED_BUILTIN, ledAn ? LOW : HIGH);
}

void wlanPortalGestartet(WiFiManager *manager) {
  ledWlanStatus = LED_SETUP;
  Serial.println("[WLAN] Einrichtungs-Hotspot aktiv: " + manager->getConfigPortalSSID());
  Serial.println("[WLAN] Einrichtungsadresse: " + WiFi.softAPIP().toString());
}

String geraeteName;  // Technischer Hostname bleibt stabil (auch nach Umbenennung).
String setupWlanName;
String anzeigeName = "Heizungskeller Sensoren";
constexpr size_t ANZEIGENAME_EEPROM_OFFSET = 512;
constexpr size_t ANZEIGENAME_MAX_BYTES = 63;
constexpr uint32_t ANZEIGENAME_MAGIC = 0x484B5331UL; // HKS1
struct AnzeigeNameSpeicher {
  uint32_t magic;
  char wert[ANZEIGENAME_MAX_BYTES + 1];
  uint32_t pruefsumme;
};

uint32_t nameChecksum(const char *buffer, size_t laenge) {
  uint32_t hash=2166136261UL;
  for (size_t i=0;i<laenge;i++) hash=(hash ^ uint8_t(buffer[i])) * 16777619UL;
  return hash;
}

bool anzeigeNameErlaubt(const String& name) {
  if (name.length() == 0 || name.length() > ANZEIGENAME_MAX_BYTES) return false;
  // Leere/control-Zeichen und Zeilenwechsel verhindern; UTF-8-Umlaute erlaubt.
  for (size_t i=0;i<name.length();i++) if (uint8_t(name[i]) < 32 || uint8_t(name[i]) == 127) return false;
  return true;
}

void anzeigeNameLaden() {
  AnzeigeNameSpeicher daten;
  EEPROM.get(ANZEIGENAME_EEPROM_OFFSET, daten);
  if (daten.magic != ANZEIGENAME_MAGIC) return;
  if (!memchr(daten.wert, 0, sizeof(daten.wert))) return;
  if (daten.pruefsumme != nameChecksum(daten.wert, sizeof(daten.wert))) return;
  String name(daten.wert);
  name.trim();
  if (anzeigeNameErlaubt(name)) anzeigeName = name;
}

void anzeigeNameSpeichern(const String& neu) {
  AnzeigeNameSpeicher daten = {};
  daten.magic = ANZEIGENAME_MAGIC;
  neu.toCharArray(daten.wert, sizeof(daten.wert));
  daten.pruefsumme = nameChecksum(daten.wert, sizeof(daten.wert));
  EEPROM.put(ANZEIGENAME_EEPROM_OFFSET, daten);
  EEPROM.commit();
  anzeigeName = neu;
}

bool wlanWechselAngefordert = false;
bool wlanLoeschenAngefordert = false;
bool neustartAngefordert = false;
bool wlanDirektWechselAngefordert = false;
String wlanNeueSSID;
String wlanNeuesPasswort;
bool wlanGetrennt = false;
uint32_t wlanGetrenntSeitMs = 0;

struct SpeicherEinstellungen {
  uint32_t magic;
  uint32_t intervallMs;       // Kurzdiagramm: 1–60 Sekunden
  uint32_t langIntervallMs;   // Langdiagramm: 1–60 Minuten
  uint32_t glattungMinuten;   // 5, 10 oder 15 Minuten
  float kapazitaetL;         // Ein Tank, NICHT Gesamtvolumen
  float abstandVollCm;       // Sensorabstand bei voller Referenz
  float abstandLeerCm;       // Sensorabstand bei leerer Referenz
  float warnSchwelleL;
  uint32_t kalibrierungAktiv; // Erst nach bewusster Einstellung = 1
  uint32_t tankAnzahl;       // 1 bis 8, alle baugleich angenommen
  char mqttHost[64];
  uint16_t mqttPort;
  char mqttUser[40];
  char mqttPass[64];
  uint32_t mqttAktiv;
};
// Exaktes Layout aus v1.5 fuer die automatische Datenuebernahme.
struct AlteV15Einstellungen {
  uint32_t magic, intervallMs, langIntervallMs, glattungMinuten;
  float kapazitaetL, abstandVollCm, abstandLeerCm, warnSchwelleL;
  uint32_t kalibrierungAktiv;
};
SpeicherEinstellungen einstellungen;
const uint32_t EINSTELLUNGEN_MAGIC = 0x1600ACCE;
const uint32_t V15_MAGIC = 0x1500ACCE;
// v1.8/v1.8.1 haengen nur drei WLAN-Timer an das gleiche EEPROM-Praefix.
const uint32_t V18_MAGIC = 0x1800ACCE;
const uint32_t V14_MAGIC = 0x1400ACCE;

// Benutzerdefinierte Kennlinie in separatem EEPROM-Bereich.
// WICHTIG: Layout von SpeicherEinstellungen bleibt bitgenau unveraendert.
// Dadurch bleiben v1.8.4 und die bestehenden Daten OTA-kompatibel.
const size_t TABELLE_OFFSET = 256;
const uint32_t TABELLE_MAGIC = 0x185ACCE1UL;
const uint8_t TABELLE_MAX_PUNKTE = 16;
struct TabellenPunkt {
  float hoeheCm; // Oelstandshoehe ab Bezugspunkt
  float liter;   // Fuellmenge in EINEM Tank
};
struct TabellenKalibrierung {
  uint32_t magic;
  uint8_t aktiv;
  uint8_t anzahl;
  uint16_t reserved;
  float sensorReferenzCm; // Sensorebene bis Bezugspunkt, nicht die Tank-Aussenhoehe
  TabellenPunkt punkte[TABELLE_MAX_PUNKTE];
  uint32_t pruefsumme;
};
static_assert(sizeof(SpeicherEinstellungen) <= TABELLE_OFFSET,
              "EEPROM-Konflikt: Alt-Einstellungen reichen in Tabellenbereich");
static_assert(TABELLE_OFFSET + sizeof(TabellenKalibrierung) <= 512,
              "EEPROM-Konflikt: Tabellenbereich zu gross");
static_assert(TEMP_EEPROM_OFFSET >= TABELLE_OFFSET + sizeof(TabellenKalibrierung),
              "EEPROM-Konflikt: Temperaturzuordnung ueberlappt Tanktabelle");
static_assert(TEMP_EEPROM_OFFSET + sizeof(TemperaturZuordnung) <= RELAIS_EEPROM_OFFSET,
              "EEPROM-Konflikt: 6 Temperatur-Slots ueberlappen Relais-Konfiguration");
static_assert(RELAIS_EEPROM_OFFSET + sizeof(RelaisKonfiguration) <= 512,
              "EEPROM-Konflikt: Relais-Konfiguration ausserhalb des EEPROM");
static_assert(ANZEIGENAME_EEPROM_OFFSET >= RELAIS_EEPROM_OFFSET + sizeof(RelaisKonfiguration),
              "EEPROM-Konflikt: Anzeigename ueberlappt Relais-Daten");
static_assert(ANZEIGENAME_EEPROM_OFFSET + sizeof(AnzeigeNameSpeicher) <= 640,
              "EEPROM-Konflikt: Anzeigename passt nicht in erweiterten Bereich");
TabellenKalibrierung tankTabelle;
bool tankTabelleGueltig = false;

uint32_t tabellenPruefsumme() {
  // FNV-1a ueber alle Daten ausser dem letzten Pruefsummenfeld.
  const uint8_t *bytes = reinterpret_cast<const uint8_t*>(&tankTabelle);
  uint32_t hash = 2166136261UL;
  for (size_t i = 0; i < offsetof(TabellenKalibrierung, pruefsumme); i++) {
    hash ^= bytes[i];
    hash *= 16777619UL;
  }
  return hash;
}

bool tabellenPunktePlausibel() {
  if (tankTabelle.aktiv > 1 || tankTabelle.anzahl < 2 ||
      tankTabelle.anzahl > TABELLE_MAX_PUNKTE ||
      !isfinite(tankTabelle.sensorReferenzCm) ||
      tankTabelle.sensorReferenzCm < 1.0f || tankTabelle.sensorReferenzCm > 450.0f) return false;
  float letzteHoehe = -1.0f;
  float letzteLiter = -1.0f;
  for (uint8_t i = 0; i < tankTabelle.anzahl; i++) {
    const float h = tankTabelle.punkte[i].hoeheCm;
    const float l = tankTabelle.punkte[i].liter;
    if (!isfinite(h) || !isfinite(l) || h < 0 || h > tankTabelle.sensorReferenzCm ||
        l < 0 || l > 100000.0f ||
        (i > 0 && (h <= letzteHoehe || l <= letzteLiter))) return false;
    letzteHoehe = h;
    letzteLiter = l;
  }
  return fabsf(tankTabelle.punkte[0].hoeheCm) <= 0.05f &&
         fabsf(tankTabelle.punkte[0].liter) <= 0.05f;
}

void ladeTankTabelle() {
  memset(&tankTabelle, 0, sizeof(tankTabelle));
  EEPROM.get(TABELLE_OFFSET, tankTabelle);
  tankTabelleGueltig = tankTabelle.magic == TABELLE_MAGIC &&
                        tankTabelle.pruefsumme == tabellenPruefsumme() &&
                        tabellenPunktePlausibel();
  if (!tankTabelleGueltig) {
    memset(&tankTabelle, 0, sizeof(tankTabelle));
    Serial.println("Keine gueltige benutzerdefinierte Kennlinie; lineare Kalibrierung unveraendert.");
  } else {
    Serial.printf("Eigene Kennlinie geladen: %u Punkte (aktiv=%u)\n",
                  (unsigned)tankTabelle.anzahl, (unsigned)tankTabelle.aktiv);
  }
}

bool tabelleIstAktiv() {
  return tankTabelleGueltig && tankTabelle.aktiv == 1 && tabellenPunktePlausibel() &&
         tankTabelle.punkte[tankTabelle.anzahl - 1].liter <= einstellungen.kapazitaetL + 0.05f;
}

float literAusTabelle(float abstandCm) {
  if (!tabelleIstAktiv() || !isfinite(abstandCm)) return NAN;
  const float h = tankTabelle.sensorReferenzCm - abstandCm;
  const TabellenPunkt &erster = tankTabelle.punkte[0];
  const TabellenPunkt &letzter = tankTabelle.punkte[tankTabelle.anzahl-1];
  // Ausserhalb der vom Nutzer gelieferten Tabelle KEINE Extrapolation.
  if (h < erster.hoeheCm - 0.5f || h > letzter.hoeheCm + 0.5f) return NAN;
  if (h <= erster.hoeheCm) return erster.liter;
  if (h >= letzter.hoeheCm) return letzter.liter;
  for (uint8_t i = 1; i < tankTabelle.anzahl; i++) {
    const TabellenPunkt &rechts = tankTabelle.punkte[i];
    if (h <= rechts.hoeheCm) {
      const TabellenPunkt &links = tankTabelle.punkte[i-1];
      const float f = (h - links.hoeheCm) / (rechts.hoeheCm - links.hoeheCm);
      return links.liter + f * (rechts.liter - links.liter);
    }
  }
  return NAN;
}


void standardEinstellungen() {
  einstellungen.magic = EINSTELLUNGEN_MAGIC;
  einstellungen.intervallMs = STANDARD_INTERVALL_MS;
  einstellungen.langIntervallMs = 600000UL; // 10 min => 24 h im Verlauf
  einstellungen.glattungMinuten = 5;
  einstellungen.kapazitaetL = 720.0f;
  einstellungen.abstandVollCm = 30.0f; // Platzhalter: NICHT reale Tankkalibrierung
  einstellungen.abstandLeerCm = 130.0f;
  einstellungen.warnSchwelleL = 100.0f;
  einstellungen.kalibrierungAktiv = 0;
  einstellungen.tankAnzahl = 2;
  einstellungen.mqttHost[0] = '\0';
  einstellungen.mqttPort = 1883;
  einstellungen.mqttUser[0] = '\0';
  einstellungen.mqttPass[0] = '\0';
  einstellungen.mqttAktiv = 0;
}

uint32_t letzterMesszeitpunkt = 0; // Zeit des letzten gespeicherten Diagrammpunkts
uint32_t letzterPulsMs = 0;
uint32_t letzteErfolgreicheMessung = 0;
uint32_t letzterPulsEndeUs = 0;
float sensorIntervallMs = NAN; // tatsaechlicher Abstand zweier Sensor-Pulse
uint32_t messAnzahl = 0;      // alle vollstaendig erfassten Pulse
uint32_t fehlerAnzahl = 0;    // Pulse mit unplausibler Laenge/Abstand
uint32_t ausreisserAnzahl = 0;
float letzterRohwert = NAN;
float letzterMedian = NAN;
bool letzteMessungGueltig = false;
String sensorText = "Warte auf ersten Echo-Impuls";
float kandidatAbstand = NAN;
uint8_t kandidatAnzahl = 0;

const size_t MEDIAN_N = 7;
float medianPuffer[MEDIAN_N];
size_t medianAnzahl = 0;
size_t medianPosition = 0;

// Die Langzeitpuffer (Kurz- und Langdiagramm, 900-Sekunden-Fenster)
// entfallen. Home Assistant speichert/verarbeitet den Verlauf.
// Hier werden nur sieben gueltige Messimpulse fuer den Median gehalten.
uint32_t minFreierHeap = UINT32_MAX;
uint32_t letzteHeapMessungMs = 0;

String htmlEscape(String text) {
  text.replace("&", "&amp;");
  text.replace("<", "&lt;");
  text.replace(">", "&gt;");
  text.replace("\"", "&quot;");
  text.replace("'", "&#39;");
  return text;
}

String jsonEscape(const String& input) {
  String output;
  output.reserve(input.length() + 8);
  for (size_t i = 0; i < input.length(); i++) {
    char c = input[i];
    switch (c) {
      case '\\': output += "\\\\"; break;
      case '"': output += "\\\""; break;
      case '\n': output += "\\n"; break;
      case '\r': output += "\\r"; break;
      case '\t': output += "\\t"; break;
      default: if ((uint8_t)c >= 32) output += c; break;
    }
  }
  return output;
}

String formatLaufzeit() {
  uint32_t sekunden = millis() / 1000UL;
  return String(sekunden / 86400UL) + "d " + String((sekunden / 3600UL) % 24UL) +
         "h " + String((sekunden / 60UL) % 60UL) + "m " + String(sekunden % 60UL) + "s";
}

String jsonZahl(float wert) {
  if (!isfinite(wert)) return "null";
  return String(wert, 1);
}

bool alteKalibrierungGueltig(float kapaz, float voll, float leer, float warn, uint32_t aktiv) {
  return isfinite(kapaz) && kapaz >= 1 && kapaz <= 100000 &&
         isfinite(voll) && voll >= 0 && voll <= 430 &&
         isfinite(leer) && leer <= 450 && leer >= voll + 20 &&
         isfinite(warn) && warn >= 0 && warn <= kapaz && aktiv <= 1;
}

void ladeEinstellungen() {
  EEPROM.begin(640);
  SpeicherEinstellungen neu;
  EEPROM.get(0, neu);
  bool alteFelderOK = neu.intervallMs >= MIN_INTERVALL_MS && neu.intervallMs <= MAX_INTERVALL_MS &&
    neu.langIntervallMs >= 60000UL && neu.langIntervallMs <= 3600000UL &&
    (neu.glattungMinuten == 5 || neu.glattungMinuten == 10 || neu.glattungMinuten == 15) &&
    alteKalibrierungGueltig(neu.kapazitaetL, neu.abstandVollCm,
                           neu.abstandLeerCm, neu.warnSchwelleL, neu.kalibrierungAktiv);
  if ((neu.magic == EINSTELLUNGEN_MAGIC || neu.magic == V18_MAGIC) && alteFelderOK &&
      neu.tankAnzahl >= 1 && neu.tankAnzahl <= 8 &&
      neu.mqttPort > 0 && neu.mqttAktiv <= 1 &&
      memchr(neu.mqttHost, 0, sizeof(neu.mqttHost)) &&
      memchr(neu.mqttUser, 0, sizeof(neu.mqttUser)) &&
      memchr(neu.mqttPass, 0, sizeof(neu.mqttPass))) {
    einstellungen = neu;
    if (neu.magic == V18_MAGIC) {
      einstellungen.magic = EINSTELLUNGEN_MAGIC;
      Serial.println("v1.8-Einstellungen als kompatibles v1.7-Praefix uebernommen.");
    }
    return;
  }
  // Update von v1.5: zuerst neue Defaults anlegen, danach alte Kalibrierung kopieren.
  AlteV15Einstellungen alt;
  EEPROM.get(0, alt);
  standardEinstellungen();
  if (alt.magic == V15_MAGIC &&
      alt.intervallMs >= MIN_INTERVALL_MS && alt.intervallMs <= MAX_INTERVALL_MS &&
      alt.langIntervallMs >= 60000UL && alt.langIntervallMs <= 3600000UL &&
      (alt.glattungMinuten == 5 || alt.glattungMinuten == 10 || alt.glattungMinuten == 15) &&
      alteKalibrierungGueltig(alt.kapazitaetL, alt.abstandVollCm,
                             alt.abstandLeerCm, alt.warnSchwelleL, alt.kalibrierungAktiv)) {
    einstellungen.intervallMs = alt.intervallMs;
    einstellungen.langIntervallMs = alt.langIntervallMs;
    einstellungen.glattungMinuten = alt.glattungMinuten;
    einstellungen.kapazitaetL = alt.kapazitaetL;
    einstellungen.abstandVollCm = alt.abstandVollCm;
    einstellungen.abstandLeerCm = alt.abstandLeerCm;
    einstellungen.warnSchwelleL = alt.warnSchwelleL;
    einstellungen.kalibrierungAktiv = alt.kalibrierungAktiv;
    Serial.println("v1.5-Kalibrierung uebernommen; Tankanzahl=2, MQTT aus.");
  } else if (alt.magic == V14_MAGIC && alt.intervallMs >= MIN_INTERVALL_MS &&
             alt.intervallMs <= MAX_INTERVALL_MS) {
    einstellungen.intervallMs = alt.intervallMs;
  }
  // Neue Einstellungen erst nach Speichern im Browser ins Flash schreiben.
}

// Temperatur-Adresse: Werte der DS18B20 sind ROM-Adressen und kein Messindex.
// Dadurch vertauschen sich die Bezeichnungen bei zeitweisem Ausfall nicht.
uint32_t tempChecksumAdressen(const uint8_t* adressen, size_t byteAnzahl) {
  uint32_t c = 2166136261UL;
  for (size_t i=0;i<byteAnzahl;i++) c = (c ^ adressen[i]) * 16777619UL;
  return c;
}
uint32_t tempChecksum() {
  return tempChecksumAdressen(&tempZuordnung.addressen[0][0], sizeof(tempZuordnung.addressen));
}
void tempZuordnungSpeichern() {
  tempZuordnung.magic = TEMP_MAP_MAGIC;
  tempZuordnung.pruefsumme = tempChecksum();
  EEPROM.put(TEMP_EEPROM_OFFSET, tempZuordnung);
  EEPROM.commit();
}
void tempZuordnungLaden() {
  EEPROM.get(TEMP_EEPROM_OFFSET, tempZuordnung);
  if (tempZuordnung.magic == TEMP_MAP_MAGIC && tempZuordnung.pruefsumme == tempChecksum()) return;
  // Alte 5-Slot-Zuordnung aus v1.9.1 uebernehmen, sechster Slot bleibt frei.
  AlteTemperaturZuordnung alt;
  EEPROM.get(TEMP_EEPROM_OFFSET, alt);
  const bool altOK = alt.magic == TEMP_MAP_MAGIC_ALT &&
    alt.pruefsumme == tempChecksumAdressen(&alt.addressen[0][0], sizeof(alt.addressen));
  memset(tempZuordnung.addressen, 0, sizeof(tempZuordnung.addressen));
  if (altOK) {
    memcpy(tempZuordnung.addressen, alt.addressen, sizeof(alt.addressen));
    Serial.println("Bestehende 5-Slot-Zuordnung uebernommen.");
  }
  tempZuordnungSpeichern();
}
void relaisKonfigurationLaden() {
  EEPROM.get(RELAIS_EEPROM_OFFSET, relaisKonfiguration);
  if (relaisKonfiguration.magic != RELAIS_CONFIG_MAGIC || relaisKonfiguration.mqttFreigabe > 1) {
    relaisKonfiguration.magic = RELAIS_CONFIG_MAGIC;
    relaisKonfiguration.mqttFreigabe = 0;
    EEPROM.put(RELAIS_EEPROM_OFFSET, relaisKonfiguration);
    EEPROM.commit();
  }
  mqttRelayFreigabe = relaisKonfiguration.mqttFreigabe == 1;
}
void relaisKonfigurationSpeichern(bool freigeben) {
  mqttRelayFreigabe = freigeben;
  relaisKonfiguration.magic = RELAIS_CONFIG_MAGIC;
  relaisKonfiguration.mqttFreigabe = freigeben ? 1 : 0;
  EEPROM.put(RELAIS_EEPROM_OFFSET, relaisKonfiguration);
  EEPROM.commit();
}
bool tempAdresseGueltig(const uint8_t* address) {
  for (uint8_t k=0;k<8;k++) if (address[k]) return true;
  return false;
}
String tempAdresseHex(const uint8_t* addr) {
  if (!tempAdresseGueltig(addr)) return "";
  const char hex[] = "0123456789ABCDEF";
  String result; result.reserve(16);
  for(uint8_t k=0;k<8;k++) { result += hex[(addr[k] >> 4) & 15]; result += hex[addr[k] & 15]; }
  return result;
}
bool tempAdresseLesen(const String &s, uint8_t* ziel) {
  if(s.length()!=16) return false;
  for(uint8_t k=0;k<8;k++) {
    int h = isDigit(s[k*2]) ? s[k*2]-'0' : (s[k*2]>='A'&&s[k*2]<='F'?s[k*2]-'A'+10:(s[k*2]>='a'&&s[k*2]<='f'?s[k*2]-'a'+10:-1));
    int l = isDigit(s[k*2+1]) ? s[k*2+1]-'0' : (s[k*2+1]>='A'&&s[k*2+1]<='F'?s[k*2+1]-'A'+10:(s[k*2+1]>='a'&&s[k*2+1]<='f'?s[k*2+1]-'a'+10:-1));
    if(h<0 || l<0) return false;
    ziel[k] = (h<<4)|l;
  }
  return OneWire::crc8(ziel,7)==ziel[7] && ziel[0]==0x28;
}

// ROM-Adressen werden logischen Slots pro Bus zugeordnet. Zwei Buchsen
// an demselben GPIO sind elektrisch identisch: physische Steckposition
// daher nur durch einzelnes Anstecken + manuelle Zuordnung identifizierbar.
void temperaturScannen() {
  bool geaendert=false;
  for(uint8_t b=0;b<3;b++) {
    dsBus[b]->begin(); // neue Sensoren auch nach dem Start erkennen
    dsBus[b]->setWaitForConversion(false);
    uint8_t anzahl = dsBus[b]->getDeviceCount();
    for(uint8_t k=0;k<anzahl && k<12;k++) {
      DeviceAddress addr;
      if(!dsBus[b]->getAddress(addr,k) || addr[0]!=0x28) continue;
      bool zugeordnet=false;
      for(uint8_t s=0;s<TEMP_SLOT_ANZAHL;s++) {
        if(TEMP_BUS_FUER_STECKER[s]==b && memcmp(tempZuordnung.addressen[s],addr,8)==0) { zugeordnet=true; break; }
      }
      if(zugeordnet) continue;
      for(uint8_t s=0;s<TEMP_SLOT_ANZAHL;s++) if(TEMP_BUS_FUER_STECKER[s]==b && !tempAdresseGueltig(tempZuordnung.addressen[s])) {
        memcpy(tempZuordnung.addressen[s],addr,8);
        geaendert=true;
        Serial.printf("Temperatur-Slot %u erkennt: %s\n",(unsigned)(s+1),tempAdresseHex(addr).c_str());
        break;
      }
    }
  }
  if(geaendert) tempZuordnungSpeichern();
  tempScanMs=millis();
}
void temperaturInitialisieren() {
  for(uint8_t b=0;b<3;b++) { dsBus[b]->begin(); dsBus[b]->setWaitForConversion(false); }
  temperaturScannen();
  tempAnfrageMs = millis() - 7000UL;
}
void temperaturAktualisieren() {
  const uint32_t jetzt = millis();
  if((uint32_t)(jetzt-tempScanMs)>=120000UL) temperaturScannen();
  if(!tempKonversionLaeuft) {
    if((uint32_t)(jetzt-tempAnfrageMs) >= 7000UL) {
      for(uint8_t b=0;b<3;b++) dsBus[b]->requestTemperatures();
      tempAnfrageMs=jetzt;
      tempKonversionLaeuft=true;
    }
  } else if((uint32_t)(jetzt-tempAnfrageMs)>=850UL) {
    for(uint8_t s=0;s<TEMP_SLOT_ANZAHL;s++) {
      float wert=NAN;
      if(tempAdresseGueltig(tempZuordnung.addressen[s])) {
        wert=dsBus[TEMP_BUS_FUER_STECKER[s]]->getTempC(tempZuordnung.addressen[s]);
      }
      if(!isfinite(wert) || wert<=-100.0f || wert < -55.0f || wert > 125.0f || wert == 85.0f) {
        temperaturC[s]=NAN;
      } else {
        temperaturC[s]=wert;
        temperaturLetzterErfolgMs[s]=jetzt;
      }
    }
    tempKonversionLaeuft=false;
  }
}

void speichereEinstellungen() {
  EEPROM.put(0, einstellungen);
  EEPROM.commit();
}

bool parseZahl(const String &s, float &wert) {
  if (s.length() == 0 || s.length() > 24) return false;
  char *ende;
  wert = strtof(s.c_str(), &ende);
  return ende != s.c_str() && *ende == '\0' && isfinite(wert);
}

// Browser-Zahlen: Dezimalpunkt und deutsches Komma akzeptieren.
bool parseZahlDE(const String &s, float &wert) {
  String norm = s;
  norm.trim();
  norm.replace(',', '.');
  return parseZahl(norm, wert);
}

float medianBerechnen() {
  if (!medianAnzahl) return NAN;
  float kopie[MEDIAN_N];
  for (size_t i = 0; i < medianAnzahl; i++) kopie[i] = medianPuffer[i];
  for (size_t i = 1; i < medianAnzahl; i++) {
    float wert = kopie[i];
    size_t j = i;
    while (j > 0 && kopie[j-1] > wert) {
      kopie[j] = kopie[j-1];
      j--;
    }
    kopie[j] = wert;
  }
  size_t mitte = medianAnzahl / 2;
  return (medianAnzahl % 2) ? kopie[mitte] : (kopie[mitte-1] + kopie[mitte]) / 2.0f;
}

void medianLeeren() {
  medianAnzahl = 0;
  medianPosition = 0;
  letzterMedian = NAN;
}

void gueltigenAbstandUebernehmen(float abstandCm) {
  letzterRohwert = abstandCm;
  letzteMessungGueltig = true;
  letzteErfolgreicheMessung = millis();
  sensorText = "Messung gueltig";
  medianPuffer[medianPosition] = abstandCm;
  medianPosition = (medianPosition + 1) % MEDIAN_N;
  if (medianAnzahl < MEDIAN_N) medianAnzahl++;
  letzterMedian = medianBerechnen();
}

void impulsVerarbeiten(uint32_t dauerUs, uint32_t endeUs) {
  messAnzahl++;
  letzterPulsMs = millis();
  if (letzterPulsEndeUs != 0) {
    const uint32_t abstandUs = endeUs - letzterPulsEndeUs; // uint32-Ueberlauf sicher
    if (abstandUs >= 10000UL && abstandUs <= 5000000UL) {
      const float neuMs = abstandUs / 1000.0f;
      if (!isfinite(sensorIntervallMs)) sensorIntervallMs = neuMs;
      else sensorIntervallMs = sensorIntervallMs * 0.8f + neuMs * 0.2f;
    }
  }
  letzterPulsEndeUs = endeUs;
  letzteMessungGueltig = false;
  letzterRohwert = NAN;

  // Schutz gegen unplausible Pulse (z.B. Stoerimpulse bei Kabelkontakt).
  if (dauerUs < 300UL || dauerUs > 50000UL) {
    fehlerAnzahl++;
    sensorText = "Impulslaenge unplausibel";
    return;
  }
  const float abstandCm = dauerUs * 0.0343f / 2.0f;
  if (abstandCm < MIN_ABSTAND_CM || abstandCm > MAX_ABSTAND_CM) {
    fehlerAnzahl++;
    sensorText = "Messung ausserhalb des Bereichs";
    return;
  }

  // Einzelne Fehlreflexionen nicht in den Median uebernehmen.
  // Ein echter Positionswechsel wird nach 3 aehnlichen Pulsen akzeptiert.
  if (medianAnzahl >= 5 && fabsf(abstandCm - letzterMedian) > SPRUNG_GRENZE_CM) {
    if (isfinite(kandidatAbstand) &&
        fabsf(abstandCm - kandidatAbstand) <= NEUER_ABSTAND_TOLERANZ_CM) {
      kandidatAnzahl++;
      kandidatAbstand = kandidatAbstand * 0.5f + abstandCm * 0.5f;
    } else {
      kandidatAbstand = abstandCm;
      kandidatAnzahl = 1;
    }
    if (kandidatAnzahl < NEUER_ABSTAND_BESTAETIGUNGEN) {
      ausreisserAnzahl++;
      sensorText = "Einzelsprung vorerst verworfen";
      return;
    }
    // Wiederholter Sprung -> vermutlich veraenderter echter Abstand.
    medianLeeren();
  }
  kandidatAnzahl = 0;
  kandidatAbstand = NAN;
  gueltigenAbstandUebernehmen(abstandCm);
}

// Berechnet den geglaetteten Abstand aus den letzten 5/10/15 Minuten.
// Das ist bewusst ein Zeitfenster und NICHT ein Median aus nur 7 Pulsen.
bool abstandFrisch() {
  return letzteErfolgreicheMessung != 0 &&
         (uint32_t)(millis() - letzteErfolgreicheMessung) < KEIN_SIGNAL_MS;
}

float literFuerAbstand(float cm) {
  if (tabelleIstAktiv()) return literAusTabelle(cm);
  if (einstellungen.kalibrierungAktiv != 1 || !isfinite(cm)) return NAN;
  const float von = einstellungen.abstandVollCm;
  const float bis = einstellungen.abstandLeerCm;
  // Keine falschen Literwerte bei weit ausserhalb der Kalibrierung.
  if (cm < von - 3.0f || cm > bis + 3.0f) return NAN;
  float f = (bis - cm) / (bis - von); // vereinfachte lineare Kennlinie
  if (f < 0) f = 0;
  if (f > 1) f = 1;
  return einstellungen.kapazitaetL * f;
}

// Schneller, lokaler Median ist die Sicherheits-/Plausibilitaetsstufe.
// Zeitfenster und Verbrauchsstatistik laufen in Home Assistant.
float stabileLiter() {
  if (!abstandFrisch() || medianAnzahl < MEDIAN_N || !isfinite(letzterMedian)) return NAN;
  return literFuerAbstand(letzterMedian);
}

void sensorAktualisieren() {
  uint32_t breiteUs, endeUs;
  // Alle bislang eingetroffenen Pulse ohne kuenstliches Messintervall verarbeiten.
  // Bei starker Last in kleinen Bloecken die Eingangsqueue abarbeiten.
  uint8_t verarbeitet = 0;
  while (verarbeitet < 32 && naechstenPulsLesen(breiteUs, endeUs)) {
    impulsVerarbeiten(breiteUs, endeUs);
    verarbeitet++;
  }

  // Wenn kein Signal mehr eintrifft, nicht alte Zahlen als aktuell ausgeben.
  if (letzterPulsMs == 0 || (uint32_t)(millis() - letzterPulsMs) >= KEIN_SIGNAL_MS) {
    letzteMessungGueltig = false;
    letzterRohwert = NAN;
    sensorText = "Kein aktueller Echo-Impuls";
  }

  // Keine Sekunden-/Langzeitpuffer mehr; nur Median und Gueltigkeit.
}

// ----- Home Assistant MQTT Auto-Discovery; ausschliesslich lokales Netzwerk -----
// MQTT-Relaisbefehle sind ausschliesslich im Heimnetz nutzbar.
void mqttBefehl(char* topic, byte* payload, unsigned int laenge) {
  if (!mqttRelayFreigabe || !einstellungen.mqttAktiv || laenge==0 || laenge>8) return;
  String wert;
  for(unsigned int i=0;i<laenge;i++) wert += char(payload[i]);
  if(wert!="ON" && wert!="OFF") return;
  String pfad(topic);
  for(uint8_t i=0;i<2;i++) {
    if(pfad == mqttBasis+"/relay_"+String(i+1)+"/set") {
      relaisSchaltbefehl(i,wert=="ON");
      return;
    }
  }
}

bool discoveryRelais(uint8_t nummer) {
  const String chip=String(ESP.getChipId(),HEX);
  const String id="heizoel_"+chip+"_relay_"+String(nummer+1);
  const String topic="homeassistant/switch/"+id+"/config";
  String cfg="{\"name\":\"Relais "+String(nummer+1)+"\",\"unique_id\":\""+id+"\"";
  cfg+=",\"state_topic\":\""+mqttBasis+"/relay_"+String(nummer+1)+"_state\"";
  cfg+=",\"command_topic\":\""+mqttBasis+"/relay_"+String(nummer+1)+"/set\"";
  cfg+=",\"availability_topic\":\""+mqttOnlineTopic+"\"";
  cfg+=",\"payload_on\":\"ON\",\"payload_off\":\"OFF\"";
  cfg+=",\"device\":{\"identifiers\":[\"heizoel_"+chip+"\"],\"name\":\""+jsonEscape(anzeigeName)+"\",\"manufacturer\":\"DIY\",\"model\":\"Wemos D1 mini\",\"sw_version\":\""+VERSION+"\"}}";
  return mqtt.publish(topic.c_str(),cfg.c_str(),true);
}


void mqttInitialisieren() {
  mqttBasis = "heizoel/" + String(ESP.getChipId(), HEX);
  mqttOnlineTopic = mqttBasis + "/availability";
  mqtt.setServer(einstellungen.mqttHost, einstellungen.mqttPort);
  mqtt.setCallback(mqttBefehl);
  mqtt.setKeepAlive(30);
  mqtt.setSocketTimeout(2);
  if (!mqtt.setBufferSize(1024)) Serial.println("WARNUNG: MQTT-Puffer nicht anlegbar!");
}

// Alle Topics sind durch die Chip-ID eindeutig. HA Discovery enthaelt
// Geraete-ID, so erscheinen mehrere Sensoren gemeinsam an einem Geraet.
bool discoverySensor(const char* key, const char* label, const char* einheit,
                     const char* stateClass, const char* deviceClass, bool binary=false) {
  const String chip = String(ESP.getChipId(), HEX);
  const String id = "heizoel_" + chip + "_" + key;
  const String typ = binary ? "binary_sensor" : "sensor";
  const String topic = "homeassistant/" + typ + "/" + id + "/config";
  const String zustand = mqttBasis + "/" + key;
  String cfg;
  cfg.reserve(700);
  cfg += "{\"name\":\"" + String(label) + "\"";
  cfg += ",\"unique_id\":\"" + id + "\"";
  cfg += ",\"state_topic\":\"" + zustand + "\"";
  cfg += ",\"availability_topic\":\"" + mqttOnlineTopic + "\"";
  if (einheit && *einheit) cfg += ",\"unit_of_measurement\":\"" + String(einheit) + "\"";
  if (stateClass && *stateClass) cfg += ",\"state_class\":\"" + String(stateClass) + "\"";
  if (deviceClass && *deviceClass) cfg += ",\"device_class\":\"" + String(deviceClass) + "\"";
  if (binary) cfg += ",\"payload_on\":\"ON\",\"payload_off\":\"OFF\"";
  cfg += ",\"device\":{\"identifiers\":[\"heizoel_" + chip + "\"]";
  cfg += ",\"name\":\"" + jsonEscape(anzeigeName) + "\",\"manufacturer\":\"DIY\"";
  cfg += ",\"model\":\"Wemos D1 mini / SR04M-2\",\"sw_version\":\"" + String(VERSION) + "\"}}";
  return mqtt.publish(topic.c_str(), cfg.c_str(), true);
}

void mqttDiscoverySenden() {
  bool ok = true;
  ok &= discoverySensor("tank_liters", "Tank einzeln", "L", "measurement", "");
  yield();
  ok &= discoverySensor("estimated_total_liters", "Gesamtbestand geschätzt", "L", "measurement", "");
  yield();
  ok &= discoverySensor("percent", "Füllstand gemessener Tank", "%", "measurement", "");
  yield();
  ok &= discoverySensor("distance_cm", "Abstand (Median)", "cm", "measurement", "distance");
  yield();
  ok &= discoverySensor("signal_dbm", "WLAN-Signal", "dBm", "measurement", "signal_strength");
  yield();
  ok &= discoverySensor("tank_count", "Tankanzahl", "", "", "");
  yield();
  ok &= discoverySensor("low_level", "Füllstand niedrig", "", "", "problem", true);
  yield();
  ok &= discoverySensor("sensor_fault", "Sensorstörung", "", "", "problem", true);
  for (uint8_t i=0;i<TEMP_SLOT_ANZAHL;i++) {
    yield();
    const String key = String("temp_") + String(i+1);
    const String label = "Slot " + String(i+1);
    ok &= discoverySensor(key.c_str(), label.c_str(), "°C", "measurement", "temperature");
  }
  for (uint8_t i=0;i<2;i++) {
    yield();
    if (mqttRelayFreigabe) ok &= discoveryRelais(i);
    else {
      // Alte retained Home-Assistant-Switch-Configs entfernen, wenn gesperrt.
      const String id="heizoel_"+String(ESP.getChipId(),HEX)+"_relay_"+String(i+1);
      const String topic="homeassistant/switch/"+id+"/config";
      ok &= mqtt.publish(topic.c_str(), "", true);
    }
  }
  if (!ok) Serial.println("WARNUNG: MQTT Discovery nicht vollstaendig gesendet.");
}

void mqttWerteSenden() {
  if (!mqtt.connected()) return;
  // 'None' setzt einen Wert in Home Assistant auf unbekannt statt veraltete
  // Messwerte weiter als aktuellen Bestand auszugeben.
  const float einzeln = stabileLiter();
  const bool gueltig = isfinite(einzeln);
  const String t = mqttBasis + "/";
  mqtt.publish((t + "tank_liters").c_str(), gueltig ? String(einzeln, 1).c_str() : "None", true);
  mqtt.publish((t + "estimated_total_liters").c_str(), gueltig ? String(einzeln * einstellungen.tankAnzahl, 1).c_str() : "None", true);
  mqtt.publish((t + "percent").c_str(), gueltig ? String(einzeln / einstellungen.kapazitaetL * 100.0f, 1).c_str() : "None", true);
  mqtt.publish((t + "distance_cm").c_str(), (abstandFrisch() && medianAnzahl == MEDIAN_N) ? String(letzterMedian, 1).c_str() : "None", true);
  mqtt.publish((t + "signal_dbm").c_str(), String(WiFi.RSSI()).c_str(), true);
  mqtt.publish((t + "tank_count").c_str(), String(einstellungen.tankAnzahl).c_str(), true);
  mqtt.publish((t + "low_level").c_str(), !gueltig ? "None" : (einzeln <= einstellungen.warnSchwelleL ? "ON" : "OFF"), true);
  mqtt.publish((t + "sensor_fault").c_str(), abstandFrisch() ? "OFF" : "ON", true);
  for (uint8_t i=0;i<TEMP_SLOT_ANZAHL;i++) {
    const String topic=t+"temp_"+String(i+1);
    mqtt.publish(topic.c_str(),isfinite(temperaturC[i]) ? String(temperaturC[i],1).c_str():"None",true);
  }
  for (uint8_t i=0;i<2;i++) {
    const String topic=t+"relay_"+String(i+1)+"_state";
    mqtt.publish(topic.c_str(),relaisAn[i] ? "ON" : "OFF",true);
  }
}

void mqttAktualisieren() {
  if (mqttNeuKonfigurieren) {
    mqttNeuKonfigurieren = false;
    // Bei Umkonfiguration sicherheitshalber beide Ausgaenge deaktivieren.
    relaisSchalten(0,false); relaisSchalten(1,false);
    if (mqtt.connected()) mqtt.publish(mqttOnlineTopic.c_str(), "offline", true);
    mqtt.disconnect();
    mqttInitialisieren();
    mqttLetzterVersuchMs = 0;
    mqttLetzteWerteMs = 0;
  }
  if (!einstellungen.mqttAktiv || einstellungen.mqttHost[0] == '\0' ||
      WiFi.status() != WL_CONNECTED) {
    if (mqtt.connected() || mqttWarVerbunden) {
      relaisSchalten(0,false); relaisSchalten(1,false);
      if (mqtt.connected()) mqtt.publish(mqttOnlineTopic.c_str(), "offline", true);
      mqtt.disconnect();
      mqttWarVerbunden=false;
    }
    return;
  }
  if (!mqtt.connected()) {
    // MQTT-Verbindungsverlust: keine eingeschalteten Relais stehen lassen.
    // Kein Eingriff in den Web-only-Tischtest, falls MQTT nie verbunden war.
    if (mqttWarVerbunden) { relaisSchalten(0,false); relaisSchalten(1,false); mqttWarVerbunden=false; }
    const uint32_t jetzt = millis();
    if (mqttLetzterVersuchMs && (uint32_t)(jetzt-mqttLetzterVersuchMs) < 10000UL) return;
    mqttLetzterVersuchMs = jetzt;
    const bool login = einstellungen.mqttUser[0] != '\0';
    const bool erfolgreich = login ?
      mqtt.connect(geraeteName.c_str(), einstellungen.mqttUser, einstellungen.mqttPass,
                   mqttOnlineTopic.c_str(), 0, true, "offline") :
      mqtt.connect(geraeteName.c_str(), mqttOnlineTopic.c_str(), 0, true, "offline");
    if (erfolgreich) {
      Serial.println("MQTT verbunden");
      mqttWarVerbunden = true;
      mqtt.publish(mqttOnlineTopic.c_str(), "online", true);
      if (mqttRelayFreigabe) {
        mqtt.subscribe((mqttBasis+"/relay_1/set").c_str());
        mqtt.subscribe((mqttBasis+"/relay_2/set").c_str());
      }
      mqttDiscoverySenden();
      mqttWerteSenden();
      mqttLetzteWerteMs = millis();
    } else {
      Serial.print("MQTT Verbindungsfehler: "); Serial.println(mqtt.state());
    }
    return;
  }
  mqtt.loop();
  if (!mqtt.connected()) {
    if (mqttWarVerbunden) {
      relaisSchalten(0,false); relaisSchalten(1,false);
      mqttWarVerbunden=false;
    }
    return;
  }
  if (relaisStatusMussSenden) {
    relaisStatusMussSenden=false;
    for (uint8_t i=0;i<2;i++) {
      const String topic=mqttBasis+"/relay_"+String(i+1)+"_state";
      mqtt.publish(topic.c_str(),relaisAn[i] ? "ON" : "OFF",true);
    }
  }
  if ((uint32_t)(millis() - mqttLetzteWerteMs) >= 30000UL) {
    mqttLetzteWerteMs = millis();
    mqttWerteSenden();
  }
}

String statusJson() {
  bool verbunden = WiFi.status() == WL_CONNECTED;
  String out;
  out.reserve(1900);
  out += "{\"version\":\"" + String(VERSION) + "\"";
  out += ",\"name\":\"" + jsonEscape(geraeteName) + "\"";
  out += ",\"display_name\":\"" + jsonEscape(anzeigeName) + "\"";
  out += ",\"connected\":" + String(verbunden ? "true" : "false");
  out += ",\"ssid\":\"" + jsonEscape(verbunden ? WiFi.SSID() : "nicht verbunden") + "\"";
  out += ",\"ip\":\"" + (verbunden ? WiFi.localIP().toString() : "-") + "\"";
  out += ",\"rssi\":" + String(verbunden ? WiFi.RSSI() : 0);
  out += ",\"chip_id\":\"" + String(ESP.getChipId(), HEX) + "\"";
  out += ",\"wifi_recovery_seconds\":" + String(WLAN_AUSFALL_NEUSTART_MS / 1000UL);
  out += ",\"wifi_offline_seconds\":" + String(wlanGetrennt ? (millis() - wlanGetrenntSeitMs) / 1000UL : 0UL);
  out += ",\"boot_reason\":\"" + jsonEscape(ESP.getResetReason()) + "\"";
  out += ",\"heap_bytes\":" + String(ESP.getFreeHeap());
  out += ",\"heap_min_bytes\":" + String(minFreierHeap == UINT32_MAX ? ESP.getFreeHeap() : minFreierHeap);
  out += ",\"heap_max_block_bytes\":" + String(ESP.getMaxFreeBlockSize());
  out += ",\"heap_fragmentation_percent\":" + String(ESP.getHeapFragmentation());
  out += ",\"relay_switch_count\":[" + String(relaisSchaltAnzahl[0]) + "," + String(relaisSchaltAnzahl[1]) + "]";
  out += ",\"flash_real_bytes\":" + String(ESP.getFlashChipRealSize());
  out += ",\"ota_free_bytes\":" + String(ESP.getFreeSketchSpace());
  out += ",\"sketch_bytes\":" + String(ESP.getSketchSize());
  out += ",\"uptime_seconds\":" + String(millis() / 1000UL);
  out += ",\"uptime\":\"" + formatLaufzeit() + "\"";
  out += ",\"measurement_mode\":\"autonomous-interrupt\"";
  out += ",\"sensor_valid\":" + String(letzteMessungGueltig ? "true" : "false");
  out += ",\"sensor_state\":\"" + jsonEscape(sensorText) + "\"";
  out += ",\"distance_cm\":" + jsonZahl(letzterRohwert);
  out += ",\"median_cm\":" + jsonZahl(letzterMedian);
  out += ",\"measurement_count\":" + String(messAnzahl);
  out += ",\"error_count\":" + String(fehlerAnzahl);
  out += ",\"outlier_count\":" + String(ausreisserAnzahl);
  out += ",\"queue_dropped\":" + String(verworfenePufferImpulse());
  out += ",\"sensor_interval_ms\":" +
      ((letzterPulsMs != 0) && ((uint32_t)(millis()-letzterPulsMs) < KEIN_SIGNAL_MS) ? jsonZahl(sensorIntervallMs) : String("null"));
  bool sensorOnline = (letzterPulsMs != 0) && ((uint32_t)(millis()-letzterPulsMs) < KEIN_SIGNAL_MS);
  out += ",\"sensor_rate_hz\":" +
      (sensorOnline && isfinite(sensorIntervallMs) && sensorIntervallMs > 0.0f ? jsonZahl(1000.0f/sensorIntervallMs) : String("null"));
  out += ",\"last_pulse_ago_ms\":" + (letzterPulsMs ? String(millis()-letzterPulsMs) : String("null"));
  out += ",\"last_ok_ago_s\":" + (letzteErfolgreicheMessung ? String((millis() - letzteErfolgreicheMessung)/1000UL) : String("null"));
  out += ",\"interval_ms\":" + String(einstellungen.intervallMs); // nur Kurzdiagramm
  out += ",\"long_interval_ms\":" + String(einstellungen.langIntervallMs);
  // Kompatibilitaetsfelder fuer aeltere Clients: ab v1.9.5 nur kurzer Median.
  out += ",\"filter_backend\":\"homeassistant\"";
  out += ",\"smooth_minutes\":0";
  out += ",\"smooth_points\":" + String(medianAnzahl);
  out += ",\"smooth_ready\":" + String((abstandFrisch() && medianAnzahl == MEDIAN_N) ? "true" : "false");
  out += ",\"smoothed_cm\":" + (abstandFrisch() ? jsonZahl(letzterMedian) : String("null"));
  out += ",\"calibrated\":" + String((einstellungen.kalibrierungAktiv == 1 || tabelleIstAktiv()) ? "true" : "false");
  out += ",\"calibration_mode\":\"" + String(tabelleIstAktiv() ? "table" : "linear") + "\"";
  out += ",\"table_saved\":" + String(tankTabelleGueltig ? "true" : "false");
  out += ",\"capacity_l\":" + jsonZahl(einstellungen.kapazitaetL);
  out += ",\"tank_count\":" + String(einstellungen.tankAnzahl);
  out += ",\"total_capacity_l\":" + jsonZahl(einstellungen.kapazitaetL * einstellungen.tankAnzahl);
  out += ",\"full_cm\":" + jsonZahl(einstellungen.abstandVollCm);
  out += ",\"empty_cm\":" + jsonZahl(einstellungen.abstandLeerCm);
  out += ",\"warning_l\":" + jsonZahl(einstellungen.warnSchwelleL);
  out += ",\"provisional_liters\":" + (abstandFrisch() ? jsonZahl(literFuerAbstand(letzterMedian)) : String("null"));
  const float liter = stabileLiter();
  out += ",\"liters\":" + jsonZahl(liter); // Bestehendes Feld: gemessener Tank
  out += ",\"estimated_total_liters\":" + jsonZahl(isfinite(liter) ? liter * einstellungen.tankAnzahl : NAN);
  out += ",\"mqtt_relay_enabled\":" + String(mqttRelayFreigabe ? "true" : "false");
  out += ",\"mqtt_enabled\":" + String(einstellungen.mqttAktiv ? "true" : "false");
  out += ",\"mqtt_connected\":" + String(mqtt.connected() ? "true" : "false");
  out += ",\"mqtt_host\":\"" + jsonEscape(String(einstellungen.mqttHost)) + "\"";
  out += ",\"mqtt_port\":" + String(einstellungen.mqttPort);
  out += ",\"mqtt_user\":\"" + jsonEscape(String(einstellungen.mqttUser)) + "\"";
  out += ",\"percent\":" + (isfinite(liter) ? jsonZahl(liter * 100.0f / einstellungen.kapazitaetL) : String("null"));
  out += ",\"low_level\":" + String(isfinite(liter) && liter <= einstellungen.warnSchwelleL ? "true" : "false");
  out += ",\"temperature_c\":[";
  for (uint8_t i=0;i<TEMP_SLOT_ANZAHL;i++) { if (i) out+=","; out+=jsonZahl(temperaturC[i]); }
  out += "]";
  out += ",\"relay_state\":[" + String(relaisAn[0]?"true":"false") + "," + String(relaisAn[1]?"true":"false") + "]";
  out += "}";
  return out;
}

String temperaturJson() {
  String out="{\"slots\":[";
  for(uint8_t i=0;i<TEMP_SLOT_ANZAHL;i++) {
    if(i) out+=",";
    out+="{\"slot\":"+String(i)+",\"name\":\"Slot "+String(i+1)+"\",\"bus\":"+String(TEMP_BUS_FUER_STECKER[i]+1);
    out+=",\"rom\":\""+tempAdresseHex(tempZuordnung.addressen[i])+"\",\"c\":"+jsonZahl(temperaturC[i])+"}";
  }
  out+="],\"detected\":[";
  bool first=true;
  for(uint8_t b=0;b<3;b++) {
    uint8_t n=dsBus[b]->getDeviceCount();
    for(uint8_t k=0;k<n && k<12;k++) {
      DeviceAddress addr;
      if(!dsBus[b]->getAddress(addr,k)||addr[0]!=0x28) continue;
      if(!first) out+=",";
      first=false;
      out+="{\"bus\":"+String(b+1)+",\"rom\":\""+tempAdresseHex(addr)+"\"}";
    }
  }
  out+="]}";
  return out;
}

String tabellenJson() {
  String out;
  out.reserve(780);
  out += "{\"saved\":" + String(tankTabelleGueltig ? "true" : "false");
  out += ",\"active\":" + String(tabelleIstAktiv() ? "true" : "false");
  out += ",\"reference_cm\":" + (tankTabelleGueltig ? jsonZahl(tankTabelle.sensorReferenzCm) : String("null"));
  out += ",\"points\":[";
  if (tankTabelleGueltig) {
    for (uint8_t i=0; i<tankTabelle.anzahl; i++) {
      if (i) out += ",";
      out += "{\"height_cm\":" + jsonZahl(tankTabelle.punkte[i].hoeheCm);
      out += ",\"liters\":" + jsonZahl(tankTabelle.punkte[i].liter) + "}";
    }
  }
  out += "]}";
  return out;
}

const char UI_HEADER[] PROGMEM = R"HTML(<!doctype html><html lang="de"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Sensorzentrale – Lite</title><style>
:root{color-scheme:dark;font-family:system-ui,Arial,sans-serif}*{box-sizing:border-box}body{margin:0;background:#0b1423;color:#eff5ff}main{max-width:850px;margin:auto;padding:16px 14px 38px}h1{font-size:24px;margin:5px 0}h2{font-size:17px}p{font-size:13px;color:#afc0d8;line-height:1.55}header{margin:12px 0}.sub,.notice,.muted{color:#afc0d8;font-size:12px}
nav{display:flex;gap:7px;overflow-x:auto;margin:15px 0 18px}nav a{white-space:nowrap;text-decoration:none;border-radius:7px;background:#263b57;color:#fff;padding:9px 12px;font-size:12px}nav a.selected{background:#2878db}.panel,.tile{border:1px solid #34445f;background:#172338;border-radius:10px;padding:13px;margin-bottom:12px}.tile{margin:0}.grid,.grid3{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:10px;margin:10px 0}.label{font-size:12px;color:#b1c3da}.value{font-size:20px;font-weight:700;margin-top:6px}.big{font-size:26px}.row{display:flex;gap:10px;align-items:center;margin:9px 0}.field{min-width:0}label{display:block;font-size:13px;margin:10px 0 5px}input,select{width:100%;font-size:15px;padding:9px;border:1px solid #546b88;background:#0c1423;color:#fff;border-radius:7px}input[type=checkbox]{width:auto}button,.action{display:inline-block;text-align:center;text-decoration:none;width:100%;border:0;border-radius:7px;background:#2878db;color:white;padding:10px;font-size:13px;font-weight:700;cursor:pointer}button.secondary{background:#344760}button:disabled{opacity:.55}.relay-actions{display:grid;grid-template-columns:1fr 1fr;gap:8px}.status-note{padding:9px;border:1px solid #354b67;border-radius:8px;font-size:13px}.status-note.error{color:#ffcdcc}a{color:#9bc9ff}footer{margin-top:20px;font-size:12px;color:#adbed1}.tag{color:#a6ebc4}.tag.bad{color:#ffb0b0}form{margin:10px 0}canvas{display:none}@media(max-width:600px){.grid,.grid3{grid-template-columns:1fr}}
</style></head><body><main><header><h1 id="display_name">Sensorzentrale</h1><div class="sub">Wemos D1 Mini · HA-Lite · Firmware v1.9.6</div><span id="connected" class="tag" aria-live="polite"></span></header>
<nav><a href="/">Status</a><a href="/tank">Tankkalibrierung</a><a href="/hardware">Slots / Relais</a><a href="/mqtt/settings">MQTT</a><a href="/network">WLAN</a><a href="/system">System / OTA</a></nav>
)HTML";
const char UI_DASHBOARD[] PROGMEM = R"HTML(<section class="panel"><h2>Aktueller Status</h2><p id="online_state">Lade Status ...</p><div class="grid">
<div class="tile"><div class="label">Tankabstand · Median</div><div class="value" id="distance">–</div></div>
<div class="tile"><div class="label">Geschätzter Gesamtbestand</div><div class="value" id="liters">–</div></div>
<div class="tile"><div class="label">Temperaturen · Slot 1–6</div><div id="temps" class="value" style="font-size:14px;line-height:2">–</div></div>
<div class="tile"><div class="label">Relais 1 / 2</div><div class="value" id="relays">–</div></div>
</div></section><section class="panel"><h2>Verbindungsdiagnose</h2><div class="grid">
<div class="tile"><div class="label">WLAN / MQTT</div><div class="value" id="net">–</div></div>
<div class="tile"><div class="label">RAM aktuell / Minimum</div><div class="value" id="ram">–</div></div>
<div class="tile"><div class="label">Groesster freier Block / Fragmentierung</div><div class="value" id="frag">–</div></div>
<div class="tile"><div class="label">Laufzeit / Neustartursache</div><div class="value" id="boot" style="font-size:14px">–</div></div>
</div><p>Verlauf und Langzeitfilter liegen jetzt in Home Assistant. Hier werden nur Messwerte, Einstellungen und Diagnosen angezeigt.</p><a href="/api/status">Status als JSON</a></section>
<script>
const out=(id,t)=>{const e=document.getElementById(id);if(e)e.textContent=t};
const f=(v,unit='',d=1)=>typeof v==='number'&&Number.isFinite(v)?v.toFixed(d)+unit:'–';
let laden=false;
async function aktualisieren(){if(laden)return;laden=true;
try{const ctrl=new AbortController(),timeout=setTimeout(()=>ctrl.abort(),4000);let r;
try{r=await fetch('/api/status',{cache:'no-store',signal:ctrl.signal})}finally{clearTimeout(timeout)}
if(!r.ok)throw Error('HTTP '+r.status);const s=await r.json();
out('online_state','Status online · letzte Aktualisierung '+new Date().toLocaleTimeString('de-DE'));
out('distance',f(s.median_cm,' cm'));out('liters',f(s.estimated_total_liters,' L'));
out('temps',(s.temperature_c||[]).map((x,i)=>'Slot '+(i+1)+': '+f(x,' °C')).join(' · '));
out('relays',(s.relay_state||[]).map((x,i)=>(i+1)+': '+(x?'EIN':'AUS')).join(' · '));
out('net',f(s.rssi,' dBm',0)+' · '+(s.mqtt_connected?'MQTT OK':'MQTT offline'));
out('ram',Math.round(s.heap_bytes/1024)+' KB / '+Math.round(s.heap_min_bytes/1024)+' KB');
out('frag',Math.round(s.heap_max_block_bytes/1024)+' KB / '+s.heap_fragmentation_percent+' %');
out('boot',s.uptime+' · '+s.boot_reason);
}catch(err){out('online_state','Statusabfrage fehlgeschlagen: '+err.message)}finally{laden=false}}
aktualisieren();setInterval(aktualisieren,15000);
</script>
)HTML";
const char UI_TANK[] PROGMEM = R"HTML(<section class="panel"><h2>Tankverwaltung & Kalibrierung</h2>
<p>Hier konfigurierst du die Anzahl und Kapazität der Tanks sowie die Zuordnung des gemessenen Abstands zum Füllstand.</p>
<div class="status-note">Standard: 2 baugleiche Tanks mit je 720 Litern. Gemessen wird nur ein Tank. Die Gesamtmenge ist eine Schätzung.</div>
<form method="post" action="/calibration"><div class="grid">
<div class="field"><label for="capacity">Kapazität je Tank (L)</label><input type="number" id="capacity" name="capacity" min="1" max="100000" step="0.1" required></div>
<div class="field"><label for="tanks">Tankanzahl</label><input type="number" id="tanks" name="tanks" min="1" max="8" step="1" required></div>
<div class="field"><label for="full">Abstand bei voller Referenz (cm)</label><input type="number" id="full" name="full" min="0" max="430" step="0.1" required></div>
<div class="field"><label for="empty">Abstand bei leerer Referenz (cm)</label><input type="number" id="empty" name="empty" min="20" max="450" step="0.1" required></div>
<div class="field"><label for="warning">Warnschwelle im gemessenen Tank (L)</label><input type="number" id="warning" name="warning" min="0" step="0.1" required></div>
</div><div class="row"><input type="checkbox" id="enable" name="enable" value="1"><label for="enable" style="margin:0">Literberechnung aktivieren</label></div>
<button type="submit">Tank-Einstellungen speichern</button></form>
<p class="notice">Lineare Kalibrierung als einfache Alternative. Diese Werte bleiben auch bei Nutzung einer eigenen Kennlinie gespeichert.</p>
<p class="notice" id="init_note">Lade gespeicherte Werte ...</p></section>
<script>
(async()=>{try{const r=await fetch('/api/status',{cache:'no-store'});if(!r.ok)throw Error('HTTP '+r.status);const s=await r.json();
for(const [id,val] of Object.entries({capacity:s.capacity_l,tanks:s.tank_count,full:s.full_cm,empty:s.empty_cm,warning:s.warning_l}))document.getElementById(id).value=val;
document.getElementById('enable').checked=s.calibrated;document.getElementById('connected').textContent='● Verbunden';document.getElementById('connected').className='tag good';document.getElementById('init_note').textContent='Einstellungen geladen · Kapazität gesamt: '+s.total_capacity_l+' L';}
catch(e){document.getElementById('init_note').textContent='Einstellungen nicht geladen. Verbindung prüfen und Seite erneut öffnen.';document.getElementById('connected').textContent='● Statusfehler';document.getElementById('connected').className='tag bad';}})();
</script>
<section class="panel"><h2>Eigene Füllstandstabelle</h2>
<p>Für jeden Tanktyp individuell: trage <b>Ölstandshöhe über dem Nullpunkt</b> in cm und die zugehörige <b>Literzahl für einen Tank</b> ein. Die Werte lassen sich aus einer Peiltabelle übernehmen. Es gibt keine fest hinterlegte DEHOUST-Tabelle.</p>
<div class="status-note" id="table_status">Lade Kennlinie ...</div>
<form id="table_form" action="/calibration/table" method="post">
<div class="grid"><div class="field"><label for="table_reference">Sensor-Referenzhöhe (cm)</label><input id="table_reference" name="reference" type="number" min="1" max="450" step="0.1" required placeholder="Abstand Sensor bis Tabellen-Nullpunkt"></div>
<div class="field"><label for="table_use">Berechnungsmethode</label><select name="use_table" id="table_use"><option value="0">Lineare Kalibrierung verwenden</option><option value="1">Eigene Füllstandstabelle verwenden</option></select></div></div>
<p class="notice">Sensor-Referenzhöhe = Entfernung zwischen Sensor-Messebene und Höhen-Nullpunkt deiner Tabelle. Nicht einfach die äußere Tankhöhe eintragen.</p>
<div class="grid" style="margin-bottom:7px"><b>Ölstandshöhe (cm)</b><b>Tankinhalt (L)</b></div>
<div id="table_rows"></div>
<input type="hidden" id="table_count" name="count" value="2">
<div class="grid"><button type="button" class="secondary" id="table_add">+ Tabellenpunkt hinzufügen</button><button type="submit">Eigene Tabelle speichern</button></div>
</form>
<p class="notice">2–16 Tabellenpunkte. Erster Punkt: 0 cm = 0 L. Danach müssen Höhe und Literzahl streng ansteigen. Werte außerhalb des erfassten Bereichs werden als „unbekannt“ angezeigt; es wird nicht frei extrapoliert. Der zweite Tank wird weiterhin nur geschätzt.</p>
<p class="notice" id="table_error" role="status"></p>
</section>
<script>
(function(){
const rows=document.getElementById('table_rows');
const note=document.getElementById('table_error');
function addRow(h='',l='') {
 if(rows.children.length>=16)return;
 const row=document.createElement('div');row.className='grid trow';
 const a=document.createElement('input');a.type='number';a.step='0.1';a.min='0';a.max='450';a.required=true;a.placeholder='Höhe in cm';a.value=h;
 const b=document.createElement('input');b.type='number';b.step='0.1';b.min='0';b.required=true;b.placeholder='Liter je Tank';b.value=l;
 const cell=document.createElement('div');cell.style.display='flex';cell.style.gap='6px';cell.append(b);
 const del=document.createElement('button');del.type='button';del.className='secondary';del.textContent='×';del.title='Punkt entfernen';del.style.width='44px';del.onclick=()=>{if(rows.children.length>2)row.remove()};cell.append(del);
 row.append(a,cell);rows.append(row);
}
document.getElementById('table_add').onclick=()=>{addRow();if(rows.children.length>=16)note.textContent='Maximal 16 Punkte';};
document.getElementById('table_form').addEventListener('submit', e=>{
 const r=[...rows.querySelectorAll('.trow')];
 if(r.length<2||r.length>16){e.preventDefault();note.textContent='2 bis 16 Punkte erforderlich';return;}
 r.forEach((row,i)=>{const inp=row.querySelectorAll('input');inp[0].name='h'+i;inp[1].name='l'+i;});
 document.getElementById('table_count').value=String(r.length);
 if(!confirm('Eigene Kennlinie speichern? Die Literanzeige kann sich dadurch ändern.'))e.preventDefault();
});
(async()=>{try{
 const r=await fetch('/api/calibration',{cache:'no-store'});if(!r.ok)throw Error('HTTP '+r.status);
 const v=await r.json();rows.replaceChildren();
 if(v.saved){document.getElementById('table_reference').value=v.reference_cm;document.getElementById('table_use').value=v.active?'1':'0';v.points.forEach(p=>addRow(p.height_cm,p.liters));}
 else {addRow(0,0);addRow();document.getElementById('table_use').value='0';}
 document.getElementById('table_status').textContent=v.saved ? (v.active?'Eigene Tabelle aktiv':'Tabelle gespeichert (lineare Berechnung aktiv)'):'Noch keine eigene Tabelle gespeichert';
}catch(e){addRow(0,0);addRow();note.textContent='Tabelle konnte nicht geladen werden: Verbindung prüfen.';}
})();
})();
</script>
)HTML";
const char UI_FILTER[] PROGMEM = R"HTML(<section class="panel"><h2>Verlauf & Filter</h2><p>Langzeitfilter und Messverlauf laufen jetzt in Home Assistant. Auf dem Wemos bleibt der kurze 7-Puls-Median.</p><a href="/">Zum Status</a></section>)HTML";
const char UI_MQTT[] PROGMEM = R"HTML(<section class="panel"><h2>Home Assistant über MQTT</h2><p>Broker und MQTT Discovery einrichten. Die Messwerte werden dann als Sensoren in Home Assistant angeboten.</p>
<div class="status-note" id="mqtt_state">Lade MQTT-Verbindungsstatus ...</div>
<form method="post" action="/mqtt"><div class="grid">
<div class="field"><label for="mqtt_host">MQTT-Broker (IP oder Hostname)</label><input id="mqtt_host" name="host" maxlength="63" placeholder="192.168.178.10"></div>
<div class="field"><label for="mqtt_port">Port</label><input id="mqtt_port" name="port" type="number" min="1" max="65535" required></div>
<div class="field"><label for="mqtt_user">Benutzername</label><input id="mqtt_user" name="user" maxlength="39" autocomplete="off"></div>
<div class="field"><label for="mqtt_pass">Passwort (leer = unverändert)</label><input id="mqtt_pass" name="password" type="password" maxlength="63" autocomplete="new-password"></div>
</div><div class="row"><input type="checkbox" id="mqtt_enable" name="enable" value="1"><label for="mqtt_enable" style="margin:0">MQTT aktivieren</label></div>
<div class="row"><input type="checkbox" id="relay_mqtt_enable" name="relay_mqtt" value="1"><label for="relay_mqtt_enable" style="margin:0">Relais 1 und 2 als Home-Assistant-Schalter freigeben (Tischtest)</label></div>
<button type="submit">MQTT speichern</button></form>
<p class="notice">Relaisfreigabe startet standardmäßig AUS. Nach Freigabe erscheinen zwei Schalter über MQTT Discovery. Bei Neustart oder MQTT-Ausfall gehen die Ausgänge auf AUS. Unbedingt nur spannungsfrei / mit ungefahrlicher Kleinspannung testen, keine Heizungs- oder Netzspannungssteuerung anschließen. MQTT ohne TLS nur im vertrauenswürdigen Heimnetz.</p>
</section><script>
(async()=>{try{const r=await fetch('/api/status',{cache:'no-store'});if(!r.ok)throw Error();const s=await r.json();
document.getElementById('mqtt_host').value=s.mqtt_host;document.getElementById('mqtt_port').value=s.mqtt_port;
document.getElementById('mqtt_user').value=s.mqtt_user;document.getElementById('mqtt_enable').checked=s.mqtt_enabled;document.getElementById('relay_mqtt_enable').checked=!!s.mqtt_relay_enabled;
document.getElementById('mqtt_state').textContent=!s.mqtt_enabled?'MQTT ist deaktiviert.':s.mqtt_connected?'MQTT ist verbunden.':'MQTT ist aktiviert, aber nicht verbunden.';
document.getElementById('connected').textContent='● Verbunden';document.getElementById('connected').className='tag good';}
catch(e){document.getElementById('mqtt_state').textContent='Status nicht erreichbar';document.getElementById('connected').textContent='● Statusfehler';document.getElementById('connected').className='tag bad';}})();</script>
)HTML";
const char UI_NETWORK[] PROGMEM = R"HTML(<section class="panel"><h2>WLAN-Verbindung</h2><div class="grid3">
<div class="tile"><div class="label">Netzwerkname</div><div class="value" id="ssid">–</div></div>
<div class="tile"><div class="label">IP-Adresse</div><div class="value" id="ip">–</div></div>
<div class="tile"><div class="label">Signalstärke</div><div class="value" id="rssi">–</div></div>
</div><p class="notice">Normalbetrieb ausschließlich über das Heim-WLAN. Kein paralleler Rettungs-Hotspot.</p>
<p class="notice"><b>LED-Diagnose (eingebaute LED):</b> dauerhaft an = WLAN verbunden; langsam blinken = keine WLAN-Verbindung; Doppelblitz = Einrichtungs-Hotspot aktiv; sehr schnelles Blinken = OTA-Update. Die LED bestätigt nur die WLAN-Verbindung, nicht die Erreichbarkeit des Webservers oder MQTT-Brokers.</p>
</section><section class="panel"><h2>WLAN ändern</h2>
<p>Wähle hier direkt ein anderes 2,4-GHz-WLAN, ohne zuvor in den Hotspot zu wechseln. WLAN-Name manuell eingeben, da im Normalbetrieb keine zusätzlichen WLAN-Scans gestartet werden.</p>
<form action="/wifi/change" method="post" onsubmit="return confirm('WLAN wechseln? Die Verbindung wird getrennt; danach die neue IP im Router suchen.')"><div class="grid">
<div class="field"><label for="new_wifi_ssid">Neues WLAN (SSID)</label><input id="new_wifi_ssid" name="ssid" maxlength="32" required autocomplete="off"></div>
<div class="field"><label for="new_wifi_pass">WLAN-Passwort</label><input type="password" id="new_wifi_pass" name="password" maxlength="63" autocomplete="new-password"></div>
</div><button type="submit">WLAN speichern und neu starten</button></form>
<p class="notice">Nach dem Neustart wird der Wemos über das neue WLAN erreichbar. Bei falschem Kennwort sollte WiFiManager beim Booten das Einrichtungs-WLAN bereitstellen. Sichere vorher den bisherigen Sketch.</p>
</section><section class="panel"><h2>WLAN-Reparatur</h2>
<p>Nur verwenden, wenn ein manueller Setup-Hotspot nötig ist.</p>
<form action="/wifi" method="post"><button class="secondary" type="submit">Einrichtungs-Hotspot starten</button></form>
<p class="notice">Bei einem anhaltenden WLAN-Ausfall erfolgt nach drei Minuten ein Neustart. Der WiFiManager übernimmt dann die Wiederverbindung bzw. Einrichtung.</p>
</section>
<script>(async()=>{try{const r=await fetch('/api/status',{cache:'no-store'});if(!r.ok)throw Error();const s=await r.json();document.getElementById('ssid').textContent=s.ssid;document.getElementById('ip').textContent=s.ip;document.getElementById('rssi').textContent=s.rssi+' dBm';document.getElementById('connected').textContent='● Verbunden';document.getElementById('connected').className='tag good';}catch(e){document.getElementById('connected').textContent='● Statusfehler';document.getElementById('connected').className='tag bad';}})();</script>
)HTML";
const char UI_SYSTEM[] PROGMEM = R"HTML(<section class="panel"><h2>Gerätestatus</h2><div class="grid3">
<div class="tile"><div class="label">Technischer Netzwerkname</div><div class="value" id="name">–</div></div>
<div class="tile"><div class="label">IP-Adresse</div><div class="value" id="ip">–</div></div>
<div class="tile"><div class="label">Laufzeit</div><div class="value" id="uptime">–</div></div>
<div class="tile"><div class="label">Freier RAM</div><div class="value" id="heap_bytes">–</div></div>
<div class="tile"><div class="label">Flash-Speicher</div><div class="value" id="flash_total">–</div></div>
<div class="tile"><div class="label">Freier OTA-Bereich</div><div class="value" id="ota_free">–</div></div>
</div><p class="notice">Letzter Boot: <span id="boot_reason">–</span></p></section>
<section class="panel"><h2>Anzeigename</h2>
<p>Hier kannst du den Titel der Oberfläche und den Gerätenamen in Home Assistant anpassen. Sensor-Slots, MQTT-Topics und der technische Netzwerkname bleiben unverändert.</p>
<form method="post" action="/display-name"><label for="display_name_input">Name der Sensorzentrale</label>
<input id="display_name_input" name="display_name" maxlength="48" required placeholder="Heizungskeller Sensoren">
<button type="submit">Anzeigename speichern</button></form>
<p class="notice" id="display_name_hint">Wird dauerhaft gespeichert. Bereits in Home Assistant manuell vergebene Entitätsnamen bleiben dort bestehen.</p></section>
<section class="panel"><h2>Firmware-Update</h2>
<p>Update über die Arduino IDE (Netzwerk-Port 8266) oder per BIN-Datei über den geschützten Browser-Upload.</p>
<p class="notice">Arduino OTA Host: <code id="ota_host">–</code></p>
<a class="action" href="/firmware">Firmware-Update öffnen</a>
<p class="notice">Nur selbst erstellte, passende ESP8266-Firmware installieren. OTA-Passwort und Web-Passwort entsprechen dem im Sketch festgelegten Admin-Kennwort.</p></section>
<section class="panel"><h2>Geräteaktionen</h2>
<form action="/led" method="post"><button type="submit">LED dreimal blinken lassen (danach wieder WLAN-Status)</button></form>
<form action="/restart" method="post" onsubmit="return confirm('Wemos wirklich neu starten?')"><button class="secondary" type="submit">Neustarten</button></form>
<form action="/reset" method="post" onsubmit="return confirm('Gespeichertes WLAN wirklich löschen?')"><button class="danger" type="submit">WLAN-Zugangsdaten löschen</button></form>
<p class="notice">Achtung: Ein Reset der WLAN-Daten führt beim nächsten Start zur erneuten Einrichtung.</p></section>
<script>(async()=>{try{const r=await fetch('/api/status',{cache:'no-store'});if(!r.ok)throw Error();const s=await r.json();
const fmt=v=>Math.round(v/1024)+' KB';for(const [k,v] of Object.entries({name:s.name,ip:s.ip,uptime:s.uptime,heap_bytes:s.heap_bytes+' Bytes',flash_total:fmt(s.flash_real_bytes),ota_free:fmt(s.ota_free_bytes),boot_reason:s.boot_reason,ota_host:s.name+'.local'}))document.getElementById(k).textContent=v;
document.getElementById('display_name_input').value=s.display_name||'Heizungskeller Sensoren';
document.getElementById('connected').textContent='● Verbunden';document.getElementById('connected').className='tag good';}
catch(e){document.getElementById('connected').textContent='● Statusfehler';document.getElementById('connected').className='tag bad';}})();</script>
)HTML";
const char UI_HARDWARE[] PROGMEM = R"HTML(<section class="panel"><h2>Temperaturfühler</h2>
<p>6 x DS18B20: Slot 1–2 an D1, Slot 3–4 an D2, Slot 5–6 an D5. Pro GPIO teilen sich beide Buchsen dieselbe Datenleitung. Die logische Zuordnung erfolgt über die eindeutige ROM-Adresse. Namen kannst du in Home Assistant selbst ändern.</p>
<div class="status-note" id="tempmsg">Sensoradressen werden gesucht ...</div>
<div id="temp_cards" class="grid"></div>
<p class="notice">Wichtig: Die Firmware kann zwei physische Buchsen am selben GPIO nicht unterscheiden. Für die Zuordnung zunächst jeden Sensor einzeln anstecken und seine ROM-Adresse dem gewünschten Slot zuweisen. Weitere Sensortypen (z. B. DHT22-Feuchtigkeit) erfordern ein anderes Protokoll und eine eigene Firmware-Erweiterung.</p></section>
<section class="panel"><h2>Relais – manuelle Prüfung</h2>
<p>IN1: D7, IN2: D0. Beim Neustart und bei MQTT-Verbindungsverlust werden beide Relais auf AUS gesetzt. Home-Assistant-Schalter lassen sich unter Home Assistant / MQTT ausdrücklich freigeben.</p>
<div class="grid">
<div class="tile"><div class="label">Relais 1 · D7</div><div class="value" id="r1">–</div><div class="grid relay-actions"><button type="button" data-relay-ch="1" data-relay-state="on">EIN</button><button class="secondary" type="button" data-relay-ch="1" data-relay-state="off">AUS</button></div></div>
<div class="tile"><div class="label">Relais 2 · D0</div><div class="value" id="r2">–</div><div class="grid relay-actions"><button type="button" data-relay-ch="2" data-relay-state="on">EIN</button><button class="secondary" type="button" data-relay-ch="2" data-relay-state="off">AUS</button></div></div>
</div><p class="notice" id="relay_message" role="status">Relais nur ohne angeschlossene Last testen. Kein Seitenneuladen. Kurze Schaltpause gegen schnelle Klickfolgen.</p>
</section><script>
const names=['Slot 1','Slot 2','Slot 3','Slot 4','Slot 5','Slot 6'];
const pins=['D1','D1','D2','D2','D5','D5'];
function e(tag,txt){const n=document.createElement(tag);if(txt!==undefined)n.textContent=txt;return n}
async function tempLaden(){
 try{
  const res=await fetch('/api/temperatures',{cache:'no-store'});if(!res.ok)throw Error('HTTP '+res.status);
  const t=await res.json();const cards=document.getElementById('temp_cards');cards.replaceChildren();
  t.slots.forEach(s=>{
   const card=e('div');card.className='tile';
   const head=e('h3',names[s.slot]+' · '+pins[s.slot]);card.append(head);
   const value=e('div',typeof s.c==='number'?s.c.toFixed(1)+' °C':'Nicht erkannt');value.className='value';card.append(value);
   card.append(e('p','Gespeicherte Adresse: '+(s.rom||'keine')));
   const form=e('form');form.method='post';form.action='/temperature/assign';
   const hidden=e('input');hidden.type='hidden';hidden.name='slot';hidden.value=String(s.slot);form.append(hidden);
   const select=e('select');select.name='rom';
   const empty=e('option','Nicht zugeordnet');empty.value='none';select.append(empty);
   t.detected.filter(x=>x.bus===s.bus).forEach(x=>{const opt=e('option',x.rom);opt.value=x.rom;opt.selected=x.rom===s.rom;select.append(opt)});
   if(!t.detected.some(x=>x.bus===s.bus&&x.rom===s.rom))select.value='none';
   form.append(select);const btn=e('button','Zuordnung speichern');btn.type='submit';form.append(btn);card.append(form);cards.append(card);
  });
  document.getElementById('tempmsg').textContent='Sensoren online: '+t.detected.length+' · Zuordnungen gespeichert';
 }catch(err){document.getElementById('tempmsg').textContent='Sensordaten nicht erreichbar: '+err.message;}
}
async function relayLaden(){try{const r=await fetch('/api/status',{cache:'no-store'});if(!r.ok)throw Error();const s=await r.json();
 if(typeof aktualisiereRelaisAnzeige==='function')aktualisiereRelaisAnzeige(s.relay_state);
 else {document.getElementById('r1').textContent=s.relay_state[0]?'EIN':'AUS';document.getElementById('r2').textContent=s.relay_state[1]?'EIN':'AUS';}
}catch(e){}}
tempLaden();relayLaden();setInterval(tempLaden,30000);setInterval(relayLaden,5000);
</script>
)HTML";

const char UI_FOOTER[] PROGMEM = R"HTML(<footer>Sensorzentrale · MQTT ist die dauerhafte Datenverbindung. Messwerte nur zur Orientierung. Keine Freigabe des Webservers im Internet.</footer>
<script>
for(const a of document.querySelectorAll('nav a'))if(a.getAttribute('href')===location.pathname)a.classList.add('selected');
const relayBusy=[false,false],relayButtons=[...document.querySelectorAll('button[data-relay-ch]')];
function aktualisiereRelaisAnzeige(states){if(!Array.isArray(states)||states.length!==2)return;
states.forEach((on,i)=>{const e=document.getElementById('r'+(i+1));if(e)e.textContent=on?'EIN':'AUS';
relayButtons.filter(b=>Number(b.dataset.relayCh)===i+1).forEach(b=>b.setAttribute('aria-pressed',String((b.dataset.relayState==='on')===on)));});}
function relaisMeldung(s){const e=document.getElementById('relay_message');if(e)e.textContent=s;}
async function relaisKlick(ch,an){if(relayBusy[ch-1])return;relayBusy[ch-1]=true;
relayButtons.filter(b=>Number(b.dataset.relayCh)===ch).forEach(b=>b.disabled=true);
try{const ctrl=new AbortController(),timer=setTimeout(()=>ctrl.abort(),4000);let response;
try{response=await fetch('/api/relay',{method:'POST',credentials:'same-origin',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams({ch:String(ch),state:an?'on':'off'}),signal:ctrl.signal})}finally{clearTimeout(timer)}
const obj=await response.json();if(Array.isArray(obj.relay_state))aktualisiereRelaisAnzeige(obj.relay_state);
if(!response.ok)throw Error(obj.message||String(response.status));relaisMeldung('Relais '+ch+' geschaltet');}
catch(e){relaisMeldung('Relais nicht bestaetigt: '+e.message)}
finally{relayBusy[ch-1]=false;relayButtons.filter(b=>Number(b.dataset.relayCh)===ch).forEach(b=>b.disabled=false);}}
relayButtons.forEach(b=>b.addEventListener('click',()=>relaisKlick(Number(b.dataset.relayCh),b.dataset.relayState==='on')));
fetch('/api/status',{cache:'no-store'}).then(r=>r.json()).then(s=>{document.getElementById('display_name').textContent=s.display_name||'Sensorzentrale';document.title=(s.display_name||'Sensorzentrale')+' · v'+s.version}).catch(()=>{});
</script></main></body></html>)HTML";
// Versendet geteilte Seiten direkt aus PROGMEM statt eine riesige HTML-Seite
// erst im knappen ESP8266-RAM zusammenzusetzen.
void webSeite(const char* inhalt) {
  webserver.sendHeader("Cache-Control", "no-store");
  webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
  webserver.send(200, "text/html; charset=utf-8", "");
  webserver.sendContent_P(UI_HEADER);
  webserver.sendContent_P(inhalt);
  webserver.sendContent_P(UI_FOOTER);
  webserver.sendContent("");
}

void ledBlinken(uint8_t anzahl) {
  // Nicht blockieren und nicht gegen den Status-Ticker ansteuern.
  // Drei Impulse = 6 Takte (100 ms AN / 100 ms AUS).
  if (anzahl > 0 && anzahl <= 8) ledTestTakte = anzahl * 2;
}

void zeigeHinweis(const String& titel, const String& beschreibung) {
  String html = "<!doctype html><html lang='de'><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<body style='font-family:system-ui;background:#101827;color:white;max-width:550px;margin:48px auto;padding:16px'>";
  html += "<h2>" + htmlEscape(titel) + "</h2><p>" + htmlEscape(beschreibung) + "</p>";
  html += "<p><a style='color:#9cf' href='/'>Zurück</a></p></body></html>";
  webserver.send(200, "text/html; charset=utf-8", html);
}

// Ein gemeinsamer geschuetzter Zugang fuer alle Web-Routen.
// Achtung: HTTP Basic Auth ist im Heimnetz unverschluesselt (kein HTTPS).
bool pruefeWebAnmeldung() {
  if (webserver.authenticate(WEB_BENUTZER, ADMIN_PASSWORT)) return true;
  webserver.requestAuthentication();
  return false;
}

void registriereWebseiten() {
  webserver.on("/", HTTP_GET, []() {
    if (!pruefeWebAnmeldung()) return;
    webSeite(UI_DASHBOARD);
  });
  webserver.on("/tank", HTTP_GET, [](){ if (!pruefeWebAnmeldung()) return; webSeite(UI_TANK); });
  webserver.on("/hardware", HTTP_GET, [](){ if (!pruefeWebAnmeldung()) return; webSeite(UI_HARDWARE); });
  webserver.on("/api/temperatures", HTTP_GET, [](){
    if (!pruefeWebAnmeldung()) return;
    webserver.sendHeader("Cache-Control", "no-store");
    webserver.send(200, "application/json; charset=utf-8", temperaturJson());
  });
  webserver.on("/temperature/assign", HTTP_POST, [](){
    if (!pruefeWebAnmeldung()) return;
    const String index=webserver.arg("slot"), rom=webserver.arg("rom");
    const int slot=index.toInt();
    if(index!=String(slot) || slot<0 || slot>=TEMP_SLOT_ANZAHL) {webserver.send(400,"text/plain","Stecker ungueltig");return;}
    uint8_t addr[8]={0};
    if(rom!="none" && !tempAdresseLesen(rom,addr)) {webserver.send(400,"text/plain","Sensoradresse ungueltig");return;}
    if(rom!="none") {
      bool vorhanden=false;uint8_t bus=TEMP_BUS_FUER_STECKER[slot];
      for(uint8_t k=0;k<dsBus[bus]->getDeviceCount() && k<12;k++) {
        DeviceAddress found;
        if(dsBus[bus]->getAddress(found,k) && memcmp(addr,found,8)==0) vorhanden=true;
      }
      if(!vorhanden) {webserver.send(400,"text/plain","Sensor am Bus nicht gefunden");return;}
      for(uint8_t s=0;s<TEMP_SLOT_ANZAHL;s++)if(s!=slot && memcmp(tempZuordnung.addressen[s],addr,8)==0) {
        memset(tempZuordnung.addressen[s],0,8);temperaturC[s]=NAN;
      }
    }
    memcpy(tempZuordnung.addressen[slot],addr,8);temperaturC[slot]=NAN;
    tempZuordnungSpeichern();
    webserver.sendHeader("Location", "/hardware", true);
    webserver.send(303,"text/plain","Zuordnung gespeichert");
  });
  webserver.on("/relay", HTTP_POST, [](){
    if (!pruefeWebAnmeldung()) return;
    const String ch=webserver.arg("ch"), state=webserver.arg("state");
    if((ch!="1" && ch!="2") || (state!="on" && state!="off")) {
      webserver.send(400,"text/plain","Relaisanfrage ungueltig");return;
    }
    if(!relaisSchaltbefehl(uint8_t(ch.toInt()-1),state=="on")){
      webserver.send(429,"text/plain; charset=utf-8","Relais-Schaltpause. Bitte kurz warten.");return;
    }
    webserver.sendHeader("Location", "/hardware", true);
    webserver.send(303,"text/plain","Relais geschaltet");
  });
  // Schlanke JSON-API fuer Dashboard/Hardwareseite. Kein 303-Redirect.
  webserver.on("/api/relay", HTTP_POST, [](){
    if(!pruefeWebAnmeldung())return;
    const String ch=webserver.arg("ch"), state=webserver.arg("state");
    webserver.sendHeader("Cache-Control","no-store");
    if((ch!="1"&&ch!="2")||(state!="on"&&state!="off")){
      webserver.send(400,"application/json; charset=utf-8","{\"message\":\"Ungueltige Relaisanfrage\"}");return;
    }
    const uint8_t nr=uint8_t(ch.toInt()-1);
    const bool ok=relaisSchaltbefehl(nr,state=="on");
    String antwort="{\"relay_state\":[";
    antwort+=(relaisAn[0]?"true":"false");antwort+=",";
    antwort+=(relaisAn[1]?"true":"false");antwort+="]";
    if(!ok)antwort+=",\"message\":\"Relais-Schaltpause: kurz warten\"";
    antwort+="}";
    webserver.send(ok?200:429,"application/json; charset=utf-8",antwort);
  });
  webserver.on("/filter", HTTP_GET, [](){ if (!pruefeWebAnmeldung()) return; webSeite(UI_FILTER); });
  webserver.on("/mqtt/settings", HTTP_GET, [](){ if (!pruefeWebAnmeldung()) return; webSeite(UI_MQTT); });
  webserver.on("/network", HTTP_GET, [](){ if (!pruefeWebAnmeldung()) return; webSeite(UI_NETWORK); });
  webserver.on("/system", HTTP_GET, [](){ if (!pruefeWebAnmeldung()) return; webSeite(UI_SYSTEM); });
  webserver.on("/display-name", HTTP_POST, [](){
    if (!pruefeWebAnmeldung()) return;
    if (!webserver.hasArg("display_name")) {
      webserver.send(400,"text/plain; charset=utf-8","Anzeigename fehlt");return;
    }
    String neu=webserver.arg("display_name"); neu.trim();
    if (!anzeigeNameErlaubt(neu)) {
      webserver.send(400,"text/plain; charset=utf-8","Anzeigename muss 1–63 UTF-8-Bytes enthalten und darf keine Steuerzeichen haben.");return;
    }
    if (neu != anzeigeName) {
      anzeigeNameSpeichern(neu);
      if (mqtt.connected()) mqttDiscoverySenden(); // retained HA-Geräteanzeige aktualisieren.
    }
    webserver.sendHeader("Location", "/system", true);
    webserver.send(303,"text/plain","Name gespeichert");
  });
  webserver.on("/api/status", HTTP_GET, []() {
    if (!pruefeWebAnmeldung()) return;
    webserver.sendHeader("Cache-Control", "no-store");
    webserver.send(200, "application/json; charset=utf-8", statusJson());
  });
  webserver.on("/api/calibration", HTTP_GET, []() {
    if (!pruefeWebAnmeldung()) return;
    webserver.sendHeader("Cache-Control", "no-store");
    webserver.send(200, "application/json; charset=utf-8", tabellenJson());
  });
  // Historische JSON-APIs entfallen, damit keine RAM-Puffer mehr benoetigt werden.
  webserver.on("/api/history", HTTP_GET, [](){if(!pruefeWebAnmeldung())return;webserver.send(410,"text/plain","Verlauf jetzt in Home Assistant");});
  webserver.on("/api/history_long", HTTP_GET, [](){if(!pruefeWebAnmeldung())return;webserver.send(410,"text/plain","Langzeitverlauf jetzt in Home Assistant");});
  webserver.on("/settings", HTTP_POST, [](){if(!pruefeWebAnmeldung())return;webserver.send(410,"text/plain","Verlauf-Intervall wird in Home Assistant verwaltet");});
  webserver.on("/calibration", HTTP_POST, []() {
    if (!pruefeWebAnmeldung()) return;
    float kapaz, voll, leer, warn;
    String t = webserver.arg("tanks");
    const int tanks = t.toInt();
    if (t != String(tanks) || tanks < 1 || tanks > 8 ||
        !parseZahl(webserver.arg("capacity"), kapaz) ||
        !parseZahl(webserver.arg("full"), voll) ||
        !parseZahl(webserver.arg("empty"), leer) ||
        !parseZahl(webserver.arg("warning"), warn) ||
        kapaz < 1 || kapaz > 100000 || voll < 0 || voll > 430 ||
        leer < voll + 20 || leer > 450 || warn < 0 || warn > kapaz) {
      webserver.send(400, "text/plain; charset=utf-8", "Kalibrierung ungueltig: Grenzen pruefen (Leerabstand mindestens 20 cm groesser als Vollabstand).");
      return;
    }
    einstellungen.kapazitaetL = kapaz;
    // Bestehende Tabelle bei Kapazitaetswechsel ggf. automatisch deaktivieren.
    // Daten bleiben fuer spaetere Korrektur gespeichert.
    if (tankTabelleGueltig &&
        tankTabelle.punkte[tankTabelle.anzahl-1].liter > kapaz + 0.05f) {
      tankTabelle.aktiv = 0;
      tankTabelle.pruefsumme = tabellenPruefsumme();
      EEPROM.put(TABELLE_OFFSET, tankTabelle);
      tankTabelleGueltig = tabellenPunktePlausibel();
    }
    einstellungen.tankAnzahl = tanks;
    einstellungen.abstandVollCm = voll;
    einstellungen.abstandLeerCm = leer;
    einstellungen.warnSchwelleL = warn;
    einstellungen.kalibrierungAktiv = webserver.hasArg("enable") ? 1 : 0;
    speichereEinstellungen();
    webserver.sendHeader("Location", "/tank", true);
    webserver.send(303, "text/plain; charset=utf-8", "Gespeichert");
  });
  // Eigene Tabelle separat speichern; v1.8.4-Datenstruktur bleibt unberuehrt.
  webserver.on("/calibration/table", HTTP_POST, []() {
    if (!pruefeWebAnmeldung()) return;
    const String countText = webserver.arg("count");
    const int punkte = countText.toInt();
    float referenz;
    if (punkte < 2 || punkte > TABELLE_MAX_PUNKTE || countText != String(punkte) ||
        !parseZahlDE(webserver.arg("reference"), referenz) || referenz < 1 || referenz > 450 ||
        (webserver.arg("use_table") != "0" && webserver.arg("use_table") != "1")) {
      webserver.send(400,"text/plain; charset=utf-8", "Ungueltige Referenzhoehe, Modus oder Punktanzahl.");
      return;
    }
    const TabellenKalibrierung vorher = tankTabelle;
    const bool warGueltig = tankTabelleGueltig;
    memset(&tankTabelle, 0, sizeof(tankTabelle));
    tankTabelle.magic = TABELLE_MAGIC;
    tankTabelle.anzahl = (uint8_t)punkte;
    tankTabelle.aktiv = webserver.arg("use_table") == "1" ? 1 : 0;
    tankTabelle.sensorReferenzCm = referenz;
    bool fehler = false;
    for (int i=0; i<punkte; i++) {
      float h, l;
      if (!parseZahlDE(webserver.arg("h"+String(i)), h) ||
          !parseZahlDE(webserver.arg("l"+String(i)), l)) { fehler = true; break; }
      tankTabelle.punkte[i].hoeheCm = h;
      tankTabelle.punkte[i].liter = l;
    }
    if (fehler || !tabellenPunktePlausibel() ||
        tankTabelle.punkte[tankTabelle.anzahl-1].liter > einstellungen.kapazitaetL + 0.05f) {
      tankTabelle = vorher;
      tankTabelleGueltig = warGueltig;
      webserver.send(400,"text/plain; charset=utf-8",
        "Tabelle ungueltig: erster Wert 0 cm / 0 L, streng steigend, maximal Tankkapazitaet und Referenzhoehe.");
      return;
    }
    tankTabelle.pruefsumme = tabellenPruefsumme();
    EEPROM.put(TABELLE_OFFSET, tankTabelle);
    if (!EEPROM.commit()) {
      tankTabelle = vorher;
      tankTabelleGueltig = warGueltig;
      webserver.send(500,"text/plain; charset=utf-8", "Speichern im Flash fehlgeschlagen.");
      return;
    }
    tankTabelleGueltig = true;
    mqttLetzteWerteMs = 0;
    webserver.sendHeader("Location", "/tank", true);
    webserver.send(303, "text/plain; charset=utf-8", "Tabelle gespeichert");
  });
  webserver.on("/filter", HTTP_POST, [](){if(!pruefeWebAnmeldung())return;webserver.send(410,"text/plain","Langzeitfilter jetzt in Home Assistant");});
  webserver.on("/mqtt", HTTP_POST, []() {
    if (!pruefeWebAnmeldung()) return;
    const String host = webserver.arg("host");
    const String portText = webserver.arg("port");
    const String user = webserver.arg("user");
    const String pass = webserver.arg("password");
    const bool aktiv = webserver.hasArg("enable");
    int port = portText.toInt();
    // Keine Pfadtrenner oder Leerzeichen in Brokeradressen akzeptieren.
    bool hostOK = host.length() < sizeof(einstellungen.mqttHost);
    for (size_t i=0; i<host.length(); i++) {
      char c=host[i];
      if (!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='.'||c=='-'||c==':'||c=='_')) hostOK=false;
    }
    if (!hostOK || (aktiv && host.length() == 0) ||
        portText != String(port) || port < 1 || port > 65535 ||
        user.length() >= sizeof(einstellungen.mqttUser) ||
        pass.length() >= sizeof(einstellungen.mqttPass)) {
      webserver.send(400, "text/plain; charset=utf-8", "Ungueltige MQTT-Einstellungen");
      return;
    }
    host.toCharArray(einstellungen.mqttHost, sizeof(einstellungen.mqttHost));
    user.toCharArray(einstellungen.mqttUser, sizeof(einstellungen.mqttUser));
    if (!pass.isEmpty()) pass.toCharArray(einstellungen.mqttPass, sizeof(einstellungen.mqttPass));
    einstellungen.mqttPort = port;
    einstellungen.mqttAktiv = aktiv ? 1 : 0;
    speichereEinstellungen();
    relaisKonfigurationSpeichern(webserver.hasArg("relay_mqtt"));
    mqttNeuKonfigurieren = true;
    webserver.sendHeader("Location", "/mqtt/settings", true);
    webserver.send(303, "text/plain; charset=utf-8", "Gespeichert");
  });
  webserver.on("/led", HTTP_POST, []() {
    if (!pruefeWebAnmeldung()) return;
    webserver.sendHeader("Location", "/system", true);
    webserver.send(303, "text/plain", "LED-Test gestartet");
    ledBlinken(3);
  });
  // WLAN direkt aus dem Heimnetz wechseln, ohne gleichzeitig einen Hotspot
  // und einen zweiten Webserver zu betreiben. Neustart nach HTTP-Antwort.
  webserver.on("/wifi/change", HTTP_POST, []() {
    if (!pruefeWebAnmeldung()) return;
    String ssid = webserver.arg("ssid");
    String pass = webserver.arg("password");
    if (ssid.length() < 1 || ssid.length() > 32 ||
        (pass.length() != 0 && (pass.length() < 8 || pass.length() > 63))) {
      webserver.send(400, "text/plain; charset=utf-8", "SSID/Kennwort ungueltig (WPA2: 8 bis 63 Zeichen).");
      return;
    }
    wlanNeueSSID = ssid;
    wlanNeuesPasswort = pass;
    wlanDirektWechselAngefordert = true;
    zeigeHinweis("WLAN wird gewechselt", "Neues Netz: " + ssid +
      ". Nach dem Neustart die neue IP im Router finden. Bei Verbindungsfehler startet das Einrichtungs-WLAN.");
  });
  webserver.on("/wifi", HTTP_POST, []() {
    if (!pruefeWebAnmeldung()) return;
    zeigeHinweis("WLAN-Einrichtung startet",
      "Verbinde dein Handy mit '"+setupWlanName+"', oeffne http://192.168.4.1 und richte das WLAN ein.");
    wlanWechselAngefordert = true;
  });
  webserver.on("/reset", HTTP_POST, []() {
    if (!pruefeWebAnmeldung()) return;
    zeigeHinweis("WLAN-Daten werden geloescht",
      "Der Wemos startet neu. Verbinde dich danach mit '"+setupWlanName+"'.");
    wlanLoeschenAngefordert = true;
  });
  webserver.on("/restart", HTTP_POST, []() {
    if (!pruefeWebAnmeldung()) return;
    zeigeHinweis("Neustart", "Der Wemos startet in wenigen Sekunden neu.");
    neustartAngefordert = true;
  });
  // HTTPUpdateServer registriert GET + POST unter /firmware mit Passwort.
  // Ein Firmware-Update belegt den OTA-Speicherbereich des Flashchips.
  httpUpdater.setup(&webserver, "/firmware", WEB_BENUTZER, ADMIN_PASSWORT);
  webserver.onNotFound([]() {
    webserver.send(404, "text/plain; charset=utf-8", "Seite nicht gefunden");
  });
}


// Arduino IDE: Netzwerk-Port (8266) mit derselben Admin-Kennung.
void starteArduinoOTA() {
  ArduinoOTA.setHostname(geraeteName.c_str());
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setPassword(ADMIN_PASSWORT);
  // Bei OTA-Dateitransfer keine Mess-ISR und keine MQTT-Nachrichten.
  ArduinoOTA.onStart([]() {
    otaWirdAusgefuehrt = true;
    relaisSchalten(0,false); relaisSchalten(1,false);
    ledWlanStatus = LED_UPDATE;
    detachInterrupt(digitalPinToInterrupt(ECHO_PIN));
    if (mqtt.connected()) mqtt.disconnect();
    Serial.println("OTA aus Arduino IDE gestartet ...");
  });
  ArduinoOTA.onEnd([]() { Serial.println("OTA erfolgreich; Neustart ..."); });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA fehlgeschlagen, Fehlercode: %u\n", (unsigned int)error);
    otaWirdAusgefuehrt = false;
    ledWlanStatus = (WiFi.status() == WL_CONNECTED) ? LED_ONLINE : LED_SUCHT;
    isrPulsAktiv = false;
    attachInterrupt(digitalPinToInterrupt(ECHO_PIN), echoWechselISR, CHANGE);
  });
  // ArduinoOTA registriert den mDNS-Dienst fuer die IDE.
  ArduinoOTA.begin();
  MDNS.addService("http", "tcp", 80);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // D1-mini-LED LOW-aktiv
  ledWlanStatus = LED_SUCHT;
  wlanLedTicker.attach_ms(100, ledTickerTick);
  // Relaisausgaenge zuerst in den inaktiven Zustand bringen.
  for (uint8_t i=0;i<2;i++) {
    digitalWrite(RELAIS_PINS[i], RELAIS_AKTIV_LOW ? HIGH : LOW);
    pinMode(RELAIS_PINS[i], OUTPUT);
    relaisAn[i]=false;
  }
  // D5 ist der dritte DS18B20-Bus, kein Triggerausgang.
  pinMode(ECHO_PIN, INPUT);
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== Sensorzentrale v1.9.6 (HA-Lite + Web-Statusfix) ===");
  Serial.println("Boot-Ursache: " + ESP.getResetReason());
  ladeEinstellungen();
  anzeigeNameLaden();
  ladeTankTabelle();
  tempZuordnungLaden();
  relaisKonfigurationLaden();
  temperaturInitialisieren();
  mqttInitialisieren();

  String chip=String(ESP.getChipId(),HEX);
  geraeteName="heizoel-"+chip;
  setupWlanName="Heizoel-Setup-"+chip;

  WiFi.mode(WIFI_STA);
  wifiManager.setHostname(geraeteName.c_str());
  wifiManager.setConnectTimeout(25);
  wifiManager.setSaveConnectTimeout(25);
  wifiManager.setWiFiAutoReconnect(true);
  wifiManager.setAPCallback(wlanPortalGestartet);
  // Dashboard und OTA erst nach erfolgreicher STA-Verbindung starten.
  // WiFiManager uebernimmt ausschliesslich beim Booten/auf Wunsch den AP.

  Serial.println("Setup-SSID: "+setupWlanName);
  Serial.println("Setup-Portal: http://192.168.4.1");
  if (!wifiManager.autoConnect(setupWlanName.c_str(),SETUP_AP_PASSWORT)) {
    Serial.println("WLAN-Einrichtung fehlgeschlagen. Neustart...");
    delay(1500);
    ESP.restart();
  }
  Serial.println("Heimnetz-IP: "+WiFi.localIP().toString());
  ledWlanStatus = LED_ONLINE;
  WiFi.mode(WIFI_STA); // AP im Normalbetrieb ausdruecklich aus
  WiFi.setAutoReconnect(true);

  starteArduinoOTA();
  registriereWebseiten();
  webserver.begin();
  ledBlinken(2);
  // Ab jetzt wird JEDER vom Sensor erzeugte Echo-Impuls asynchron aufgezeichnet.
  attachInterrupt(digitalPinToInterrupt(ECHO_PIN), echoWechselISR, CHANGE);
  Serial.println("Echo-Interrupt aktiv an D6: Abtastung im Sensortakt.");
}

void loop() {
  webserver.handleClient();
  ArduinoOTA.handle(); // beinhaltet MDNS.update()
  if (otaWirdAusgefuehrt) return;
  if ((uint32_t)(millis() - letzteHeapMessungMs) >= 5000UL) {
    letzteHeapMessungMs = millis();
    minFreierHeap = min(minFreierHeap, ESP.getFreeHeap());
  }

  // Kein periodisches pulseIn: Ringpuffer fortlaufend abarbeiten.
  sensorAktualisieren();
  temperaturAktualisieren();
  mqttAktualisieren();

  // Kein zeitgleicher Rettungs-Hotspot und kein Webserver-Neustart:
  // Bei laengerem Ausfall uebernimmt WiFiManager nach EINEM Neustart.
  if (WiFi.status() == WL_CONNECTED) {
    ledWlanStatus = LED_ONLINE;
    if (wlanGetrennt) Serial.println("WLAN-Verbindung wiederhergestellt.");
    wlanGetrennt = false;
  } else if (!wlanGetrennt) {
    ledWlanStatus = LED_SUCHT;
    wlanGetrennt = true;
    wlanGetrenntSeitMs = millis();
    Serial.println("WLAN verloren. SDK versucht automatisch wiederzuverbinden.");
  } else if ((uint32_t)(millis() - wlanGetrenntSeitMs) >= WLAN_AUSFALL_NEUSTART_MS) {
    Serial.println("WLAN >180 s nicht erreichbar: Neustart zum WiFiManager-Fallback.");
    delay(100);
    ESP.restart();
  }

  if (wlanDirektWechselAngefordert) {
    wlanDirektWechselAngefordert = false;
    ledWlanStatus = LED_SUCHT;
    delay(250); // Browser-Antwort zuerst abschliessen
    // Nur beim absichtlichen Aendern der Zugangsdaten Flash beschreiben.
    WiFi.persistent(true);
    WiFi.begin(wlanNeueSSID.c_str(), wlanNeuesPasswort.c_str());
    WiFi.persistent(false);
    delay(250);
    ESP.restart(); // WiFiManager prueft das neue Netz beim Start
  }

  if (wlanWechselAngefordert) {
    wlanWechselAngefordert=false;
    ledWlanStatus = LED_SETUP;
    delay(850);
    webserver.stop();
    mqtt.disconnect();
    detachInterrupt(digitalPinToInterrupt(ECHO_PIN));
    wifiManager.startConfigPortal(setupWlanName.c_str(),SETUP_AP_PASSWORT);
    delay(300);
    ESP.restart();
  }
  if (wlanLoeschenAngefordert) {
    wlanLoeschenAngefordert=false;
    ledWlanStatus = LED_SUCHT;
    delay(850);
    wifiManager.resetSettings();
    ESP.restart();
  }
  if (neustartAngefordert) {
    neustartAngefordert=false;
    delay(850);
    ESP.restart();
  }
}
