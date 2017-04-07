#include <RFM69.h>
#include <SPI.h>

// For debugging
#define NODEID      1
#define NETWORKID   98
#define FREQUENCY   RF69_433MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
#define KEY         "SOMERANDOMSTRING" //has to be same 16 characters/bytes on all nodes, not more not less!
#define LED         9
#define SERIAL_BAUD 115200
#define ACK_TIME    30  // # of ms to wait for an ack

RFM69 radio;
bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network
bool mock = true;

typedef struct {
    char        sensorId[16];   // ID of the sensor; this is the node's ID card on the netwwork
    float       temp;           // temperature
    bool        low_batt;       // Low battery indicator
} tempStruct;
tempStruct inData;

typedef struct {
    char        curDate[10];    // To display the date on the thermostat nodes
    char        curTime[5];     // To display the time on the thermostat nodes
    float       targetTemp;     // Target temperature in thermostat node's room
    bool        burner;         // Burner is on (water is being heated)
    bool        valve;          // Valve to thermostat node's room open
} gwStruct;
gwStruct outData;

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(10);
    radio.initialize(FREQUENCY,NODEID,NETWORKID);
    //radio.setHighPower(); //uncomment only for RFM69HW!
    radio.encrypt(KEY);
    radio.promiscuous(promiscuousMode);
    char buff[50];
    sprintf(buff, "\nListening at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
    Serial.println(buff);
    strncpy(outData.curDate, "2015-12-08", 10);
    strncpy(outData.curTime, "21:41", 5);
    outData.burner = false;
    outData.valve = false;
}

byte theNodeID;
byte ackCount=0;
void loop() {
    bool transmit = false;
    // 6 characters is the preamble
    if(Serial.available() > 6) {
        String serIn = Serial.readStringUntil('\n');
        String preamble = serIn.substring(0, 6);
        String data = serIn.substring(6);
        if (preamble == "date: ")
            strncpy(outData.curDate, data.c_str(), 10);
        if (preamble == "time: ")
            strncpy(outData.curTime, data.c_str(), 5);
        if (preamble == "temp: ")
            outData.targetTemp = data.toFloat();
        if (preamble == "burn: ")
            outData.burner = (data == "True");
        if (preamble == "valv: ")
            outData.valve = (data == "True");
        if (preamble == "transm") {
            transmit = true;
            Serial.println("Transmitting following data:");
            Serial.print("Date: ");
            Serial.println(outData.curDate);
            Serial.print("Time: ");
            Serial.println(outData.curTime);
            Serial.print("Temp: ");
            Serial.println(outData.targetTemp);
            Serial.print("Burn: ");
            Serial.println(outData.burner);
            Serial.print("Valv: ");
            Serial.println(outData.valve);

        }
    }
    else if(mock) {
        strncpy(outData.curDate, "1970-01-01", 10);
        strncpy(outData.curTime, "12:00", 5);
        outData.targetTemp = 22.5f;
        outData.burner = (inData.temp < outData.targetTemp);
        outData.valve = (inData.temp < outData.targetTemp);
        transmit = true;
    }
    if (radio.receiveDone()) {
        Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
        Serial.print(" [RX_RSSI:");Serial.print(radio.readRSSI());Serial.print("]");
        if (promiscuousMode) {
            Serial.print("to [");Serial.print(radio.TARGETID, DEC);Serial.print("] ");
        }

        if (radio.DATALEN != sizeof(tempStruct))
        Serial.print("Invalid tempStruct received, not matching tempStruct struct!");
        else {
            inData = *(tempStruct*)radio.DATA; //assume radio.DATA actually contains our struct and not something else
            Serial.print(" nodeId=");
            Serial.print(radio.SENDERID);
            Serial.print(" sensorId=");
            Serial.print(inData.sensorId);
            Serial.print(" temp=");
            Serial.print(inData.temp);
            Serial.print(" batt=");
            Serial.print(inData.low_batt ? "Low" : "OK");
        }

        theNodeID = radio.SENDERID;
        delay(3);
        if (radio.ACKRequested()) {
            radio.sendACK();

            // When a node requests an ACK, respond to the ACK
            // and also send a packet requesting an ACK (every 3rd one only)
            // This way both TX/RX NODE functions are tested on 1 end at the GATEWAY
            // if (ackCount++%3==0) {
            //     Serial.print(" Pinging node ");
            //     Serial.print(theNodeID);
            //     Serial.print(" - ACK...");
            //     delay(3); //need this when sending right after reception .. ?
            //     if (radio.sendWithRetry(theNodeID, "ACK TEST", 8, 0))  // 0 = only 1 attempt, no retries
            //         Serial.print("ok!");
            //     else
            //         Serial.print("nothing");
            // }
        }
        Serial.println();
        if (mock) {
            Serial.print("Sending to node: ");
            Serial.println(theNodeID);
            if (radio.sendWithRetry(theNodeID, (const void*)(&outData), sizeof(outData))) {
                Serial.println("Ack received");
            }
            Blink(LED,3);
            Serial.println("End mock Transmission");
        }
    }
    if (transmit and not mock) {
        Serial.print("Sending to node: ");
        Serial.println(theNodeID);
        radio.sendWithRetry(theNodeID, (const void*)(&outData), sizeof(outData));
        Blink(LED,3);
        Serial.println("End  REALTransmission");
    }
}



void Blink(byte PIN, int DELAY_MS)
{
    pinMode(PIN, OUTPUT);
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
}
