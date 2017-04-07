#ifndef ChauffeinoDisplay_h
#define ChauffeinoDisplay_h

#include <Arduino.h>
#include <LiquidCrystal.h>

class ChauffeinoDisplay: public LiquidCrystal {
    private:
        String roomId;
        float tempMeasured, tempRequested;
        uint8_t idLine;
        uint8_t errorLine;


    public:
        ChauffeinoDisplay(void);
        ChauffeinoDisplay(uint8_t pins[], uint8_t dimensions[]);
        void setDimensions(uint8_t dimensions[]);
        void print(String text, int row, int col);
        void clearRow(int row);
        void printTemperatures(bool low_batt);
        void printId();
        void printId(String id);
        void printError(String error);
        void setTempRequested(float temp);
        void setTempMeasured(float temp);
        void setTemperatures(float tempMeasured, float tempRequested);
        void setId(String roomId);
};

#endif
