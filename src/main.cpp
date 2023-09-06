#include <M5Core2.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <cstdlib>
#include <ctime>
#include <list>

#include "Wifi.h"

#include "internet.h"
#include "timer.h"
#include "network.h"

////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////

int n;
int ssidLength = 26;
int thisPage = 0;
int selectedNetwork = 0;
const int pageSize = 4;
bool wifiPage = false;
bool leftLocked = false;
bool rightLocked = false;
String wifiName = "";
String wifiPassword = "";
std::list<String> networkList;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

bool alarmEnabled = false;
unsigned long alarmTime = 0;

int delayInSeconds = 0;

RTC_TimeTypeDef TimeStruct;

// LCD variables
int sWidth;
int sHeight;

// Structs
struct point
{
    int x;
    int y;
};

struct size
{
    int hor;
    int vert;
};

// Enums
enum windowType{
    Internet,
    Paint,
    Clock,
    Wifi,
};

windowType currentWindow = Internet;

// Mouse
point mousePos = {
    0,
    0,
};

point lastMousePos = {
    0,
    0,
};

int tBarHeight = 30;
int offset = 5;

// Window 
point smallWinPos = {
    60,
    25,
};

size smallWinSize = {
    250,
    160,
};

// Start Button
point startButtonPos = {
    
};

size startSize = {
    50,
    tBarHeight - 10,
};


// Start Menu
size startMenuSize = {
    100,
    120,
};

bool isStartMenuOpen = false;
bool isWindowOpen = false;

int clockColor = WHITE;

// API
String apiURL = "https://api.kanye.rest";
String kanyeQuote = "";

// Sprites
TFT_eSprite screen = TFT_eSprite(&M5.Lcd);
TFT_eSprite mouse = TFT_eSprite(&M5.Lcd);
TFT_eSprite window = TFT_eSprite(&M5.Lcd);
TFT_eSprite taskbar = TFT_eSprite(&M5.Lcd);
TFT_eSprite icons = TFT_eSprite(&M5.Lcd);
TFT_eSprite displayClock = TFT_eSprite(&M5.Lcd);



////////////////////////////////////////////////////////////////////
// Button Zone Declarations
////////////////////////////////////////////////////////////////////

Button s(0, sHeight - tBarHeight, startSize.hor + 10, startSize.vert + 10, "start");
Button i(15, 15, 35, 35, "internet");
Button p(15, 55, 35, 35, "paint");
Button w(15, 95, 35, 35, "wifi");
Button c(270, 20, 35, 35, "close");
Button u(185, 80, 40, 40, "up");
Button d(125, 80, 40, 40, "down");
Button e(155, 125, 60, 40, "enter");
Button sc(150, 125, 65, 35);

////////////////////////////////////////////////////////////////////
// Method header declarations
////////////////////////////////////////////////////////////////////

// Button Methods
void buttonAction(Event &e);
void drawButtons();

// Render Methods
void drawScreen();
void drawBackground();
void drawInternet();
void drawIcons();
void drawClock();
void drawWindow(struct point pos, struct size windowSize);
void drawTaskBar();
void drawStartButton();
void drawStartMenu();

void deleteSprites();
void fetchQuote();
String httpGETRequest(const char* serverName);


void setInitialTime();
void setAlarmTime(int delay);

void DrawMenu();
void LCD_Clear();
void Show(int nav);
void Search();
void Select();
void IncrementNum();
void DecrementNum();

void setup() {

    WiFi.disconnect();

    // Initialize the device
    M5.begin();

    // Get Width/Height
    sWidth = M5.Lcd.width();
    sHeight = M5.Lcd.height();

    // Button Handler   
    M5.Buttons.addHandler(buttonAction, E_TOUCH);   
    
    // WiFi.begin(wifiName.c_str(), wifiPassword.c_str());
    // while(WiFi.status() != WL_CONNECTED){
    //     delay(500);
    // }

    // On Startup
    drawScreen();
}

void loop() {

    M5.update();

    if(WiFi.status() == WL_CONNECTED) {
        timeClient.update();
    }

    M5.Rtc.GetTime(&TimeStruct);

    TouchPoint_t coordinate;
    coordinate = M5.Touch.getPressPoint();

    if(alarmTime - millis() < 10000) {
        clockColor = RED;
    }

    if (alarmEnabled && (millis() > alarmTime))
    {
        screen.setColorDepth(8);
        screen.createSprite(sWidth, sHeight - tBarHeight);
        screen.fillSprite(RED);

        screen.setTextColor(WHITE);
        screen.setTextDatum(MC_DATUM);
        screen.drawString("Time's Up", 160, 120, 2);

        screen.pushSprite(0, 0);

        Serial.println("alarm will play");
        M5.Spk.begin();
        for(int i = 0; i < 3; i++) M5.Spk.DingDong();
        alarmEnabled = false;

        delay(250);

        clockColor = WHITE;

        drawScreen();

    }

    if(M5.Touch.ispressed() && coordinate.x > 0 && coordinate.x < 320 && coordinate.y > 0 && coordinate.y < 240) {

        mousePos = {
            coordinate.x,
            coordinate.y,
        };

        drawScreen();

        // things to minimize redrawing

        // bool redrawn = false;

        // Icon Position
        // if((mousePos.x < 50 && mousePos.x > 0 && mousePos.y < 150 && mousePos.y > 0) || 
        //     (lastMousePos.x < 50 && lastMousePos.x > 0 && lastMousePos.y < 150 && lastMousePos.y > 0)) {
        //         redrawn = true;
        //         drawIcons();
        //     }
        // if((mousePos.x < 320 && mousePos.x > 0 && mousePos.y < 240 && mousePos.y > 210) || 
        //     (lastMousePos.x < 320 && lastMousePos.x > 0 && lastMousePos.y < 240 && lastMousePos.y > 210)) {
        //         redrawn = true;
        //         drawTaskBar();
        //     }
        // if(isWindowOpen){
        //         if((mousePos.x < 310 && mousePos.x > 60 && mousePos.y < 185 && mousePos.y > 25) || 
        //         (lastMousePos.x < 310 && lastMousePos.x > 60 && lastMousePos.y < 185 && lastMousePos.y > 25)) {
        //             redrawn = true;
        //             drawWindow(smallWinPos, smallWinSize);
        //         }
        //     }
        
        // if(!redrawn) {
        //     drawScreen();
        // } else {
        //     redrawn = false;
        // }
        
        
        // set old mouse pos
        lastMousePos = mousePos;

        mouse.setColorDepth(8);
        mouse.createSprite(sWidth, sHeight);
        mouse.fillSprite(TFT_TRANSPARENT);

        // Draw Mouse
        mouse.fillTriangle(mousePos.x, mousePos.y, mousePos.x + 9, mousePos.y + 13, mousePos.x - 2, mousePos.y + 16, WHITE);
        mouse.drawTriangle(mousePos.x, mousePos.y, mousePos.x + 9, mousePos.y + 13, mousePos.x - 2, mousePos.y + 16, BLACK);

        mouse.pushSprite(0, 0, TFT_TRANSPARENT);

        // drawButtons();

        deleteSprites();
    }

    if(!isWindowOpen) {
        drawClock();
    }

    if(currentWindow == Clock && M5.BtnA.wasPressed()) {
        if ((delayInSeconds - 5) < 0)
        {
            delayInSeconds = 0;
        }
        else
        {
            delayInSeconds -= 5;
        }
        Serial.println(delayInSeconds);
        delay(100);  
        drawWindow(smallWinPos, smallWinSize);
    } else if (currentWindow == Wifi && M5.BtnA.wasPressed()) {
        DecrementNum();
    } else if (currentWindow == Clock && M5.BtnB.wasPressed()) {
        setAlarmTime(delayInSeconds);
        delay(100);
        isWindowOpen = false;
        drawScreen();
    } else if (currentWindow == Internet && M5.BtnB.wasPressed()) {
        fetchQuote();
        delay(100);
        drawWindow(smallWinPos, smallWinSize);
    } else if (currentWindow == Wifi && M5.BtnB.wasPressed()) {
        Select();
    } else if (currentWindow == Clock && M5.BtnC.wasPressed()) {
        delayInSeconds += 5;
        Serial.println(delayInSeconds);
        delay(100);
        drawWindow(smallWinPos, smallWinSize);
    } else if (currentWindow == Wifi && M5.BtnC.wasPressed()) {
        IncrementNum();
    } 
}

void fetchQuote() {
    String response = httpGETRequest(apiURL.c_str());
    Serial.println(response);
    const size_t jsonCapacity = 264;
    DynamicJsonDocument objResponse(jsonCapacity);

    DeserializationError error = deserializeJson(objResponse, response);
    if (error) {
        return;
    }


    String quote = objResponse["quote"];
    kanyeQuote = quote;
}

String httpGETRequest(const char* serverName) {
    HTTPClient http;
    http.begin(serverName);

    int httpResponseCode = http.GET();
    String response = http.getString();

    http.end();
    return response;
}

void deleteSprites() {

    screen.deleteSprite();
    mouse.deleteSprite();
    window.deleteSprite();
    taskbar.deleteSprite();
    icons.deleteSprite();
    displayClock.deleteSprite();
}

void drawScreen() {
    drawBackground();
    drawIcons();  
    drawTaskBar();

    if(isWindowOpen) {
        drawWindow(smallWinPos, smallWinSize);
    } else {
        drawClock();
    }
}

void drawBackground() {
    screen.setColorDepth(8);
    screen.createSprite(sWidth, sHeight - tBarHeight);
    screen.fillSprite(CYAN);

    screen.pushSprite(0, 0);  
}

void drawIcons() {
    icons.setColorDepth(8);
    icons.createSprite(60, sHeight - tBarHeight);
    icons.fillSprite(CYAN);

    icons.pushImage(20, 20, 24, 24, network);    
    icons.pushImage(20, 60, 24, 24, internet);
    icons.pushImage(20, 100, 24, 24, timer);

    icons.pushSprite(0, 0, TFT_BLACK);
}

void drawClock() {

    displayClock.setColorDepth(8);
    displayClock.createSprite(240, 30);
    displayClock.fillSprite(CYAN);

    displayClock.setCursor(0, 0);
    displayClock.setTextSize(3);
    displayClock.setTextColor(clockColor);
    displayClock.printf("%02d : %02d : %02d\n", TimeStruct.Hours, TimeStruct.Minutes, TimeStruct.Seconds);

    displayClock.pushSprite(75, 100, TFT_TRANSPARENT);

    displayClock.deleteSprite();
}

void drawTaskBar() {

    taskbar.setColorDepth(8);
    taskbar.createSprite(sWidth, tBarHeight);
    taskbar.fillSprite(TFT_TRANSPARENT);

    taskbar.fillRect(0, 0, sWidth, tBarHeight, LIGHTGREY);
    taskbar.drawLine(0, 0, sWidth, 0, BLACK);

    drawStartButton();

    if(isWindowOpen) {

        taskbar.fillRect(offset*2 + startSize.hor, offset, startSize.hor*2, startSize.vert, DARKGREY);

        switch(currentWindow) {
            case Internet:
                taskbar.setTextColor(BLACK);
                taskbar.setTextDatum(MC_DATUM);
                taskbar.drawString("Internet", offset*2 + (startSize.hor*2), (tBarHeight / 2), 2);
                break;
            case Paint:
                taskbar.setTextColor(BLACK);
                taskbar.setTextDatum(MC_DATUM);
                taskbar.drawString("Paint", offset*2 + (startSize.hor*2), (tBarHeight / 2), 2);
                break;
            case Clock:
                taskbar.setTextColor(BLACK);
                taskbar.setTextDatum(MC_DATUM);
                taskbar.drawString("Clock", offset*2 + (startSize.hor*2), (tBarHeight / 2), 2);
                break;
            case Wifi:
                taskbar.setTextColor(BLACK);
                taskbar.setTextDatum(MC_DATUM);
                taskbar.drawString("Wifi", offset*2 + (startSize.hor*2), (tBarHeight / 2), 2);
                break;
        }
    }

    taskbar.pushSprite(0, sHeight - tBarHeight, TFT_TRANSPARENT);

}

void drawStartButton() {

    taskbar.setTextColor(BLACK);
    taskbar.setTextDatum(MC_DATUM);

    taskbar.fillRect(offset, offset, startSize.hor, startSize.vert, DARKGREY);
    taskbar.drawString("Start", offset + (startSize.hor / 2), (tBarHeight / 2), 2);
}

void drawStartMenu() {

    // Draw BG
    screen.fillRect(0, sHeight - (tBarHeight + startMenuSize.vert), startMenuSize.hor, startMenuSize.vert, LIGHTGREY);
    // Outline
    screen.drawRect(0, sHeight - (tBarHeight + startMenuSize.vert), startMenuSize.hor, startMenuSize.vert, WHITE);
}

void drawWindow(struct point pos, struct size windowSize) {    

    window.setColorDepth(8);
    window.createSprite(windowSize.hor, windowSize.vert);
    window.fillSprite(TFT_TRANSPARENT);

    // Window BG
    window.fillRect(0, 0, windowSize.hor, windowSize.vert, LIGHTGREY);

    // Outline
    window.drawRect(0, 0, windowSize.hor, windowSize.vert, WHITE);

    // Internet BG
    window.fillRect(offset, 25, smallWinSize.hor - 10, smallWinSize.vert - 30, WHITE);

    switch(currentWindow) {
        case Internet:
            // Top Bar
            window.fillRect(offset, offset, windowSize.hor - 10, 20, BLUE);
            window.drawRect(offset, offset, windowSize.hor - 10, 20, DARKGREY);
            // Window Name
            // window.set
            window.setTextColor(WHITE);
            window.setTextDatum(MC_DATUM);
            window.drawString("Internet", windowSize.hor / 2, offset*3, 2);

            if(WiFi.status() == WL_CONNECTED) {
                drawInternet();
            } else {
                window.setTextColor(BLACK);
                window.setTextDatum(MC_DATUM);
                window.drawString("Wifi Not Connected...", smallWinSize.hor / 2, smallWinSize.vert / 2);
            }
            break;
        case Paint:
            // Top Bar
            window.fillRect(offset, offset, windowSize.hor - 10, 20, DARKCYAN);
            window.drawRect(offset, offset, windowSize.hor - 10, 20, DARKGREY);
            // Window Name
            window.setTextColor(WHITE);
            window.setTextDatum(MC_DATUM);
            window.drawString("Paint", windowSize.hor / 2, offset*3, 2);
            // Drawing
            window.fillRect(offset, 25, smallWinSize.hor - 10, smallWinSize.vert - 30, MAGENTA);
            window.fillRect(windowSize.hor / 2 - 5, windowSize.vert / 2 + 10, 10, 30, GREEN);
            window.fillCircle(windowSize.hor / 2, windowSize.vert / 2, 22, GREEN);
            window.fillCircle(windowSize.hor / 2, windowSize.vert / 2, 20, YELLOW);
            window.fillCircle(windowSize.hor / 2, windowSize.vert / 2, 10, WHITE);
            break;
        case Clock:
            // Top Bar
            window.fillRect(offset, offset, windowSize.hor - 10, 20, BLACK);
            window.drawRect(offset, offset, windowSize.hor - 10, 20, DARKGREY);
            // Window Name
            window.setTextColor(WHITE);
            window.setTextDatum(MC_DATUM);
            window.drawString("Clock", windowSize.hor / 2, offset*3, 2);
            // Clock BG
            window.fillRect(offset, 25, smallWinSize.hor - 10, smallWinSize.vert - 30, WHITE);
            // 
            window.setTextColor(RED);
            window.setTextDatum(MC_DATUM);
            window.drawString(String(delayInSeconds), windowSize.hor / 2, 75, 2);
            window.drawString("<", windowSize.hor / 2 - 20, 75, 2);
            window.drawString(">", windowSize.hor / 2 + 20, 75, 2);
            window.fillRect(windowSize.hor / 2 - 30, 115, 60, 25, RED);
            window.setTextColor(WHITE);
            window.setTextDatum(MC_DATUM);
            window.drawString("Enter", windowSize.hor / 2, windowSize.vert - 35, 2);
            break;
        case Wifi:
            // Top Bar
            window.fillRect(offset, offset, windowSize.hor - 10, 20, LIGHTGREY);
            window.drawRect(offset, offset, windowSize.hor - 10, 20, DARKGREY);

            // // Window Name
            // window.setTextColor(WHITE);
            // window.setTextDatum(MC_DATUM);
            // window.drawString("Wifi", windowSize.hor / 2, offset*3, 2);
            // window.setTextColor(BLACK);
            // window.drawString("WiFi scanner", windowSize.hor / 2, 35, 2);
            // DrawMenu();

            // Initialize WiFi in reciever mode
            WiFi.mode(WIFI_STA);
            WiFi.disconnect();

            // Loading Screen
            // LCD_Clear();
            window.fillRect(offset, 25, smallWinSize.hor - 10, smallWinSize.vert - 30, WHITE);
            window.drawString("Please wait.", smallWinSize.hor / 2, 50, 2);
            window.drawString("Searching...", smallWinSize.hor / 2, 70, 2);
            Serial.println("Searching networks.....");
            
            // Get info from scanned networks
            n = WiFi.scanNetworks();
            LCD_Clear();
            Show(0);

            // Print total networks found at top of screen
            // window.setCursor(smallWinSize.hor / 2 - 20, 10);
            // window.printf("TOTAL: %d", n);
            // window.setCursor(10, 35);

            // // Print list of networks
            // for (int i = (thisPage * pageSize); i < ((thisPage * pageSize) + pageSize); i++)
            // {
            //     if (i >= n) break;

            //     if(i == selectedNetwork) {
            //         window.setTextColor(BLUE);
            //         wifiName = WiFi.SSID(i);
            //     } else {
            //         window.setTextColor(BLACK);
            //     }
            //     networkList.push_back(WiFi.SSID(i));

            //     window.setCursor(10, 40 + (i * 15));
            //     window.print(i + 1);
            //     String ssid = (WiFi.SSID(i).length() > ssidLength)
            //     ? (WiFi.SSID(i).substring(0, ssidLength) + "...") 
            //     : WiFi.SSID(i);
                
            //     String strength;
            //     if(WiFi.RSSI(i) < -90) {
            //         strength = "*";
            //     } else if(WiFi.RSSI(i) < -75) {
            //         strength = "**";
            //     } else {
            //         strength = "***";
            //     }
                
            //     window.print(") " + ssid + " " + strength + "\n");
            // }
            // // Draw arrows and select buttons
            // wifiPage = true;
            // window.setTextColor(BLUE);
            // window.fillRect(60, 110, 35, 35, BLACK); 
            // window.drawString("<", 70, 120);
            // window.fillRect(150, 110, 35, 35, BLACK); 
            // window.drawString(">", 160, 120);
            // // window.fillRect(205, 110, 35, 35, BLACK); 
            // // window.drawString("O", 220, 120);
            // DrawMenu();
            break;
    }

    // Delete Square
    window.fillRect(windowSize.hor - (offset*2 + 15), offset + 2, 15, 15, RED);
    window.drawRect(windowSize.hor - (offset*2 + 15), offset + 2, 15, 15, DARKGREY);
    window.setTextColor(BLACK);
    window.setTextDatum(MC_DATUM);
    window.drawString("X", windowSize.hor - (offset*2 + 7), offset + 9, 2);

    

    window.pushSprite(pos.x, pos.y);
}

void drawInternet() {

    window.setTextColor(BLACK);
    window.setTextDatum(MC_DATUM);
    window.setTextWrap(true, true);
    window.setTextPadding(sWidth);
    // window.drawString(kanyeQuote, smallWinSize.hor / 2, smallWinSize.vert / 2, 1);
    window.setCursor(5, smallWinSize.vert / 2 - 20);
    window.print(kanyeQuote);
}

void buttonAction(Event &e)
{

    // Button Variable
    Button &b = *e.button;
    // Start Button
    if (b.instanceIndex() == 4)
    {
        drawStartMenu();
    }
    // Internet Button
    else if (b.instanceIndex() == 5)
    {
        isWindowOpen = true;
        currentWindow = Wifi;
        drawScreen();
    }
    // Paint Button
    else if (b.instanceIndex() == 6)
    {
        fetchQuote();
        isWindowOpen = true;
        currentWindow = Internet;
        drawScreen();
    }
    // Wifi Button
    else if (b.instanceIndex() == 7)
    {
        isWindowOpen = true;
        currentWindow = Clock;
        drawScreen();
    }
    // Close
    else if (b.instanceIndex() == 8 && isWindowOpen) {
        isWindowOpen = false;
        drawScreen();
    }
    else if (b.instanceIndex() == 9 && isWindowOpen && currentWindow == Clock) {
        delayInSeconds += 5;
        Serial.println(delayInSeconds);
        delay(100);
    }
    else if (b.instanceIndex() == 10 && isWindowOpen && currentWindow == Clock) {
        if ((delayInSeconds - 5) < 0)
        {
            delayInSeconds = 0;
        }
        else
        {
            delayInSeconds -= 5;
        }
        Serial.println(delayInSeconds);
        delay(100);
    }
    else if (b.instanceIndex() == 11 && isWindowOpen && currentWindow == Clock) {
        setAlarmTime(delayInSeconds);
        delay(100);
        isWindowOpen = false;
        drawScreen();
    }
    else if (b.instanceIndex() == 12 && isWindowOpen && currentWindow == Wifi) {
        Search();
    }
}

void drawButtons() {

    // Start Button
    M5.Lcd.fillRect(0, sHeight - tBarHeight, startSize.hor + 10, startSize.vert + 10, YELLOW);

    // Internet Icon
    M5.Lcd.fillRect(20, 60, 28, 28, YELLOW);

    // Close Button
    M5.Lcd.fillRect(275, 25, 25, 25, YELLOW);

    // Button u(165, 80, 40, 40, "up");
    // Button d(145, 80, 40, 40, "down");

    M5.Lcd.fillRect(125, 80, 40, 40, YELLOW);
    M5.Lcd.fillRect(200, 80, 40, 40, YELLOW);
    M5.Lcd.fillRect(155, 125, 60, 40, YELLOW);
}

void setInitialTime()
{
    TimeStruct.Hours = timeClient.getHours();
    TimeStruct.Minutes = timeClient.getMinutes();
    TimeStruct.Seconds = timeClient.getSeconds();
    M5.Rtc.SetTime(&TimeStruct);
}

void setAlarmTime(int delay)
{
    unsigned long currentTime = millis();
    alarmTime = currentTime + (delay * 1000);
    alarmEnabled = true;
    Serial.println(alarmEnabled);
}

void DrawMenu(){
    // Always draw the scan button
    window.fillRect(90, 110, 65, 35, BLACK); 
    window.setTextColor(BLUE);
    window.setTextDatum(MC_DATUM);
    window.drawString("SCAN", smallWinSize.hor / 2, 125, 2);
}

void LCD_Clear() {
    // Reset the screen between changes
    // window.fillSprite(CYAN);
    window.fillRect(offset, 25, smallWinSize.hor - 10, smallWinSize.vert - 30, WHITE);
    window.pushSprite(smallWinPos.x, smallWinPos.y); 
}

void Show(int nav = 0){
    LCD_Clear();

    // Print total networks found at top of screen
    window.setCursor(smallWinSize.hor / 2 - 20, 10);
    window.printf("TOTAL: %d", n);
    window.setCursor(10, 35);

    // Print list of networks
    for (int i = (thisPage * pageSize); i < ((thisPage * pageSize) + pageSize); i++)
    {
        if (i >= n) break;

        if(i == selectedNetwork) {
            window.setTextColor(BLUE);
            wifiName = WiFi.SSID(i);
        } else {
            window.setTextColor(BLACK);
        }
        networkList.push_back(WiFi.SSID(i));

        window.setCursor(10, 40 + (i * 15));
        window.print(i + 1);
        String ssid = (WiFi.SSID(i).length() > ssidLength)
        ? (WiFi.SSID(i).substring(0, ssidLength) + "...") 
        : WiFi.SSID(i);
        
        String strength;
        if(WiFi.RSSI(i) < -90) {
            strength = "*";
        } else if(WiFi.RSSI(i) < -75) {
            strength = "**";
        } else {
            strength = "***";
        }
        
        window.print(") " + ssid + " " + strength + "\n");
    }
    // Draw arrows and select buttons
    wifiPage = true;
    window.setTextColor(BLUE);
    window.fillRect(60, 110, 35, 35, BLACK); 
    window.drawString("<", 70, 120);
    window.fillRect(150, 110, 35, 35, BLACK); 
    window.drawString(">", 160, 120);
    // window.fillRect(205, 110, 35, 35, BLACK); 
    // window.drawString("O", 220, 120);
    DrawMenu();
    window.pushSprite(smallWinPos.x, smallWinPos.y);
}

void Search() {
    // Initialize WiFi in reciever mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Loading Screen
    LCD_Clear();
    window.fillRect(offset, 25, smallWinSize.hor - 10, smallWinSize.vert - 30, WHITE);
    window.drawString("Please wait.", smallWinSize.hor / 2, 50, 2);
    window.drawString("Searching...", smallWinSize.hor / 2, 70, 2);
    
    Serial.println("Searching networks.....");
    
    // Get info from scanned networks
    n = WiFi.scanNetworks();
    Show();
}

void Select() {
    LCD_Clear();
    int waitCount = 0;
    Serial.println(wifiName);

    // Connect to WiFi
    WiFi.begin(wifiName.c_str(), wifiPassword.c_str());
    Serial.printf("Connecting");
    while (WiFi.status() != WL_CONNECTED && waitCount < 10) {
        delay(500);
        Serial.print(".");
        waitCount++;
    }

    // Draw results
    if(WiFi.status() == WL_CONNECTED) {
        Serial.print("\n\nConnected to WiFi network with IP address: ");
        Serial.println(WiFi.localIP());
        window.setTextColor(BLUE);
        window.setCursor(50, 30);
        window.printf("CONNECTION");
        window.setCursor(50, 60);
        window.printf("SUCCESSFUL");
        window.setCursor(50, 90);
        window.printf(wifiName.c_str());
        M5.Spk.DingDong();
        M5.Axp.SetLDOEnable(3, true);
        delay(1000);
        M5.Axp.SetLDOEnable(3, false);
        timeClient.begin();
        timeClient.setTimeOffset(3600 * -7);
        timeClient.update();

        setInitialTime();
    } else {
        window.setTextColor(RED);
        window.setCursor(50, 60);
        window.printf("CONNECTION");
        window.setCursor(50, 90);
        window.printf("FAILED");
    }
    window.pushSprite(smallWinPos.x, smallWinPos.y);

    delay(2000);
    isWindowOpen = false;
    drawScreen();
}

// Moves up and down the list of
void IncrementNum()
{
    selectedNetwork = selectedNetwork + 1;
    if (selectedNetwork >= n)
    {
        selectedNetwork = selectedNetwork % n;
    }
    Show();
}

void DecrementNum()
{
    selectedNetwork = selectedNetwork - 1;
    if (selectedNetwork < 0)
    {
        selectedNetwork = n - 1;
    }
    Show();
}