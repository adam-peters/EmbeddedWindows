#include <M5Core2.h>
#include <Wire.h>

///////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////
const int I2C_SDA_PIN = 32; // 21 for internal; 32 for port A
const int I2C_SCL_PIN = 33; // 22 for internal; 33 for port A
const int I2C_FREQ = 400000;

///////////////////////////////////////////////////////////////
// Put your setup code here, to run once
///////////////////////////////////////////////////////////////
void setup() {
    // Init device
    M5.begin();

    // Initialize I2C port

    
    // I2C Variables

    // Enable I2C connection
    
    // Prepare and write address (command code)
    
    // End transmission (not writing...reading)
    
    // Request to read the data from the device/register addressed above
    
    // Grab the data from the data line
    

}

///////////////////////////////////////////////////////////////
// Put your main code here, to run repeatedly
///////////////////////////////////////////////////////////////
void loop() {
}