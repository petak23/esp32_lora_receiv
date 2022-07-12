#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include "definitions.h"

/**
 * Program pre LoRa komunikáciu pomocou ESP32 - prijímač
 *
 * Posledná zmena(last change): 12.07.2022
 * @author Ing. Peter VOJTECH ml. <petak23@gmail.com>
 * @copyright  Copyright (c) 2022 - 2022 Ing. Peter VOJTECH ml.
 * @license
 * @link       http://petak23.echo-msz.eu
 * @version 1.0.0
 */

void setup()
{
  // initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("LoRa Receiver");

  // setup LoRa transceiver module
  LoRa.setPins(LORA_SS, LORA_RESET, LORA_DIO0);
  while (!LoRa.begin(LORA_FREQ))
  {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(LORA_SYNC_WORD);
  Serial.println("LoRa Initializing OK!");
}

void loop()
{
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available())
    {
      String LoRaData = LoRa.readString();
      Serial.print(LoRaData);
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}