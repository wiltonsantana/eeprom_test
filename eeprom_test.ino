#include <SPI.h>
#define CHIP_SELECT         8
#define HOLD_PIN            3
#define WRITE_PROTECT_PIN   4

#define WRITE 0b00000010
#define READ  0b00000011
#define WREN  0b00000110
#define WRDI  0b00000100
#define WRSR  0b00000001
#define RDSR  0b00000101

#define ZEROS 0b00000000
#define DATA  0b01010100
word i;
bool ctr = false;
byte status_reg = 0b11100000;
word addr = 0b0000000000000010;
//byte data = 0b01010101;

void setup() {
      Serial.begin(9600);

    pinMode(WRITE_PROTECT_PIN, OUTPUT);
    digitalWrite(WRITE_PROTECT_PIN, HIGH);  // give write permission

    pinMode(HOLD_PIN, OUTPUT);
    digitalWrite(HOLD_PIN, HIGH); // disable hold so i can perform operations

     SPI.begin();
     pinMode(CHIP_SELECT, OUTPUT);
     digitalWrite(CHIP_SELECT, HIGH);
}

void loop() {
  char std = '0';
  
  if (!ctr){
      Serial.println("MENU:");
      Serial.println("0 - Display STATUS register");
      Serial.println("1 - Display ENABLE Writing");
      Serial.println("2 - Display WRITES in a register");
      Serial.println("3 - Display DISABLE Writing");
      delay(10000);
      
      ctr = true;
    }
    
 if(Serial.available()){
   std = Serial.read();    
  }

  switch(std)
  {
    case '0':
      eeprom_spi_read_status();
      delay(100);
      break;
    case '1':
      cli();
      eeprom_spi_write_enable();
      sei();
      delay(100);
      cli();
      eeprom_spi_write_status(status_reg);
      sei();
      delay(100);
      std='0';
      break;
    case '2':
      cli();
      eeprom_spi_write_status(status_reg);
      sei();
      delay(100);
      cli();
      eeprom_spi_write_enable();
      delay(100);
      eeprom_spi_write_reg(addr,DATA);
      delay(500);
      eeprom_spi_read_reg(addr);
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
}

void eeprom_spi_write_enable(){
  //Enable writing before every writing command
  SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));
  
  Serial.println("Setting Enable Writing");

  digitalWrite(CHIP_SELECT, LOW);
  delay(10);
  SPI.transfer(WREN);
  delay(10);
  digitalWrite(CHIP_SELECT, HIGH);

  //delay(10);
  // After this condition Writing enable will be reset
  //    Power Up
  //    WRDI instruction
  //    WRSR instruction
  //    WRITE instruction  
  SPI.endTransaction();
}

void eeprom_spi_write_disable(){
  //Enable writing before every writing command
  SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));

  Serial.println("Disable Writing");

  digitalWrite(CHIP_SELECT, LOW);
  SPI.transfer(WRDI);
  digitalWrite(CHIP_SELECT, HIGH);

  delay(10);
  SPI.endTransaction();
}

void eeprom_spi_write_status(byte status_reg){
  SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));

  word cmd;

  Serial.println("Enable Writing");

  cmd = 0b0000000110001100;

  digitalWrite(CHIP_SELECT, LOW);
  delay(10);
  SPI.transfer(WRSR);
  SPI.transfer(ZEROS);
  SPI.transfer(status_reg);
  digitalWrite(CHIP_SELECT, HIGH);
  
  SPI.endTransaction();
}

void eeprom_spi_read_status(){
  SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));
  byte reg_data;
  byte cmd;
  //byte status_reg;

  cmd = 0b00000101;

  digitalWrite(CHIP_SELECT, LOW);
  SPI.transfer(cmd);
  cmd = 0b00000000;
  SPI.transfer(ZEROS);
  reg_data = SPI.transfer(ZEROS);
  digitalWrite(CHIP_SELECT, HIGH);
  delay(10);

  Serial.print("STATUS REGISTER: ");
  Serial.println(reg_data, BIN);
  
  SPI.endTransaction();
}

bool eeprom_spi_check_writing(){
  SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));

  byte cmd;
  bool result;
  byte status_reg;

  cmd = 0b00000101;

  digitalWrite(CHIP_SELECT, LOW);
  status_reg = SPI.transfer(cmd);
  
  delay(10);

  Serial.print("STATUS REGISTER: ");
  Serial.println(status_reg, BIN);
  
  SPI.endTransaction();

  result = (status_reg >> 2) & 0;
  // Check if write enable lauch is disable 
  // which means that there was a writing 
  // that occurs successfully before

  return result;
}

void eeprom_spi_write_reg(word addr, byte data){
  SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));

  Serial.println("Writing on Register");
  Serial.print("Address: ");
  Serial.println(addr, BIN);
  Serial.print("Data: ");
  Serial.println(data, BIN);
  
  digitalWrite(CHIP_SELECT, LOW);
  delay(10);
  SPI.transfer(WRITE);
  SPI.transfer(ZEROS);
  SPI.transfer(addr);
  SPI.transfer(data);
  digitalWrite(CHIP_SELECT, HIGH);
  
  SPI.endTransaction();
}

void eeprom_spi_read_reg(word addr){
  byte data_reg;

  SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));

  Serial.println("Reading Register");
  
  digitalWrite(CHIP_SELECT, LOW);
  delay(10);
  SPI.transfer(READ);
  SPI.transfer(ZEROS);
  SPI.transfer(addr);
  data_reg = SPI.transfer(ZEROS);
  digitalWrite(CHIP_SELECT, HIGH);

  Serial.print("REGISTER Value: ");
  Serial.println(data_reg, BIN);
  
  SPI.endTransaction();
}

