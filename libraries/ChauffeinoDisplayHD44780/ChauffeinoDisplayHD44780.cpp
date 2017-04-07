#include "ChauffeinoDisplayHD44780.h"

uint8_t defaultPins[] = {14, 15, 16, 17, 18, 19};
uint8_t defaultDimensions[] = {2, 16};


ChauffeinoDisplay::ChauffeinoDisplay(void): LiquidCrystal(defaultPins[0], defaultPins[1], defaultPins[2], defaultPins[3], defaultPins[4], defaultPins[5]) {
    ChauffeinoDisplay(defaultPins, defaultDimensions);
};

ChauffeinoDisplay::ChauffeinoDisplay(uint8_t pins[], uint8_t dimensions[]): LiquidCrystal(pins[0], pins[1], pins[2], pins[3], pins[4], pins[5]) {
    this->setDimensions(dimensions);
};

void ChauffeinoDisplay::setDimensions(uint8_t dimensions[]) {
    begin(dimensions[1], dimensions[0]);
    this->idLine = 1;
    this->errorLine = 2;
}

void ChauffeinoDisplay::print(String text, int row, int col) {
    setCursor(col, row);
    LiquidCrystal::print(text);
}

void ChauffeinoDisplay::clearRow(int row) {
    print("                ", row, 0);
}

void ChauffeinoDisplay::printTemperatures(bool low_batt) {
    clearRow(0);
    char tempString [17];
    dtostrf(tempMeasured, 1, 1, &tempString[0]);
    dtostrf(tempRequested, 1, 1, &tempString[5]);
    tempString[4] = '/';
    if (low_batt) {
        strcat(tempString, " BATT!");
    }
    this->print(tempString, 1, 0);
}

void ChauffeinoDisplay::printId() {
    this->printId(this->roomId);
}

void ChauffeinoDisplay::printId(String id) {
    clearRow(this->idLine);
    print(id, this->idLine, 0);
}

void ChauffeinoDisplay::printError(String error) {
    this->print(error, this->idLine, 0);
}

void ChauffeinoDisplay::setTempRequested(float temp) {
    tempRequested = temp;
}

void ChauffeinoDisplay::setTempMeasured(float temp) {
    tempMeasured = temp;
}

void ChauffeinoDisplay::setTemperatures(float tempMeasured, float tempRequested) {
    setTempMeasured(tempMeasured);
    setTempRequested(tempRequested);
}

void ChauffeinoDisplay::setId(String roomId) {
    this->roomId = roomId;
}
