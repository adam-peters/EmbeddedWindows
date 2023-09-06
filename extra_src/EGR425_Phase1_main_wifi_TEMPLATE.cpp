#include <M5Core2.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include "EGR425_Phase1_weather_bitmap_images.h"
#include "WiFi.h"
#include<string>

using namespace std;

////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////
// API variables
String urlOpenWeather = "https://api.openweathermap.org/data/2.5/weather?";
String apiKey = "695fe12f6f6a7b74f2d6bc53a52b9a36";

// WiFi variables
String wifiNetworkName = "CBU";
String wifiPassword = "";

// Time variables
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;  // 5000; 5 minutes (300,000ms) or 5 seconds (5,000ms)
int hours;
int minutes;
String period;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// LCD and Navigation variables
int sWidth;
int sHeight;
bool homePage = true;

// Weather Page variables
String cityName;
String strWeatherIcon;
String strWeatherDesc;
double tempNow;
double tempMin;
double tempMax;
char unitType = 'F';
bool Farenheit = true;

// Zip Code variable
int zipCode[5] = {9, 0, 4, 0, 3};

////////////////////////////////////////////////////////////////////
// Button zone declerations
////////////////////////////////////////////////////////////////////
Button t0(10, 50, 50, 70, "top-zero");
Button t1(70, 50, 50, 70, "top-one");
Button t2(130, 50, 50, 70, "top-two");
Button t3(190, 50, 50, 70, "top-three");
Button t4(250, 50, 50, 70, "top-four");
Button b0(10, 150, 50, 70, "bot-zero");
Button b1(70, 150, 50, 70, "bot-one");
Button b2(130, 150, 50, 70, "bot-two");
Button b3(190, 150, 50, 70, "bot-three");
Button b4(250, 150, 50, 70, "bot-four");

////////////////////////////////////////////////////////////////////
// Method header declarations
////////////////////////////////////////////////////////////////////
String httpGETRequest(const char* serverName);
void drawWeatherImage(String iconId, int resizeMult);
void fetchWeatherDetails();
void drawWeatherDisplay();
void drawZipDisplay();
void increment(int i);
void buttonAction(Event&);
int convertHours();


///////////////////////////////////////////////////////////////
// Put your setup code here, to run on startup
///////////////////////////////////////////////////////////////
void setup() {
    // Initialize the device
    M5.begin();

    // Button handler checks for button presses in specified zones
    M5.Buttons.addHandler(buttonAction,  E_TOUCH);
    
    // Set screen orientation and get height/width 
    sWidth = M5.Lcd.width();
    sHeight = M5.Lcd.height();

    // TODO 2: Connect to WiFi
    WiFi.begin(wifiNetworkName.c_str(), wifiPassword.c_str());
    Serial.printf("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.print("\n\nConnected to WiFi network with IP address: ");
    Serial.println(WiFi.localIP());

    // Set the time client to proper time zone
    timeClient.begin();
    timeClient.setTimeOffset(-28800);
    lastTime = millis();
}

///////////////////////////////////////////////////////////////
// Repeatedly run code
// Checks for built in button presses, and controls auto
// refresh after a specified time interval
///////////////////////////////////////////////////////////////
void loop() {
    M5.update();

    // If button A is pressed the Farenheit/Celsius units are toggled back and forth
    if(homePage) {
        if(M5.BtnA.wasPressed()) {
            if(Farenheit) {
                tempMin = (tempMin - 32.0) * (5.0/9.0);
                tempMax = (tempMax - 32.0) * (5.0/9.0);
                tempNow = (tempNow - 32.0) * (5.0/9.0);
                unitType = 'C';
            } else {
                tempMin = tempMin * (9.0/5.0) + 32;
                tempMax = tempMax * (9.0/5.0) + 32;
                tempNow = tempNow * (9.0/5.0) + 32;
                unitType = 'F';
            }
            Farenheit = !Farenheit;
            drawWeatherDisplay();
            lastTime = millis();
        }
    }
    
    // If button B is pressed the weather page and zip code page are toggled back and forth
    if(M5.BtnB.wasPressed()) {
        if(homePage) {
            drawZipDisplay();
        } else {
            fetchWeatherDetails();
            drawWeatherDisplay();
        }
        homePage = !homePage;
        lastTime = millis();
    }

    // Automatic page refresh after specified timer delay
    if ((millis() - lastTime) > timerDelay) {
        if (WiFi.status() == WL_CONNECTED) {

            if(homePage) {
                fetchWeatherDetails();
                drawWeatherDisplay();
            } else {
                drawZipDisplay();
            }
            
        } else {
            Serial.println("WiFi Disconnected");
        }

        // Update the last time to NOW
        lastTime = millis();
    }
    // Update time that is printed to the screen
    timeClient.update();
}


/////////////////////////////////////////////////////////////////
// This method fetches the weather details from the OpenWeather
// API and saves them into the fields defined above
/////////////////////////////////////////////////////////////////
void fetchWeatherDetails() {
    // Examples: https://api.openweathermap.org/data/2.5/weather?q=riverside,ca,usa&units=imperial&appid=YOUR_API_KEY
    String zipString = "";
    for(int i = 0; i < 5; i++) {
        zipString += String(zipCode[i]);
    }

    // Call the API using the input zip code, currently selected unit type, and API key
    String unitTypeName;
    if(Farenheit) { 
        unitTypeName = "imperial";
    } else {
        unitTypeName = "metric";
    }
    String serverURL = urlOpenWeather + "zip=" + zipString + ",us&units=" + unitTypeName + "&appid=" + apiKey;
    //Serial.println(serverURL); // Debug print

    // Make GET request and store reponse
    String response = httpGETRequest(serverURL.c_str());
    //Serial.print(response); // Debug print
    
    // Import ArduinoJSON Library and then use arduinojson.org/v6/assistant to
    // compute the proper capacity and initialize json object
    const size_t jsonCapacity = 768+250;
    DynamicJsonDocument objResponse(jsonCapacity);

    // Deserialize the JSON document and test if parsing succeeded
    DeserializationError error = deserializeJson(objResponse, response);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }
    //serializeJsonPretty(objResponse, Serial); // Debug print

    // Parse response to get the weather description and icon
    JsonArray arrWeather = objResponse["weather"];
    JsonObject objWeather0 = arrWeather[0];
    String desc = objWeather0["main"];
    String icon = objWeather0["icon"];
    String city = objResponse["name"];

    // ArduionJson library will not let us save directly to these
    // variables in the 3 lines above for unknown reason
    strWeatherDesc = desc;
    strWeatherIcon = icon;
    cityName = city;

    // Parse response to get the temperatures
    JsonObject objMain = objResponse["main"];
    tempNow = objMain["temp"];
    tempMin = objMain["temp_min"];
    tempMax = objMain["temp_max"];
    Serial.printf("NOW: %.1f F and %s\tMIN: %.1f F\tMax: %.1f F\n", tempNow, strWeatherDesc, tempMin, tempMax);
}


/////////////////////////////////////////////////////////////////
// Update the display based on the weather variables defined
// at the top of the screen.
/////////////////////////////////////////////////////////////////
void drawWeatherDisplay() {
    // Draw background - light blue if day time and navy blue of night
    uint16_t primaryTextColor;
    if (strWeatherIcon.indexOf("d") >= 0) {
        M5.Lcd.fillScreen(TFT_CYAN);
        primaryTextColor = TFT_DARKGREY;
    } else {
        M5.Lcd.fillScreen(TFT_NAVY);
        primaryTextColor = TFT_WHITE;
    }
    
    // Draw the icon on the right side of the screen
    drawWeatherImage(strWeatherIcon, 3);
    
    // Draw the 3 temperature values
    int pad = 10;
    M5.Lcd.setCursor(pad, pad);
    M5.Lcd.setTextColor(TFT_BLUE);
    M5.Lcd.setTextSize(3);
    M5.Lcd.printf("LO:%0.f%c\n", tempMin, unitType);
    
    M5.Lcd.setCursor(pad, M5.Lcd.getCursorY());
    M5.Lcd.setTextColor(primaryTextColor);
    M5.Lcd.setTextSize(10);
    M5.Lcd.printf("%0.f%c\n", tempNow, unitType);

    M5.Lcd.setCursor(pad, M5.Lcd.getCursorY());
    M5.Lcd.setTextColor(TFT_RED);
    M5.Lcd.setTextSize(3);
    M5.Lcd.printf("HI:%0.f%c\n", tempMax, unitType);

    // Draw the last updates time
    M5.Lcd.setCursor(pad, M5.Lcd.getCursorY() + 70);
    M5.Lcd.setTextColor(TFT_BLACK);
    if(timeClient.getMinutes() < 10) {
        // adds a zero for any single digit minute values
        M5.Lcd.printf("%d:0%d%s", convertHours(), timeClient.getMinutes(), period);
    } else {
        M5.Lcd.printf("%d:%d%s", convertHours(), timeClient.getMinutes(), period);
    }

    // Draw the city name
    M5.Lcd.setCursor(pad, M5.Lcd.getCursorY() + 25);
    M5.Lcd.setTextColor(primaryTextColor);
    if(cityName.length() > 13) {
        M5.Lcd.setTextSize(2); // Reduce text size if name is > 13 chars
    }
    M5.Lcd.printf("%s\n", cityName.c_str());
}


/////////////////////////////////////////////////////////////////
// Update the zip code based on the user presses of the
// 10 buttons on the screen.
/////////////////////////////////////////////////////////////////
void drawZipDisplay() {
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);

    M5.Lcd.setCursor(10, 10);
    M5.Lcd.print("Enter your Zipcode...");

    int pad = 60;
    int hor = 30;
    int vert1 = 60;
    int vert2 = 200;

    for(int i = 0; i < 5; i++){
        M5.Lcd.fillTriangle(hor + (i * pad), vert1, hor + 10 + (i * pad), vert1 - 10, hor + 20 + (i * pad), vert1, WHITE);
        M5.Lcd.setCursor(hor + (i * pad), 120);
        M5.Lcd.setTextSize(4);
        M5.Lcd.print(zipCode[i]);
        M5.Lcd.fillTriangle(hor + (i * pad), vert2, hor + 10 + (i * pad), vert2 + 10, hor + 20 + (i * pad), vert2, WHITE);
    }   
}


/////////////////////////////////////////////////////////////////
// This method updates the zip code digits based on button presses
// 0 1 2 3 4
// # # # # #
// 0 1 2 3 4 (value after % 5)
/////////////////////////////////////////////////////////////////
void increment(int i) {
    if(i > 4) {
        i = i % 5; // match bottom buttons for each column
        if(zipCode[i] == 0) {
            zipCode[i] = 9; // loops back to the highest digit
        } else {
            zipCode[i]--; // decrement (bottom buttons)
        }
    } else {
        zipCode[i]++; // increment (top buttons)
        zipCode[i] = zipCode[i] % 10; // loops back to the lowest digit
    }
    drawZipDisplay(); // redraws updated number
}


/////////////////////////////////////////////////////////////////
// This method takes in button presses from the button handler.
// It avoids reading button presses of the 3 built in buttons
// and the background. Calls increment method to change the
// zip code digits.
//
// Example:
// 0 1 2 3 4
// # # # # #
// 0 1 2 3 4 (value after % 5)
/////////////////////////////////////////////////////////////////
void buttonAction(Event& e) {
    Button& b = *e.button;
    if(b.instanceIndex() > 3) {
        increment(b.instanceIndex() - 4); // index range of 0-9
    }
}


/////////////////////////////////////////////////////////////////
// This method converts from 24 hour to 12 hour format 
// and the am/pm marker is updated based on the conversion.
//
// Format - HH:MMam or HH:MMpm
/////////////////////////////////////////////////////////////////
int convertHours() {
    int hours = timeClient.getHours();
    if(hours > 12) {
        hours = hours - 12;
        period = "pm";
    } else if(hours == 0) {
        hours = 12;
    } else if(hours == 12) {
        period = "pm";
    } else {
        period = "am";
    }
    return hours;
}


/////////////////////////////////////////////////////////////////
// This method takes in a URL and makes a GET request to the
// URL, returning the response.
/////////////////////////////////////////////////////////////////
String httpGETRequest(const char* serverURL) {
    
    // Initialize client
    HTTPClient http;
    http.begin(serverURL);

    // Send HTTP GET request and obtain response
    int httpResponseCode = http.GET();
    String response = http.getString();

    // Check if got an error
    if (httpResponseCode > 0)
        Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    else {
        Serial.printf("HTTP Response ERROR code: %d\n", httpResponseCode);
        Serial.printf("Server Response: %s\n", response);
    }

    // Free resources and return response
    http.end();
    return response;
}


/////////////////////////////////////////////////////////////////
// This method takes in an image icon string (from API) and a 
// resize multiple and draws the corresponding image (bitmap byte
// arrays found in EGR425_Phase1_weather_bitmap_images.h) to scale (for 
// example, if resizeMult==2, will draw the image as 200x200 instead
// of the native 100x100 pixels) on the right-hand side of the
// screen (centered vertically). 
/////////////////////////////////////////////////////////////////
void drawWeatherImage(String iconId, int resizeMult) {

    // Get the corresponding byte array
    const uint16_t * weatherBitmap = getWeatherBitmap(iconId);

    // Compute offsets so that the image is centered vertically and is
    // right-aligned
    int yOffset = -(resizeMult * imgSqDim - M5.Lcd.height()) / 2;
    int xOffset = sWidth - (imgSqDim*resizeMult*.8); // Right align (image doesn't take up entire array)
    //int xOffset = (M5.Lcd.width() / 2) - (imgSqDim * resizeMult / 2); // center horizontally
    
    // Iterate through each pixel of the imgSqDim x imgSqDim (100 x 100) array
    for (int y = 0; y < imgSqDim; y++) {
        for (int x = 0; x < imgSqDim; x++) {
            // Compute the linear index in the array and get pixel value
            int pixNum = (y * imgSqDim) + x;
            uint16_t pixel = weatherBitmap[pixNum];

            // If the pixel is black, do NOT draw (treat it as transparent);
            // otherwise, draw the value
            if (pixel != 0) {
                // 16-bit RBG565 values give the high 5 pixels to red, the middle
                // 6 pixels to green and the low 5 pixels to blue as described
                // here: http://www.barth-dev.de/online/rgb565-color-picker/
                byte red = (pixel >> 11) & 0b0000000000011111;
                red = red << 3;
                byte green = (pixel >> 5) & 0b0000000000111111;
                green = green << 2;
                byte blue = pixel & 0b0000000000011111;
                blue = blue << 3;

                // Scale image; for example, if resizeMult == 2, draw a 2x2
                // filled square for each original pixel
                for (int i = 0; i < resizeMult; i++) {
                    for (int j = 0; j < resizeMult; j++) {
                        int xDraw = x * resizeMult + i + xOffset;
                        int yDraw = y * resizeMult + j + yOffset;
                        M5.Lcd.drawPixel(xDraw, yDraw, M5.Lcd.color565(red, green, blue));
                    }
                }
            }
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////
// For more documentation see the following links:
// https://github.com/m5stack/m5-docs/blob/master/docs/en/api/
// https://docs.m5stack.com/en/api/core2/lcd_api
//////////////////////////////////////////////////////////////////////////////////