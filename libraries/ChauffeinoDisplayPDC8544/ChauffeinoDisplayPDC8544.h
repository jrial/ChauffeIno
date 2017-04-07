#ifndef ChauffeinoDisplay_h
#define ChauffeinoDisplay_h

#include <Arduino.h>

class ChauffeinoDisplay {
    private:
        String roomId;
        float tempMeasured, tempRequested;

    public:
        // ChauffeinoDisplay();
        ChauffeinoDisplay(uint8_t lcdPins[] = {14, 15, 16, 17, 18, 19});
        // I find row, col more intuitive...
        void setDimensions(int rows, int cols);
        void print(String text, int row, int col);
        void clearRow(int row);
        void printTemperatures(bool low_batt);
        void printId();
        void setTempRequested(float temp);
        void setTempMeasured(float temp);
        void setTemperatures(float tempMeasured, float tempRequested);
        void setId(String roomId);
};

#endif
