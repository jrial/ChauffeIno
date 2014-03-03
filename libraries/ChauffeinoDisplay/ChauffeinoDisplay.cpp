#include "ChauffeinoDisplay.h"

ChauffeinoDisplay::ChauffeinoDisplay(uint8_t pins[]): LiquidCrystal(pins[0], pins[1], pins[2], pins[3], pins[4], pins[5]) {};

void ChauffeinoDisplay::setDimensions(int rows, int cols) {
    begin(cols, rows);
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
    print(tempString, 0, 0);
}

void ChauffeinoDisplay::printId() {
    clearRow(1);
    print(roomId, 1, 0);
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
