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
* New generic display code
* Override buttons
*/

// Which screen have we got hooked up?
#define SCREEN_HD44780

#include <OneWire.h>
#include <DallasTemperature.h>
#include <RFM69.h>
#include <avr/sleep.h>
#include <LowPower.h>
#include <SPI.h>
// Select correct screen library
#ifdef SCREEN_HD44780
// Due to Arduino sillyness, if I don't include it here, it won't be found when
// compiling ChauffeinoDisplayHD44780.h, yet it also has to be included there,
// as does Arduino.h or ChauffeinoDisplayHD44780.cpp won't compile. Ugh...
#include <LiquidCrystal.h>
#include <ChauffeinoDisplayHD44780.h>
#elif SCREEN_PCD8544
#include <ChauffeinoDisplayPDC8544.h>

#endif

// Global vars - generic
uint8_t tempUpPin = 4;
uint8_t tempDownPin = 5;
const uint8_t bufferSize = 3;
float buffer[bufferSize];
float heatDemand = 16.0;
#define SERIAL_BAUD  115200
#define LED         9


// RFM69
// #define BOOT_NODEID 127  //network ID used for this unit at boot time
#define NETWORKID       98      //the network ID we are on
#define GATEWAYID       1       //the node ID we're sending to
#define ACK_TIME        100     // # of ms to wait for an ack
#define FREQUENCY       RF69_433MHZ
#define NODEID          2
#define SLEEP_LOOPS     1       // How many 8s loops to sleep before entering loop() again?
#define RECV_TIMEOUT    1000    // How long to wait for a reply from the base station after sending
// uint8_t nodeid;
const char KEY[] = "SOMERANDOMSTRING";
RFM69 radio;
bool requestACK=true;
bool promiscuousMode = false;   //set to 'true' to sniff all packets on the same network
unsigned long lastSend = 0;     // When did we last send a temperature measurement to the node?
bool failNotified = false;      // To ensure we print "no reply yet" only once.


// Battery related
int8_t batt_counter=-1;
bool low_batt = false;
uint16_t min_vcc = 2750;
const long scaleConst = 1156.300 * 1000 ; // internalRef * 1023 * 1000;

// DS18x20
int sensorPin = 3;
OneWire oneWire(sensorPin);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor;
char sensorString[8];
unsigned long lastTempRequest = 0;
int  delayInMillis = 0;
int8_t resolution = 9;
float tempC;

// LCD
ChauffeinoDisplay display;

typedef struct {
    char        sensorId[16];   // ID of the sensor; this is the node's ID card on the netwwork
    float       temp;           // Temperature
    bool        low_batt;       // Low battery indicator
} tempStruct;
tempStruct reportData;

typedef struct {
    char        curDate[10];    // To display the date on the thermostat nodes
    char        curTime[5];     // To display the time on the thermostat nodes
    float       targetTemp;     // Target temperature in thermostat node's room
    bool        burner;         // Burner is on (water is being heated)
    bool        valve;          // Valve to thermostat node's room open
} gwStruct;
gwStruct returnData;

void setup() {
    // Setup serial line for debugging
    Serial.begin(SERIAL_BAUD);
    Serial.println("Thermostat initialising");
    radio.initialize(FREQUENCY,NODEID,NETWORKID);
    radio.encrypt(KEY);
    radio.promiscuous(promiscuousMode);
    char buff[50];
    sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
    Serial.println(buff);
    sprintf(buff, "Encryption key: %s", KEY);
    Serial.println(buff);
    radio.sleep();
    // Print boot text
    display.printId("Chauffeino");
    // Setup thermal sensor
    pinMode(sensorPin, INPUT);
    sensors.begin();
    if (sensors.getDeviceCount()) {
        sensors.getAddress(sensor, 0);
        char sensorString[16];
        char tmp_hex[2];
        for (uint8_t i = 0; i < 8; i++) {
            sprintf(tmp_hex, "%02x", sensor[i]);
            sensorString[i*2] = tmp_hex[0];
            sensorString[i*2+1] = tmp_hex[1];
            // sensorString[i*2] =  sensor[i];
        }
        sprintf(buff, "Sensor ID: %s", sensorString);
        Serial.println(buff);
        if (sensor[0] == DS18B20MODEL || sensor[0] == DS1825MODEL || sensor[0] == DS1822MODEL)
        resolution = 12;
        else if (sensor[0] == DS18S20MODEL)
            resolution = 9;
        else {
            Serial.println("Sensors found, but not Dallas DS18x20 family.\nTerminating.");
            display.printError("SENSOR ERROR!!!");
            flushSerial();
            LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
        }
        sensors.setResolution(sensor, resolution);
        sensors.requestTemperaturesByAddress(sensor);
        delayInMillis = 750 / (1 << (12 - resolution));
        lastTempRequest = millis();
        int resolution = sensors.getResolution(sensor);
        sprintf(buff, "Sensor resolution: %d", resolution);
        Serial.println(buff);
        display.printId(buff);
        // With that done, let's get ourselves a node ID, shall we?
        /*
        radio.Initialize(BOOT_NODEID, FREQUENCY, NETWORKID);
        radio.Encrypt(KEY);
        String msg = "request_node_id ";
        msg += sensor_string;
        radio.Send(GATEWAYID, &msg, msg.length());
        */
        reportData.low_batt = false;
        for (uint8_t i = 0; i<16; i++) {
            reportData.sensorId[i] = sensorString[i];
        }
    } else {
        Serial.println("No sensors found.\nTerminating.");
        display.printError("SENSOR ERROR!!!");
        flushSerial();
        LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
    }
    Serial.println("Thermostat initialisation complete.");
}


void loop() {
    String room = "Living Room";
    // Check battery level every hour
    batt_counter = (batt_counter + 1) % 60;
    // Resetting Low Battery warning should require a reset or restart
    if (! low_batt && batt_counter == 0) {
        if (readVccMv() < min_vcc) {
            reportData.low_batt = true;
        }
    }
    // Get temperature in °C
    // Wait required time for DS18x20 to report
    //if (millis() - lastTempRequest >= delayInMillis) {
        tempC = sensors.getTempC(sensor);
        sensors.requestTemperaturesByAddress(sensor);
        lastTempRequest = millis();
    //}
    // Update display
    display.setId(room);
    display.setTemperatures(tempC, heatDemand);
    display.clear();
    display.printTemperatures(low_batt);
    display.printId();
    Serial.print("Temperature: ");
    Serial.println(tempC);
    reportData.temp = tempC;
    Serial.print("  Transmitting:");
    if (radio.sendWithRetry(GATEWAYID, (const void*)(&reportData), sizeof(reportData))) {
        Serial.println(" OK!");
    } else {
        Serial.println(" nothing...");
    }
    lastSend = millis();
    failNotified = false;
    Serial.print("  Receiving:");
    while (millis() - lastSend < RECV_TIMEOUT) {
        if (radio.receiveDone()) {
            Blink(LED,3);
            Serial.println(" OK!");
            if (radio.DATALEN != sizeof(gwStruct)) {
               Serial.println(" Invalid tempStruct received, not matching tempStruct struct!");
            } else {
                returnData = *(gwStruct*)radio.DATA;
                Serial.print("Date: ");
                Serial.println(returnData.curDate);
                Serial.print("Time: ");
                Serial.println(returnData.curTime);
                Serial.print("Temp: ");
                Serial.print(reportData.temp);
                Serial.print("/");
                Serial.println(returnData.targetTemp);
                Serial.print("Burn: ");
                Serial.println(returnData.burner ? "On" : "Off");
                Serial.print("Valv: ");
                Serial.println(returnData.valve ? "On" : "Off");
                if (radio.ACKRequested()) {
                    byte theNodeID = radio.SENDERID;
                    radio.sendACK();
                }
            }
            break;
        } else if (!failNotified) {
            Serial.println(" nothing...");
            failNotified = true;
        }
    }
    // Sleep for a minute
    radio.sleep();
    flushSerial();
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    // for (int i=0; i<SLEEP_LOOPS; i++) {
    //     LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    //     LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
    // }
}

// Helper method to properly flush; HardwareSerial::flush() seems broken
// See http://forum.arduino.cc/index.php?topic=151014.0, esp. this post:
// http://forum.arduino.cc/index.php?topic=151014.msg1344132#msg1344132
void flushSerial() {
    while (!(UCSR0A & (1 << UDRE0)))  // Wait for empty transmit buffer
    UCSR0A |= 1 << TXC0;  // mark transmission not complete
    while (!(UCSR0A & (1 << TXC0)));   // Wait for the transmission to complete
}

/*
  Check battery level; return external voltage,
  calculated against high-precision 1.1V internal
  VREF.
  Taken from http://code.google.com/p/tinkerit/wiki/SecretVoltmeter
  */
// long readVccMv() {
//     long result;
//     // Read 1.1V reference against AVcc
//     ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
//     delay(2); // Wait for Vref to settle
//     ADCSRA |= _BV(ADSC); // Convert
//     while (bit_is_set(ADCSRA,ADSC));
//     result = ADCL;
//     result |= ADCH<<8;
//     result =  1126400L / result; // Back-calculate AVcc in mV
//     return (uint16_t) result;
// }

// Better ReadVcc method.
// Taken from http://www.rcgroups.com/forums/showthread.php?t=1874973
int readVccMv() {
    // Read 1.1V reference against AVcc
    // set the reference to Vcc and the measurement to the internal 1.1V reference
// Leonardo, Micro, Esplora, Duemilanove, Mega 2560, Mega ADK
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
// No clue, not planning to look it up
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
// No clue, not planning to look it up
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
// All others
#else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  //result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  result = scaleConst / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return (int)result; // Vcc in millivolts
}

void Blink(byte PIN, int DELAY_MS) {
    pinMode(PIN, OUTPUT);
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
}
