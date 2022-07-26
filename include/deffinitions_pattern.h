// --- túto časť sa dá zmazať
// Vzorový súbor a popis nastavení
// Pre reálnu aplikáciu je potrebné premenovať na: deffinitions.h
// --- túto časť sa dá zmazať

/** Nastavenia a ich popis
 *
 * Posledná zmena(Last change): 25.07.2022
 *
 * @author Ing. Peter VOJTECH ml. <petak23@gmail.com>
 * @copyright  Copyright (c) 2022 - 2022 Ing. Peter VOJTECH ml.
 * @link       http://petak23.echo-msz.eu
 * @version 1.0.1
 */

/** Nastavenia Wifi */
#define WIFI_SSID "xxxxxxx"
#define WIFI_PASSWORD "xxxxxxxx"

/** ------ Nastavenia MQTT ------- */
// Nastavenia broker-a
#define MQTT_ENBLED true // Celkové povolenie MQTT
#define MQTT_HOST IPAddress(192, 168, 0, 101)
#define MQTT_PORT 1883
#define MQTT_USER "xxxxxx"
#define MQTT_PASSWORD "xxxxxx"

// Nastavenie topic
const char *topic_meteo_status = "zahradka/meteo/status"; // Publikácia dát meteostanice
const char *topic_meteo_last = "zahradka/meteo/last";     // Výzva na publikáciu posledných údajov
/* ------- Nastavenia MQTT - Koniec ------- */

/** Prihlasovanie do OTA(AsyncElegantOTA) pre update firmware */
#define OTA_USER "xxxxxx"
#define OTA_PASSWORD "xxxxxx"

/** Ostaté Nastavenia */
#define SERIAL_PORT_ENABLED false // Povolenie výstupu na sériový port - logovanie

/** Definície pinov pre vysielací modul */
#define LORA_SS 5
#define LORA_RESET 14
#define LORA_DIO0 2
// your location's frequency: 433E6 for Asia; 866E6 for Europe; 915E6 for North America
#define LORA_FREQ 866E6
// The sync word assures you don't get LoRa messages from other LoRa transceivers
// ranges from 0-0xFF
#define LORA_SYNC_WORD 0xFF