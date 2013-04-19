#ifndef ChauffeinoDisplay_h
#define ChauffeinoDisplay_h

#include <Arduino.h>
#include <LiquidCrystal.h>

class ChauffeinoDisplay: public LiquidCrystal {
    private:
        String roomId;
        float tempMeasured, tempRequested;

    public:
        ChauffeinoDisplay(uint8_t pins[]);
        // I find row, col more intuitive...
       void setDimensions(int rows, int cols);
       void print(String text, int row, int col);
       void printTemperatures();
       void printId();
       void setTempRequested(float temp);
       void setTempMeasured(float temp);
       void setTemperatures(float tempMeasured, float tempRequested);
       void setId(String roomId);
};

#endif