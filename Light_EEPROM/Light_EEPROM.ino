/*
 * Copyright (c) 2016, CESAR.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 *
 */

#include <KNoTThing.h>
#include <SPI.h>

#define CHIP_SELECT         8
#define HOLD_PIN            3
#define WRITE_PROTECT_PIN   4

#define LIGHT_BULB_PIN      2
#define LIGHT_BULB_ID       1
#define LIGHT_BULB_NAME     "Light bulb"

#define WRITE 0b00000010
#define READ  0b00000011
#define WREN  0b00000110
#define WRDI  0b00000100
#define WRSR  0b00000001
#define RDSR  0b00000101

#define ZEROS 0b00000000
#define DATA  0b01010100
#define ADDR  0b0000000000011010

bool ctr = false;
byte status_reg = 0b11100000;

KNoTThing thing;

void eeprom_spi_write_enable(){
  SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));
  
  Serial.println("Setting Enable Writing");

  digitalWrite(CHIP_SELECT, LOW);
  SPI.transfer(WREN);
  digitalWrite(CHIP_SELECT, HIGH);
   
  SPI.endTransaction();
}

void eeprom_spi_write_disable(){
  SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));

  Serial.println("Disable Writing");

  digitalWrite(CHIP_SELECT, LOW);
  SPI.transfer(WRDI);
  digitalWrite(CHIP_SELECT, HIGH);
  
  SPI.endTransaction();
}

void eeprom_spi_read_status(){
  SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));
  byte reg_data;
  
  digitalWrite(CHIP_SELECT, LOW);
  SPI.transfer(RDSR);
  SPI.transfer(ZEROS);
  reg_data = SPI.transfer(ZEROS);
  digitalWrite(CHIP_SELECT, HIGH);

  Serial.print("STATUS REGISTER: ");
  Serial.println(reg_data, BIN);
  
  SPI.endTransaction();
}

void eeprom_spi_write_status(byte status_reg){
  SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));

  Serial.println("Enable Writing");

  digitalWrite(CHIP_SELECT, LOW);
  SPI.transfer(WRSR);
  SPI.transfer(ZEROS);
  SPI.transfer(status_reg);
  digitalWrite(CHIP_SELECT, HIGH);
  
  SPI.endTransaction();
}


void eeprom_spi_read_reg(word addr){
  byte data_reg;

  SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));

  Serial.println("Reading Register");
  
  digitalWrite(CHIP_SELECT, LOW);
  SPI.transfer(READ);
  SPI.transfer(ZEROS);
  SPI.transfer(addr);
  data_reg = SPI.transfer(ZEROS);
  digitalWrite(CHIP_SELECT, HIGH);

  Serial.print("REGISTER Value: ");
  Serial.println(data_reg, BIN);
  
  SPI.endTransaction();
}

void eeprom_spi_write_reg(word addr, byte data){
  SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));

  Serial.println("Writing on Register");
  Serial.print("Address: ");
  Serial.println(addr, BIN);
  Serial.print("Data: ");
  Serial.println(data, BIN);
  
  digitalWrite(CHIP_SELECT, LOW);
  SPI.transfer(WRITE);
  SPI.transfer(ZEROS);
  SPI.transfer(addr);
  SPI.transfer(data);
  digitalWrite(CHIP_SELECT, HIGH);
  
  SPI.endTransaction();
}

static int light_read(uint8_t *val)
{
    *val = digitalRead(LIGHT_BULB_PIN);
//    Serial.print(F("Light Status: "));
//    if (*val)
//      Serial.println(F("ON"));
//    else
//      Serial.println(F("OFF"));
    return 0;
}

static int light_write(uint8_t *val)
{
    digitalWrite(LIGHT_BULB_PIN, *val);
//    Serial.print(F("Light Status: "));
//    if (*val)
//      Serial.println(F("ON"));
//    else
//      Serial.println(F("OFF"));
      /* TODO: Save light status in EEMPROM in to handle when reboot */
    return 0;
}

void setup()
{
    Serial.begin(9600);

    pinMode(LIGHT_BULB_PIN, OUTPUT);
    /* TODO: Read lamp status from eeprom for reboot cases */
    thing.init("KNoTThing");
    thing.registerBoolData(LIGHT_BULB_NAME, LIGHT_BULB_ID, KNOT_TYPE_ID_SWITCH,
        KNOT_UNIT_NOT_APPLICABLE, light_read, light_write);

    thing.registerDefaultConfig(LIGHT_BULB_ID);

    Serial.println(F("Remote Light Bulb KNoT Demo"));

    pinMode(WRITE_PROTECT_PIN, OUTPUT);
    digitalWrite(WRITE_PROTECT_PIN, HIGH);  // give write permission

    pinMode(HOLD_PIN, OUTPUT);
    digitalWrite(HOLD_PIN, HIGH); // disable hold so i can perform operations
    
     SPI.begin();
     pinMode(CHIP_SELECT, OUTPUT);
     digitalWrite(CHIP_SELECT, HIGH);
}


void loop()
{

    thing.run();

    char std = '0';
  
    if (!ctr){
        Serial.println("MENU:");
        Serial.println("0 - Display STATUS register");
        Serial.println("1 - Display ENABLE Writing");
        Serial.println("2 - Display WRITES in a register");
        Serial.println("3 - Display DISABLE Writing");
        delay(1000);
        
        ctr = true;
      }
      
     if(Serial.available()){
       std = Serial.read();    
      }

    digitalWrite(HOLD_PIN, HIGH);
    switch(std)
    {
      case '0':
        cli();
        eeprom_spi_read_status();
        sei();
        std='1';
        break;
      case '1':
        cli();
        eeprom_spi_write_enable();
        eeprom_spi_write_status(status_reg);
        sei();
        std='2';
        break;
      case '2':
        cli();
        eeprom_spi_write_enable();
        eeprom_spi_write_reg(ADDR,DATA);
        eeprom_spi_read_reg(ADDR);
        sei();
        Serial.println("REMEBER: After a write operation the WRITING IS DISABLE");
        std = '0';
        break;
      case '3':
        eeprom_spi_write_disable();
        std = '0';
        break;
        
      default:
        
        ctr = false; 
        std = '0';
                
        break;
        
    }
    digitalWrite(HOLD_PIN, LOW);  
}
