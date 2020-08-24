#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <Adafruit_MAX31865.h>

#define MENU_TIMEOUT 90

/******************************
 * Temp Sensor MAX31865
 ******************************/
 // The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF      4300.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL  1000.0
// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 max = Adafruit_MAX31865(53, 51, 50, 52);

/******************************
 * Temp Sensor DS18B20
 ******************************/
const int tempSensorPin1 = 30;
OneWire OWTempSensor1(tempSensorPin1);
DallasTemperature DTTempSensors1(&OWTempSensor1);

const int tempSensorPin2 = 31;
OneWire OWTempSensor2(tempSensorPin2);
DallasTemperature DTTempSensors2(&OWTempSensor2);

/******************************
 * EEPROM
 ******************************/
const int addrThreshold1 = 0;
const int addrTolerance = 1;
const int addrUnit = 2;

static byte threshold1 = 0;
static byte tolerance = 0;
static byte unit = 0;

static int EEPORMdataChanged = 0;

void readEEPORMData() {
    threshold1 = EEPROM.read(addrThreshold1);
    tolerance = EEPROM.read(addrTolerance);
    unit = EEPROM.read(addrUnit);
}

void writeEEPORMData() {
    if (EEPORMdataChanged) {
        EEPROM.write(addrThreshold1, threshold1);
        EEPROM.write(addrTolerance, tolerance);
        EEPROM.write(addrUnit, unit);
    }
    EEPORMdataChanged = 0;
}

/******************************
 * relay
 ******************************/
#define RELAY1_PIN 40
#define RELAY2_PIN 41 /* not used now */

static int manual1 = 0;

static int relayStatus1 = 0;
static int relayStatus2 = 0;

void initReley() {
    digitalWrite(RELAY1_PIN, HIGH);
    digitalWrite(RELAY2_PIN, HIGH);  
    pinMode(RELAY1_PIN, OUTPUT);
    pinMode(RELAY2_PIN, OUTPUT);
}

void switchOnRelay(int relayID){
    if (relayID == 1) {
        digitalWrite(RELAY1_PIN, LOW);
        relayStatus1 = 1;
    }

    if (relayID == 2) {
        digitalWrite(RELAY2_PIN, LOW);
        relayStatus2 = 1;
    }
}

void switchOffRelay(int relayID){
    if (relayID == 1) {
        digitalWrite(RELAY1_PIN, HIGH);
        relayStatus1 = 0;
    }

    if (relayID == 2) {
        digitalWrite(RELAY2_PIN, HIGH);
        relayStatus2 = 0;
    }
}

/******************************
 * LCD
 ******************************/
// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

int readButtons() {
    int adcKeyIn  = 0;
    adcKeyIn = analogRead(0);
    if (adcKeyIn > 1000) return btnNONE;

    if (adcKeyIn < 50)   return btnRIGHT;
    if (adcKeyIn < 250)  return btnUP;
    if (adcKeyIn < 450)  return btnDOWN;
    if (adcKeyIn < 650)  return btnLEFT;
    if (adcKeyIn < 850)  return btnSELECT;

 // For V1.0
/*
    if (adcKeyIn < 50)   return btnRIGHT;
    if (adcKeyIn < 195)  return btnUP;
    if (adcKeyIn < 380)  return btnDOWN;
    if (adcKeyIn < 555)  return btnLEFT;
    if (adcKeyIn < 790)  return btnSELECT;
*/
    return btnNONE;
}
 
/******************************
 * LCD MENU
 ******************************/

#define MENUELEMS 4
int menuElems = MENUELEMS;
char* menuItems[MENUELEMS] = {"Hranice: ", "Ochlad: ", "Jednotka: ", "Ventil: "};

byte degreeChar[8] = {
    0b00110, 
    0b01001, 
    0b01001,
    0b00110, 
    0b00000,
    0b00000,
    0b00000, 
    0b00000  
};

byte downArrow[8] = {
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b10101, // * * *
    0b01110, //  ***
    0b00100  //   *
};

byte upArrow[8] = {
    0b00100, //   *
    0b01110, //  ***
    0b10101, // * * *
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b00100  //   *
};

byte menuCursor[8] = {
    B01000, //  *
    B00100, //   *
    B00010, //    *
    B00001, //     *
    B00010, //    *
    B00100, //   *
    B01000, //  *
    B00000  //
};

long getTime() {
    return millis()/1000;
}

void lcdPrintUnit() {
    lcd.write(byte(0));
    if (unit == 0)
        lcd.print("C");
    else
        lcd.print("F");
}

void lcdPrintWater(){
    if (relayStatus1 == 0)
        lcd.print("Vyp");
    else
        lcd.print("Zap");
}

void lcdPrintManual(){
    if (manual1 == 0)
        lcd.print("Auto");
    if (manual1 == 1) {
        lcd.print("Zap");
        switchOnRelay(1);
    }
    if (manual1 == 2){
	    switchOffRelay(1);
        lcd.print("Vyp");
    }
}

void printMenuLine(int lineIndex) {
    lcd.write(menuItems[lineIndex]);
    switch (lineIndex) {
        case 0:
            lcd.print(threshold1);
            lcdPrintUnit();
            break;
        case 1:
            lcd.print(tolerance);
            lcdPrintUnit();
            break;
        case 2:
            lcdPrintUnit();
            break;
        case 3:
            lcdPrintManual();
            break;
    }
}

void printMenu(int menuIndex) {
    lcd.clear();
    lcd.setCursor(15,0);
    lcd.write(byte(2));
    lcd.setCursor(15,1);
    lcd.write(byte(1));

    int activeLine = menuIndex % 2;
    lcd.setCursor(0, activeLine);
    lcd.write(byte(3));

    if (activeLine == 0) {
        lcd.setCursor(1,0);
        printMenuLine(menuIndex);

        lcd.setCursor(1,1);
        printMenuLine(menuIndex + 1);
    } else {
        lcd.setCursor(1,0);
        printMenuLine(menuIndex - 1);

        lcd.setCursor(1,1);
        printMenuLine(menuIndex);
    }


}

void doMenu(){
    int lcdKey = 0;
    lcd.clear();
    int menuChanged = 1;
    do {lcdKey = readButtons();} while (lcdKey != btnNONE); // wait for releasing button
    
    long inactive_time = getTime();
    int menuIndex = 0;
    do {
        if (menuChanged) {
            printMenu(menuIndex);
            menuChanged = 0;
        }

        delay(200);

        lcdKey = readButtons();

        switch (lcdKey) {

            case btnDOWN:
                inactive_time = getTime();
                menuChanged = 1;
                menuIndex++;
                if (menuIndex == menuElems)
                    menuIndex = 0;
                break;

            case btnUP:
                inactive_time = getTime();
                menuChanged = 1;
                menuIndex--;
                if (menuIndex < 0)
                    menuIndex = menuElems-1;
                break;  

            case btnLEFT:
                inactive_time = getTime();
                menuChanged = 1;
                switch (menuIndex) {
                    case 0:
                        threshold1--;
                        EEPORMdataChanged = 1;
                        break;
                    case 1:
                        tolerance--;
                        EEPORMdataChanged = 1;
                        break;
                    case 2:
                        if (unit)
                            unit = 0;
                        else
                            unit = 1;
                        EEPORMdataChanged = 1;
                        break;
                    case 3:
                        manual1--;
                        if (manual1 < 0)
                            manual1 = 2;
                        break;
                }
                break; 

            case btnRIGHT:
                inactive_time = getTime();
                menuChanged = 1;
                switch (menuIndex) {
                    case 0:
                        threshold1++;
                        EEPORMdataChanged = 1;
                        break;
                    case 1:
                        tolerance++;
                        EEPORMdataChanged = 1;
                        break;
                    case 2:
                        if (unit)
                            unit = 0;
                        else
                            unit = 1;
                        EEPORMdataChanged = 1;
                        break;
                    case 3:
                        manual1++;
                        if (manual1 > 2)
                            manual1 = 0;
                        break;
                }
                break;                
        }
    } while (inactive_time + MENU_TIMEOUT >= millis()/1000 && lcdKey != btnSELECT);

    lcd.clear();

    writeEEPORMData();
}

/******************************
 * Utils
 ******************************/

double doUnit(double value) {
    if (unit)
        value = (9*value)/5 + 32;
    return value;
}

/******************************
 * Setup program
 ******************************/
void setup(void) {
    manual1 = 0;

    readEEPORMData();
    initReley();

    max.begin(MAX31865_2WIRE);  // set to 2WIRE or 3WIRE or 4WIRE as necessary

    DTTempSensors1.begin();
    DTTempSensors2.begin();

    lcd.begin(16, 2);
    lcd.setCursor(0,0);

    lcd.createChar(0, degreeChar);
    lcd.createChar(1, downArrow);
    lcd.createChar(2, upArrow);
    lcd.createChar(3, menuCursor);
}

/******************************
 * Main loop
 ******************************/
void loop(void) {
    int lcdKey = 0;

    float temp_platinum;

    float temp1;
    float temp2;

    temp_platinum = doUnit(max.temperature(RNOMINAL, RREF));

    DTTempSensors1.requestTemperatures(); // read temp from all connected sensors on selected pin
    temp1 = doUnit(DTTempSensors1.getTempCByIndex(0));

    DTTempSensors2.requestTemperatures(); // read temp from all connected sensors on selected pin
    temp2 = doUnit(DTTempSensors2.getTempCByIndex(0));

    lcd.setCursor(0,0);
    lcd.print(temp2);
    lcdPrintUnit();
    lcd.print("  ");


    lcd.setCursor(0,1);
    lcd.print(temp_platinum);
    lcdPrintUnit();
    lcd.print("  ");

    lcd.setCursor(9,1);
    lcd.print(temp1);
    lcdPrintUnit();
    lcd.print("  ");

    lcd.setCursor(9,0);
    lcd.print(threshold1);
    lcdPrintUnit();
    lcd.print("  ");

    lcd.setCursor(15,0);
    if (manual1 == 0) {
        if (threshold1 <= temp1) {
            if (relayStatus1 == 0)
                switchOnRelay(1);
        }
        if (threshold1 >= temp1 + tolerance) {
            if (relayStatus1 == 1)
                switchOffRelay(1);
        }

        if (relayStatus1 == 1)
            lcd.print("*");
        else
            lcd.print(" ");
    } else if (manual1 == 1) {
        lcd.print("Z");
    } else if (manual1 == 2) {
        lcd.print("V");
    }

    lcdKey = readButtons();
    if (lcdKey == btnSELECT) {
        doMenu();
    }

  delay(100);
}
