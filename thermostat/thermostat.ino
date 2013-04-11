/*
This sketch turns the moteino into a thermostat node. It reads the temperature
from a Dallas Semiconductor TS18B20 temerature sensor over the One-Wire
protocol. It uses an LCD to display status info, and two buttons to override
the programmed temperature. The program is not kept on the device, it is
stored on the controller unit, and info on the currently active program
is transmitted back to the thermostat.
*/

/*
TODO:
* Wireless reporting
* Wireless feedback from controller
* Override buttons
*/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
#include <RFM12B.h>
#include <avr/sleep.h>

// Global vars - generic
uint8_t tempUpPin = 4;
uint8_t tempDownPin = 5;
uint8_t sleepSeconds = 20;
const uint8_t bufferSize = 3;
uint8_t bufferPos = 0;
float buffer[bufferSize];

// RFM128B
#define BOOT_NODEID 127  //network ID used for this unit at boot time
#define NETWORKID    98  //the network ID we are on
#define GATEWAYID     1  //the node ID we're sending to
#define ACK_TIME     50  // # of ms to wait for an ack
#define SERIAL_BAUD  115200
uint8_t nodeid; 
uint8_t KEY[] = "SOMERANDOMSTRING";
RFM12B radio;
bool requestACK=true;

// DS18x20
int sensorPin = 3;
OneWire oneWire(sensorPin);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor;

// LCD
LiquidCrystal lcd(14, 15, 16, 17, 18, 19);


void setup() 
{
    // Setup serial line for debugging
    Serial.begin(9600);
    // Setup LCD
    for (int i = 14; i <= 19; i++) {
        pinMode(i, OUTPUT);
    }
    lcd.begin(16, 2);
    lcd.println("Chauffeino");
    // Setup thermal sensor
    pinMode(sensorPin, INPUT);
    sensors.begin();
    if (sensors.getDeviceCount()) {
        sensors.getAddress(sensor, 0);
        Serial.print("Sensor address: ");
        String sensor_string = "";
        for (uint8_t i = 0; i < 8; i++) {
            sensor_string += sensor[i];
        }
        Serial.println(sensor_string);
        if (sensor[0] == 0x28)
            sensors.setResolution(sensor, 12);
        else if (sensor[0] == 0x10 or sensor[0] == 0x22)
            sensors.setResolution(sensor, 9);
        else {
            Serial.println("Sensors found, but not Dallas DS18x20 family.\nTerminating.");
            exit(1);
        }
        int resolution = sensors.getResolution(sensor);
        Serial.print("Temp resolution: ");
        Serial.println(resolution, DEC);
        lcd.setCursor(0, 1);
        lcd.print("Res: ");
        lcd.println(resolution, DEC);
        // With that done, let's get ourselves a node ID, shall we?
        /*
        radio.Initialize(BOOT_NODEID, RF12_433MHZ, NETWORKID);
        radio.Encrypt(KEY);
        String msg = "request_node_id ";
        msg += sensor_string;
        radio.Send(GATEWAYID, &msg, msg.length());
        */
    } else {
        Serial.println("No sensors found.\nTerminating.");
        exit(1);
    }
}


void loop() 
{
    // Get temperature in °C
    Serial.println("Requesting temperature");
    sensors.requestTemperaturesByAddress(sensor);
    // delay(750); // Maybe not necessary?
    float tempC = sensors.getTempC(sensor);
    Serial.print("Temperature: ");
    Serial.println(tempC);
    // and store it in the output buffer at position bufferPos.
    buffer[bufferPos] = tempC;
    // rotate to next position in output buffer.
    bufferPos = (bufferPos + 1) % bufferSize;
    // If buffer full, report wirelessly and clear buffer
    if (not bufferPos) {
        // FIXME: report *wirelessly*
        for (int i = 0; i < bufferSize; i++) {
            Serial.print("Temp ");
            Serial.print(i);
            Serial.print(": ");
            Serial.println(buffer[i]);
        }
        // FIXME: listen for commands from controller
    }
    // FIXME: Sleep with interrupt instead of delay.
    delay(sleepSeconds*1000);
}
