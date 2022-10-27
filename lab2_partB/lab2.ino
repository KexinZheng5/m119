// Sends IMU data to another device
// Reference: https://docs.arduino.cc/tutorials/nano-33-iot/bluetooth

#include <ArduinoBLE.h>
#include <Arduino_LSM6DS3.h>

long previousMillis = 0;
int interval = 0;
int ledState = LOW;
unsigned int SIZE = 24;

BLEService mluService("lab2"); // BLE LED Service

BLECharacteristic mluCharacteristic("2A57", BLERead, SIZE, true);

void writeFloat(float value, byte buffer[], unsigned int offset){
  for (int i = 0; i < 4; ++i){
    buffer[offset + i] = ((uint8_t*)&value)[i];
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // set built in LED pin to output mode
  pinMode(LED_BUILTIN, OUTPUT);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy failed!");

    while (1);
  }
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");

    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("kexin's arduino nano 33");
  BLE.setAdvertisedService(mluService);

  // add the characteristic to the service
  mluService.addCharacteristic(mluCharacteristic);

  // add service
  BLE.addService(mluService);

  // start advertising
  BLE.advertise();

  Serial.println("BLE Ready");
}

void loop() {
  // create buffer to store mlu values
  byte buffer[SIZE];
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, HIGH);

    // while the central is still connected to peripheral:
    while (central.connected()) {
      if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
        float imuData[3];
        IMU.readAcceleration(imuData[0], imuData[1], imuData[2]);
        for (int i = 0; i < 3; ++i){
          writeFloat(imuData[i], buffer, i * 4);
        }
        IMU.readGyroscope(imuData[0], imuData[1], imuData[2]);
        for (int i = 0; i < 3; ++i){
          writeFloat(imuData[i], buffer, (i + 3) * 4);
        }
        mluCharacteristic.writeValue(buffer, SIZE);
      } 
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, LOW);
  }
}