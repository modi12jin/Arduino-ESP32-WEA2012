#include "SPD2010_LCD.h"
#include "SPD2010_Touch.h"

#define QSPI_SCK 8
#define QSPI_0 13
#define QSPI_1 14
#define QSPI_2 1
#define QSPI_3 9
#define QSPI_CS 10
#define LCD_RST 6

#define I2C_SDA 4
#define I2C_SCL 3
#define TP_RST 12 
#define TP_INT 11

#define LED 5 //背光

bool FingerNum;
uint16_t touchX,touchY;

SPD2010_LCD lcd = SPD2010_LCD(QSPI_SCK,QSPI_0,QSPI_1,QSPI_2,QSPI_3,QSPI_CS,LCD_RST);
SPD2010_Touch touch = SPD2010_Touch(I2C_SDA, I2C_SCL,TP_RST,TP_INT);

void setup() {
Serial.begin(115200);
lcd.begin();
pinMode(LED, OUTPUT);
digitalWrite(LED, HIGH); //高电平
touch.begin();
}

void loop() {
    FingerNum=touch.getTouch(&touchX,&touchY);
    if(FingerNum){
              Serial.printf("X:%d,Y:%d\n",touchX,touchY);
    }
    
    delay(100);
}