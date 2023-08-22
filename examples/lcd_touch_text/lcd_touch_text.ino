#include "WEA2012_LCD.h"
#include "WEA2012_Touch.h"

#define QSPI_SCK 5
#define QSPI_0 14
#define QSPI_1 8
#define QSPI_2 0
#define QSPI_3 1
#define QSPI_CS 10
#define LCD_RST 6

#define I2C_SDA 3
#define I2C_SCL 9
#define TP_RST 4 
#define TP_INT 11

#define LED 13 //背光

bool FingerNum;
uint16_t touchX,touchY;

WEA2012_LCD lcd = WEA2012_LCD(QSPI_SCK,QSPI_0,QSPI_1,QSPI_2,QSPI_3,QSPI_CS,LCD_RST);
WEA2012_Touch touch = WEA2012_Touch(I2C_SDA, I2C_SCL,TP_RST,TP_INT);

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
    
    delay(200);
}