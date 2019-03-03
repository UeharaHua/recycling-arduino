/*
 * Copyright (c) 2016 Intel Corporation.  All rights reserved.
 * See the bottom of this file for the license terms.
 */

#include <CurieBLE.h>

BLEPeripheral blePeripheral;  // BLE Peripheral Device (the board you're programming)
BLEService pointService("88888888-4444-4444-4444-CCCCCCCCCCCC"); // BLE LED Service

// BLE Point Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEUnsignedCharCharacteristic switchCharacteristic("88888888-4444-4444-4444-CCCCCCCCCCC1", BLERead | BLEWrite);

const int pointPin = 0; // pin to use for the LED

void setup() {

  pinMode(pointPin, INPUT);

  // set advertised local name and service UUID:
  blePeripheral.setLocalName("ItsTime");
  blePeripheral.setAdvertisedServiceUuid(pointService.uuid());

  // add service and characteristic:
  blePeripheral.addAttribute(pointService);
  blePeripheral.addAttribute(switchCharacteristic);

  // set the initial value for the characeristic:
  switchCharacteristic.setValue(0);

  // begin advertising BLE service:
  Serial.begin(9600);
  Serial.println("Start Advertising");
  blePeripheral.begin();

}

void loop() {
  Serial.println("Looking for peripheral");
  // listen for BLE peripherals to connect:
  BLECentral central = blePeripheral.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

  
    // while the central is still connected to peripheral:
    while (central.connected()) {
      
      char pointValue = digitalRead(pointPin);
    
      boolean pointChanged = (switchCharacteristic.value() != pointValue);
    
      if (pointChanged && pointValue == 1) {
        switchCharacteristic.setValue(pointValue);
        Serial.println("point is 1");
      }
      switchCharacteristic.setValue(digitalRead(0));
      Serial.println(digitalRead(pointPin));
      delay(100);
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
  delay(100);
}

/*
   Copyright (c) 2016 Intel Corporation.  All rights reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
