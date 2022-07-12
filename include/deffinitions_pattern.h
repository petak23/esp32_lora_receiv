// --- túto časť sa dá zmazať
// Vzorový súbor a popis nastavení
// Pre reálnu aplikáciu je potrebné premenovať na: deffinitions.h
// --- túto časť sa dá zmazať

/** Nastavenia a ich popis
 *
 * Posledná zmena(Last change): 12.07.2022
 *
 * @author Ing. Peter VOJTECH ml. <petak23@gmail.com>
 * @copyright  Copyright (c) 2022 - 2022 Ing. Peter VOJTECH ml.
 * @link       http://petak23.echo-msz.eu
 * @version 1.0.0
 */

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