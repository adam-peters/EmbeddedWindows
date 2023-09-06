#include <M5Core2.h>
#include <ArduinoJson.h>
#include <M5Core2.h>
#include <cstdlib>
#include <ctime>
#include <map>
#include <list>
#include <HTTPClient.h>
#include <EEPROM.h>            // read and write from flash memory
#include <NTPClient.h>         // Time Protocol Libraries
#include <WiFiUdp.h>           // Time Protocol Libraries
#include <Adafruit_VCNL4040.h> // Sensor libraries
#include "Adafruit_SHT4x.h"    // Sensor libraries
#include "utility/MPU6886.h"
#include "WiFi.h"

////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////

// URL addresses
const String URL_GCF_UPLOAD = "https://data-upload-7sdvkb3baq-uw.a.run.app";
const String URL_GCF_RETRIEVE = "https://data-retrieve-1-7sdvkb3baq-uw.a.run.app";

// WiFi Credentials
String wifiNetworkName = "CBU";
String wifiPassword = "";

// Screen
int sWidth;
int sHeight;

int pixels[168][168]; // 168 * 168

// Structs
struct point {
    int x;
    int y;
};

// Enums
enum Screen
{
    dataScreen = 0,
    averageScreen,
};

// Screen Variables
int currentScreen = 0;

// List of Enum Lengths
int enums[] = {};

// Bonus Enum Variables
int currentlySelected[0];
int highlighted = 0;

// Offset
int vertOffset = 50;
int horOffset = 25;
int space = 10;

// Time (Delay) Variables
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;

// Accelerometer Variables
float accX;
float accY;
float accZ;

// Drawing Square Variables
int inputSize = 168;
point input;


// Button Size Variables
int buttonWidth = 50;
int buttonHeight = 50;
int buttonSpace = space + buttonWidth;

// Drawing Variables
int brushSize = 2;
point lastPressed;

////////////////////////////////////////////////////////////////////
// Button Zones
////////////////////////////////////////////////////////////////////

// Button
// point add;
// point sub;
// point mult;
// point div;

// point buttons[] = {add, sub, mult, div};

// Buttons
// Button a(add.x, add.y, buttonWidth, buttonHeight, "add");
// Button s(sub.x, sub.y, buttonWidth, buttonHeight, "sub");
// Button m(mult.x, mult.y, buttonWidth, buttonHeight, "mult");
// Button d(div.x, div.y, buttonWidth, buttonHeight, "div");


////////////////////////////////////////////////////////////////////
// Method header declarations
////////////////////////////////////////////////////////////////////

// Screens
void renderInputScreen();

// Data

// Render Methods
void draw(int x, int y);
void drawInputBox();
void drawMathButtons();

// Helper Methods
void interpolate(int x, int y);

// Button Methods
void buttonAction(Event &e);
void testButton();

////////////////////////////////////////////////////////////////////
// Setup
////////////////////////////////////////////////////////////////////

// Setup Method to Initialize Code
void setup()
{

    // Initialize the device
    M5.begin();
    M5.IMU.Init();

    // Get Width/Height
	sWidth = M5.Lcd.width();
	sHeight = M5.Lcd.height();

    input.x = 160 - (inputSize / 2);
    input.y = 5;

    lastPressed.x = 0;
    lastPressed.y = 0;

    // Button Handler   
    M5.Buttons.addHandler(buttonAction, E_TOUCH);   

    // SCREEN!
    renderInputScreen();
}

////////////////////////////////////////////////////////////////////
// Loop
////////////////////////////////////////////////////////////////////

void loop()
{
    M5.update();

    // Touch Buttons
    if (M5.BtnA.wasPressed())
    {
    }
    else if (M5.BtnB.wasPressed())
    {
    }
    else if (M5.BtnC.wasPressed())
    {
    }
    
    
    
    // Get accelerator data from sensor
    M5.IMU.getAccelData(&accX, &accY, &accZ);

    // Check to see if the user shakes device
    if((accX >= 2 || accY >= 2 || accZ >= 2)){
        lastPressed.x = 0;
        lastPressed.y = 0;
        drawInputBox();
    }

    //  
    TouchPoint_t coordinate;
    coordinate = M5.Touch.getPressPoint();

    // 
    if(M5.Touch.ispressed() && coordinate.x <= (input.x + inputSize - brushSize) && coordinate.x >= (input.x + brushSize) 
        && coordinate.y <= (input.y + inputSize - brushSize) && coordinate.y >= (input.y + brushSize)) {
        
        draw(coordinate.x, coordinate.y);
    }
}

////////////////////////////////////////////////////////////////////
// Screens
////////////////////////////////////////////////////////////////////

void renderInputScreen()
{
    // Clear Screen
    M5.Lcd.clear();

    drawInputBox();
    drawMathButtons();

}

////////////////////////////////////////////////////////////////////
// Drawing Methods
////////////////////////////////////////////////////////////////////

void draw(int x, int y)
{
    interpolate(x, y);
    M5.Lcd.fillCircle(x, y, brushSize, WHITE);
}

void interpolate(int x, int y) {\
    if(lastPressed.x != 0 && lastPressed.y != 0) {

        // Distances between Points
        int xDistance = (x - lastPressed.x);
        int yDistance = (y - lastPressed.y);

        // Number of circles to draw inbetween (The Manhaten Distance / BrushSize)
        int dist = fabs(xDistance) + fabs(yDistance);
        int n = dist / brushSize; 

        // Ratio for making horizontal and vertical components more even

        if(n != 0) {

            // Places to draw circles
            xDistance /= n;
            yDistance /= n;

            // Loop to draw Circles
            for(int i = 1; i < n; i++) {
                M5.Lcd.fillCircle(xDistance * i + (lastPressed.x), yDistance * i + (lastPressed.y), brushSize, WHITE);
            }
        }
        
    } 

    lastPressed.x = x;
    lastPressed.y = y;
}

void drawInputBox() {
    // Clear Drawing
    M5.Lcd.fillRect(input.x, input.y, inputSize, inputSize, BLACK);

    // Draw Box
    M5.Lcd.drawRect(input.x, input.y, inputSize, inputSize, WHITE);
}

void drawMathButtons() {
    // How many buttons, affects how big they are
    int n = 4;

    int bins = sWidth / n;
    buttonWidth = bins - space;

    // Need to add symbols for any new buttons as well
    String symbols[n] = {"+", "-", "*", "/"};

    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextFont(3);
    
    int x = 0;
    int y = 0;
    // Loop to Draw Buttons
    for(int i = 0; i < n; i++) {

        // x and y
        x = space + (buttonWidth * i) + (space * i);
        y = sHeight - (buttonHeight);
        
        // Draw Button
        M5.Lcd.drawRoundRect(x, y, buttonWidth, buttonHeight, 10,  WHITE);

        // Add Button Pos
        // buttons[i].x = x;
        // buttons[i].y = y;

        // Write Symbol in Button
        M5.Lcd.drawString(symbols[i], x + (buttonWidth / 2), sHeight - (buttonHeight / 2), 2);
    }
}

void highlightButton(int b) {

}

////////////////////////////////////////////////////////////////////
// Text Methods
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Helper Methods
////////////////////////////////////////////////////////////////////

void incrementEnum()
{
    currentlySelected[highlighted] = currentlySelected[highlighted] + 1;
    if (currentlySelected[highlighted] >= enums[highlighted])
    {
        currentlySelected[highlighted] = currentlySelected[highlighted] % enums[highlighted];
    }
}

void decrementEnum()
{
    currentlySelected[highlighted] = currentlySelected[highlighted] - 1;
    if (currentlySelected[highlighted] < 0)
    {
        currentlySelected[highlighted] = enums[highlighted] - 1;
    }
}

////////////////////////////////////////////////////////////////////
// Button Handler and Testing
////////////////////////////////////////////////////////////////////

void buttonAction(Event &e)
{

    // Button Variable
    Button &b = *e.button;
    //Screen Button
    if (b.instanceIndex() == 3) {

    }
    // Top Button
    else if (currentScreen == averageScreen && b.instanceIndex() == 4)
    {
    }
    // Middle Button
    else if (currentScreen == averageScreen && b.instanceIndex() == 5)
    {
    }
    // Bottom Button
    else if (currentScreen == averageScreen && b.instanceIndex() == 6)
    {
    }
    // End Button
    else if (b.instanceIndex() == 7)
    {
    }
}

// void testButton()
// {

//     // Users
//     M5.Lcd.fillRect(xPos, topButtonY, buttonWidth, buttonHeight, YELLOW);
//     // Time
//     M5.Lcd.fillRect(xPos, middleButtonY, buttonWidth, buttonHeight, YELLOW);
//     // Data
//     M5.Lcd.fillRect(xPos, bottomButtonY, buttonWidth, buttonHeight, YELLOW);

//     // Enter Button
//     M5.Lcd.fillRect(middleXPos, smallBottom, buttonWidth - 200, buttonHeight, YELLOW);
// }