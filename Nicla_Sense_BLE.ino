/*
   This sketch shows how nicla can be used in standalone mode.
   Without the need for an host, nicla can run sketches that
   are able to configure the bhi sensors and are able to read all
   the bhi sensors data.
*/


#include "Arduino.h"
#include "Arduino_BHY2.h"
#include "Nicla_System.h"
#include <ArduinoBLE.h>
#include <U8g2lib.h>
#include <U8x8lib.h>
#include <Wire.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

SensorXYZ accel(SENSOR_ID_ACC);
SensorXYZ gyro(SENSOR_ID_GYRO);
SensorXYZ mag(SENSOR_ID_MAG);
Sensor motion(SENSOR_ID_ANY_MOTION);
Sensor temp(SENSOR_ID_TEMP);
Sensor gas(SENSOR_ID_GAS);
SensorQuaternion rotation(SENSOR_ID_RV);

BLEService NiclaSenseService("07e1f0e2-b68f-4f71-8bfc-19b3b0427b68");
BLEUnsignedIntCharacteristic  ledCharacteristic("8e0d0578-4d37-4c89-abc6-61b2c085bd1c", BLERead | BLEWrite); 

BLEShortCharacteristic        gyroXCharacteristic("85a2072b-ace2-456a-b6cb-50d81d81eca1", BLERead | BLENotify);
BLEShortCharacteristic        gyroYCharacteristic("337eabbb-8f75-4113-9c26-6f6fb54293e1", BLERead | BLENotify);
BLEShortCharacteristic        gyroZCharacteristic("03771361-106f-441d-bb9d-8f1ab06ddb25", BLERead | BLENotify);

BLEShortCharacteristic        accXCharacteristic("ac3f440e-6a52-43df-a32f-9c018fad815b", BLERead | BLENotify);
BLEShortCharacteristic        accYCharacteristic("cf714d19-331d-4f2c-af24-3619738e1671", BLERead | BLENotify);
BLEShortCharacteristic        accZCharacteristic("d3ab4220-bbe3-4ce2-814a-436853dcb9ce", BLERead | BLENotify);

BLEIntCharacteristic          magCharacteristic("bcbb213c-9393-42f6-959a-aa2b859a77de", BLERead | BLENotify);
BLEShortCharacteristic        tempCharacteristic("5d75d722-fce9-471b-93e2-6c625ef6d634", BLERead | BLENotify);
BLEShortCharacteristic        motionCharacteristic("d70869aa-892a-4020-8d54-a21a8b0c474e", BLERead | BLENotify);

void setup()
{
  nicla::begin();
  Serial.begin(115200);
  nicla::leds.begin();
  BHY2.begin();
  Wire.begin();
  u8g2.begin();
  
  while (!Serial);
  
  accel.begin();
  gyro.begin();
  temp.begin();
  gas.begin();
  rotation.begin();
  motion.begin();
  
  BLE.setLocalName("Nicla Shra1");
  BLE.setAdvertisedService(NiclaSenseService);

  // add the characteristic to the service
  NiclaSenseService.addCharacteristic(ledCharacteristic);
  NiclaSenseService.addCharacteristic(gyroXCharacteristic);
  NiclaSenseService.addCharacteristic(gyroYCharacteristic);
  NiclaSenseService.addCharacteristic(gyroZCharacteristic);
  NiclaSenseService.addCharacteristic(accXCharacteristic);
  NiclaSenseService.addCharacteristic(accYCharacteristic);
  NiclaSenseService.addCharacteristic(accZCharacteristic);
  NiclaSenseService.addCharacteristic(magCharacteristic);
  NiclaSenseService.addCharacteristic(tempCharacteristic);
  NiclaSenseService.addCharacteristic(motionCharacteristic);

  loadSenseVals();

  // add service
  BLE.addService(NiclaSenseService);
  BLE.advertise();
}

bool loadSenseVals()
{
  Serial.print("Updating sensor values");
  
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
  u8g2.drawStr(0,10,"Nicla Test");  // write something to the internal memory
  u8g2.drawLine(0,12, 128, 12);
  u8g2.drawStr(5, 22, "Temp:");
  u8g2.drawStr(5, 35, "AccX:");
  u8g2.drawStr(5, 45, "AccY:");
  u8g2.drawStr(5, 55, "AccZ:");
  
  u8g2.drawStr(40, 22, (String(temp.value() + String("C"))).c_str());
  u8g2.drawStr(40, 35, (String(accel.x())).c_str());
  u8g2.drawStr(40, 45, (String(accel.y())).c_str());
  u8g2.drawStr(40, 55, (String(accel.z())).c_str());
  
  u8g2.sendBuffer();          // transfer internal memory to the display

  
  Serial.println(String("acceleration: ") + accel.toString());
  Serial.println(String("gyroscope: ") + gyro.toString());
  Serial.println(String("temperature: ") + String(temp.value(), 3));
  Serial.println(String("gas: ") + String(gas.value(), 3));
  Serial.println(String("rotation: ") + rotation.toString());

  
  
  BLEDevice central = BLE.central();
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());
    if (central.connected()) {
      if (ledCharacteristic.written()) {
        if (ledCharacteristic.value()) {   // any value other than 0
          int val = ledCharacteristic.value();
          int blue = (int)(round)(val / 1000000);
          val -= blue * 1000000;
          int green = (int)(round)(val/1000);
          val -= green * 1000;
          int red = (int)(round)(val);
          Serial.println("LED on");
          nicla::leds.setColor(blue, green, red);
          //delay(500);
          //nicla::leds.setColor(yellow);
          
        } else {                           // a 0 value
          Serial.println(F("LED off"));
          nicla::leds.setColor(off);      // will turn the LED off
        }
      }
      tempCharacteristic.writeValue(temp.value() * 100);
      gyroXCharacteristic.writeValue(gyro.x());
      gyroYCharacteristic.writeValue(gyro.y());
      gyroZCharacteristic.writeValue(gyro.z());
      accXCharacteristic.writeValue(accel.x());
      accYCharacteristic.writeValue(accel.y());
      accZCharacteristic.writeValue(accel.z());
      magCharacteristic.writeValue(mag.z());
      motionCharacteristic.writeValue(motion.value());
      return true;
      
    } else {
      Serial.print("Not Connected");
      return false;
    }
  }
  Serial.print("Not Connected !!!");
  return false;
}

void loop()
{
  //BLEDevice central = BLE.central();
  static auto printTime = millis();
  

  // Update function should be continuously polled
  BHY2.update();

  if (millis() - printTime >= 500) {
    printTime = millis();
    bool updated = loadSenseVals();
    
  }
}
