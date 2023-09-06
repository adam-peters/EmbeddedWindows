#include <M5Core2.h>
#include "../include/I2C_RW.h"

///////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////
int const PIN_SDA = 32;
int const PIN_SCL = 33;

///////////////////////////////////////////////////////////////
// Register defines
///////////////////////////////////////////////////////////////
#define VCNL_I2C_ADDRESS 0x60
#define VCNL_REG_PROX_DATA 0x08
#define VCNL_REG_ALS_DATA 0x09
#define VCNL_REG_WHITE_DATA 0x0A

#define VCNL_REG_PS_CONFIG 0x03
#define VCNL_REG_ALS_CONFIG 0x00
#define VCNL_REG_WHITE_CONFIG 0x04

///////////////////////////////////////////////////////////////
// Put your setup code here, to run once
///////////////////////////////////////////////////////////////
void setup() {

    // Init device
    M5.begin();

    // Connect to VCNL sensor
    I2C_RW::initI2C(VCNL_I2C_ADDRESS, 400000, PIN_SDA, PIN_SCL);
    //I2C_RW::scanI2cLinesForAddresses(false);

	// Write registers to initialize/enable VCNL sensors
    I2C_RW::writeReg8Addr16DataWithProof(VCNL_REG_PS_CONFIG, 2, 0x0800, " to enable proximity sensor", true);
    I2C_RW::writeReg8Addr16DataWithProof(VCNL_REG_ALS_CONFIG, 2, 0x0000, " to enable ambient light sensor", true);
    I2C_RW::writeReg8Addr16DataWithProof(VCNL_REG_WHITE_CONFIG, 2, 0x0000, " to enable raw white light sensor", true);
}

///////////////////////////////////////////////////////////////
// Put your main code here, to run repeatedly
///////////////////////////////////////////////////////////////
void loop()
{
    // I2C call to read sensor proximity data and print
    int prox = I2C_RW::readReg8Addr16Data(VCNL_REG_PROX_DATA, 2, "to read proximity data", false);
    Serial.printf("Proximity: %d\n", prox);

	// I2C call to read sensor ambient light data and print
    int als = I2C_RW::readReg8Addr16Data(VCNL_REG_ALS_DATA, 2, "to read ambient light data", false);
    als = als * 0.1; // See pg 12 of datasheet - we are using ALS_IT (7:6)=(0,0)=80ms integration time = 0.10 lux/step for a max range of 6553.5 lux
    Serial.printf("Ambient Light: %d\n", als);

	// I2C call to read white light data and print
    int rwl = I2C_RW::readReg8Addr16Data(VCNL_REG_WHITE_DATA, 2, "to read white light data", false);
    rwl = rwl * 0.1;
    Serial.printf("White Light: %d\n\n", rwl);

	// Delay so we aren't reading sensors too frequently
    delay(1000);
}