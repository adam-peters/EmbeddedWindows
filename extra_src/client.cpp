#include <M5Core2.h>
#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLE2902.h>
#include <M5Core2.h>
#include <cstdlib>
#include <ctime>
#include <map>
#include <list>
#include "utility/MPU6886.h"

////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////

// Bluetooth Variables
static BLERemoteCharacteristic *attackP1;
static BLERemoteCharacteristic *attackP2;
static BLEAdvertisedDevice *bleRemoteServer;
static boolean doConnect = false;
static boolean doScan = false;
bool deviceConnected = false;
static BLEUUID SERVICE_UUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID CHARACTERISTIC_UUID_P1("beb5483e-36e1-4688-b7f5-ea07361b26a8");
static BLEUUID CHARACTERISTIC_UUID_P2("6e649b86-3da2-434a-814f-f6854794d3fd");

// LCD variables
int sWidth;
int sHeight;
const int SCREEN_X_BUFFER = 25;
const int SCREEN_Y_BUFFER = 35;

// Accelerometer Variables
float accX;
float accY;
float accZ;

// Text Variables
const int TEXT_SIZE = 2;

// Color Variables
const int CARD_COLOR = BLUE;
const int CARD_BG_COLOR = CYAN;
const int DICE_OUTLINE_COLOR = RED;
const int DICE_BG_COLOR = LIGHTGREY;

// Time (Delay) Variables
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;

// Dice Variables
const int DICE_SIZE = 50;
int selectedDice = 0;
int dice[3];

// Dice Positions
int DICE_X_POS = 250;
int DICE_Y_POS_ONE = 40;
int DICE_Y_POS_TWO = DICE_Y_POS_ONE + 70;
int DICE_Y_POS_THREE = DICE_Y_POS_TWO + 70;

// Player Variables
int PLAYER_ONE_HEALTH = 20;
int PLAYER_TWO_HEALTH = 20;

int PLAYER_ONE_ATTACK = 0;
int PLAYER_TWO_ATTACK = 0;

int PLAYER_ONE_DODGES = 0;
int PLAYER_TWO_DODGES = 0;

int PLAYER_ONE_SHIELD = 0;
int PLAYER_TWO_SHIELD = 0;

bool isBlinded = false;

int NUM_OF_REROLLS = 2;

// Turn Variables
bool awaitResponse = false;
int num_of_turns = 0;

// List of Different Moves
enum Move
{
    Attack = 0,
    Dodge,
    Shield,
    Burn,
    Blind,
};

int NUM_OF_MOVES = 5;
int currentMove = 0;

// List of States
enum GameState
{
    AwaitingConnection = 0,
    StartScreen,
    StartGame,
    Turn,
    TurnOver,
    EndGame,
};

int currentState = 0;

// Cory's farts
const int PIXEL_SIZE = 10;

void drawPixel(int color, int x, int y)
{
    M5.Lcd.setCursor(x, y);
    M5.Lcd.fillRect(x, y, PIXEL_SIZE, PIXEL_SIZE, color);
}

void drawPixelLineX(int color, int x, int y, int length)
{
    for (int i = 0; i < length; i++)
    {
        drawPixel(color, x + (PIXEL_SIZE * i), y);
    }
}

void drawPixelLineY(int color, int x, int y, int length)
{
    for (int i = 0; i < length; i++)
    {
        drawPixel(color, x, y + (PIXEL_SIZE * i));
    }
}

void printWinner(int x, int y)
{
    M5.Lcd.setCursor(x, y);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.print("W");
    delay(100);
    M5.Lcd.setTextColor(ORANGE);
    M5.Lcd.print("i");
    delay(100);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.print("n");
    delay(100);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.print("n");
    delay(100);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.print("e");
    delay(100);
    M5.Lcd.setTextColor(ORANGE);
    M5.Lcd.print("r");
    delay(100);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.print(".");
}

void drawFire1()
{
    drawPixelLineX(RED, 110, 230, 2);
    drawPixelLineX(BLACK, 130, 230, 1);
    drawPixelLineX(RED, 140, 230, 4);

    drawPixelLineX(RED, 90, 220, 3);
    drawPixelLineX(ORANGE, 120, 220, 5);
    drawPixelLineX(RED, 170, 220, 2);

    drawPixelLineX(RED, 80, 210, 1);
    drawPixelLineX(BLACK, 90, 210, 1);
    drawPixelLineX(ORANGE, 100, 210, 2);
    drawPixelLineX(YELLOW, 120, 210, 1);
    drawPixelLineX(ORANGE, 130, 210, 1);
    drawPixelLineX(YELLOW, 140, 210, 1);
    drawPixelLineX(ORANGE, 150, 210, 2);
    drawPixelLineX(RED, 170, 210, 3);

    drawPixelLineX(RED, 80, 200, 2);
    drawPixelLineX(ORANGE, 100, 200, 2);
    drawPixelLineX(YELLOW, 120, 200, 5);
    drawPixelLineX(ORANGE, 170, 200, 2);
    drawPixelLineX(RED, 190, 200, 2);

    drawPixelLineX(RED, 80, 190, 1);
    drawPixelLineX(ORANGE, 90, 190, 2);
    drawPixelLineX(YELLOW, 110, 190, 1);
    drawPixelLineX(ORANGE, 120, 190, 1);
    drawPixelLineX(YELLOW, 130, 190, 1);
    drawPixelLineX(ORANGE, 140, 190, 1);
    drawPixelLineX(YELLOW, 150, 190, 1);
    drawPixelLineX(ORANGE, 160, 190, 3);
    drawPixelLineX(RED, 190, 190, 2);

    drawPixelLineX(RED, 70, 180, 1);
    drawPixelLineX(BLACK, 80, 180, 1);
    drawPixelLineX(RED, 90, 180, 2);
    drawPixelLineX(ORANGE, 110, 180, 2);
    drawPixelLineX(YELLOW, 130, 180, 1);
    drawPixelLineX(ORANGE, 140, 180, 4);
    drawPixelLineX(RED, 180, 180, 4);

    drawPixelLineX(RED, 80, 170, 3);
    drawPixelLineX(ORANGE, 110, 170, 4);
    drawPixelLineX(RED, 150, 170, 1);
    drawPixelLineX(YELLOW, 160, 170, 1);
    drawPixelLineX(RED, 170, 170, 4);

    drawPixelLineX(RED, 80, 160, 2);
    drawPixelLineX(BLACK, 100, 160, 1);
    drawPixelLineX(ORANGE, 110, 160, 1);
    drawPixelLineX(RED, 120, 160, 3);
    drawPixelLineX(ORANGE, 150, 160, 1);
    drawPixelLineX(RED, 160, 160, 2);
    drawPixelLineX(BLACK, 180, 160, 1);
    drawPixelLineX(RED, 190, 160, 1);

    drawPixelLineX(RED, 90, 150, 4);
    drawPixelLineX(ORANGE, 130, 150, 1);
    drawPixelLineX(RED, 140, 150, 6);
    drawPixelLineX(BLACK, 200, 150, 1);
    drawPixelLineX(ORANGE, 210, 150, 1);

    drawPixelLineX(RED, 90, 140, 1);
    drawPixelLineX(BLACK, 100, 140, 1);
    drawPixelLineX(RED, 110, 140, 5);
    drawPixelLineX(ORANGE, 160, 140, 1);
    drawPixelLineX(RED, 170, 140, 2);

    drawPixelLineX(RED, 110, 130, 1);
    drawPixelLineX(BLACK, 120, 130, 1);
    drawPixelLineX(RED, 130, 130, 5);

    drawPixelLineX(ORANGE, 100, 120, 1);
    drawPixelLineX(BLACK, 110, 120, 1);
    drawPixelLineX(RED, 120, 120, 4);

    drawPixelLineX(RED, 130, 110, 1);
    drawPixelLineX(BLACK, 140, 110, 2);
    drawPixelLineX(ORANGE, 160, 110, 1);

    drawPixelLineX(RED, 130, 100, 1);
    drawPixelLineX(BLACK, 140, 100, 3);
    drawPixelLineX(RED, 170, 100, 1);

    drawPixelLineX(RED, 140, 90, 1);
}

void drawFire2()
{
    drawPixelLineX(RED, 110, 230, 4);
    drawPixelLineX(BLACK, 150, 230, 2);
    drawPixelLineX(RED, 160, 230, 1);

    drawPixelLineX(RED, 80, 220, 4);
    drawPixelLineX(ORANGE, 120, 220, 5);
    drawPixelLineX(RED, 170, 220, 3);

    drawPixelLineX(RED, 70, 210, 3);
    drawPixelLineX(ORANGE, 100, 210, 2);
    drawPixelLineX(YELLOW, 120, 210, 4);
    drawPixelLineX(ORANGE, 160, 210, 1);
    drawPixelLineX(RED, 170, 210, 3);

    drawPixelLineX(RED, 70, 200, 1);
    drawPixelLineX(BLACK, 80, 200, 1);
    drawPixelLineX(RED, 90, 200, 1);
    drawPixelLineX(ORANGE, 100, 200, 2);
    drawPixelLineX(YELLOW, 120, 200, 5);
    drawPixelLineX(ORANGE, 170, 200, 2);
    drawPixelLineX(RED, 190, 200, 2);

    drawPixelLineX(RED, 60, 190, 3);
    drawPixelLineX(ORANGE, 90, 190, 2);
    drawPixelLineX(YELLOW, 110, 190, 1);
    drawPixelLineX(ORANGE, 120, 190, 1);
    drawPixelLineX(YELLOW, 130, 190, 1);
    drawPixelLineX(ORANGE, 140, 190, 1);
    drawPixelLineX(YELLOW, 150, 190, 1);
    drawPixelLineX(ORANGE, 160, 190, 3);
    drawPixelLineX(RED, 190, 190, 2);

    drawPixelLineX(RED, 60, 180, 1);
    drawPixelLineX(BLACK, 70, 180, 1);
    drawPixelLineX(RED, 80, 180, 1);
    drawPixelLineX(ORANGE, 90, 180, 5);
    drawPixelLineX(YELLOW, 140, 180, 1);
    drawPixelLineX(ORANGE, 150, 180, 3);
    drawPixelLineX(RED, 180, 180, 4);

    drawPixelLineX(RED, 80, 170, 1);
    drawPixelLineX(ORANGE, 90, 170, 1);
    drawPixelLineX(BLACK, 100, 170, 1);
    drawPixelLineX(ORANGE, 110, 170, 4);
    drawPixelLineX(RED, 150, 170, 1);
    drawPixelLineX(ORANGE, 160, 170, 1);
    drawPixelLineX(RED, 170, 170, 4);

    drawPixelLineX(RED, 70, 160, 3);
    drawPixelLineX(BLACK, 100, 160, 1);
    drawPixelLineX(ORANGE, 110, 160, 1);
    drawPixelLineX(RED, 120, 160, 3);
    drawPixelLineX(ORANGE, 150, 160, 1);
    drawPixelLineX(RED, 160, 160, 4);

    drawPixelLineX(RED, 90, 150, 2);
    drawPixelLineX(BLACK, 110, 150, 1);
    drawPixelLineX(RED, 120, 150, 1);
    drawPixelLineX(ORANGE, 130, 150, 1);
    drawPixelLineX(RED, 140, 150, 6);
    drawPixelLineX(BLACK, 200, 150, 1);
    drawPixelLineX(ORANGE, 210, 150, 1);

    drawPixelLineX(RED, 80, 140, 3);
    drawPixelLineX(BLACK, 110, 140, 1);
    drawPixelLineX(RED, 120, 140, 4);
    drawPixelLineX(ORANGE, 160, 140, 1);
    drawPixelLineX(RED, 170, 140, 3);

    drawPixelLineX(RED, 80, 130, 1);
    drawPixelLineX(BLACK, 90, 130, 1);
    drawPixelLineX(RED, 100, 130, 2);
    drawPixelLineX(BLACK, 120, 130, 1);
    drawPixelLineX(RED, 130, 130, 5);
    drawPixelLineX(BLACK, 180, 130, 2);
    drawPixelLineX(RED, 200, 130, 1);

    drawPixelLineX(ORANGE, 100, 120, 1);
    drawPixelLineX(RED, 110, 120, 6);
    drawPixelLineX(BLACK, 170, 120, 2);
    drawPixelLineX(ORANGE, 190, 120, 1);

    drawPixelLineX(RED, 80, 110, 1);
    drawPixelLineX(BLACK, 90, 110, 4);
    drawPixelLineX(RED, 130, 110, 1);
    drawPixelLineX(BLACK, 140, 110, 1);
    drawPixelLineX(ORANGE, 150, 110, 1);
    drawPixelLineX(RED, 160, 110, 1);

    drawPixelLineX(RED, 90, 100, 1);
    drawPixelLineX(BLACK, 100, 100, 3);
    drawPixelLineX(RED, 130, 100, 2);
    drawPixelLineX(BLACK, 150, 100, 1);
    drawPixelLineX(RED, 160, 100, 1);
    
    drawPixelLineX(RED, 140, 90, 1);
    
    drawPixelLineX(RED, 130, 80, 1);
}

void drawFire3(){
    drawPixelLineX(RED, 110, 230, 7);
    
    drawPixelLineX(RED, 80, 220, 4);
    drawPixelLineX(ORANGE, 120, 220, 5);
    drawPixelLineX(RED, 170, 220, 3);

    drawPixelLineX(RED, 70, 210, 3);
    drawPixelLineX(ORANGE, 100, 210, 2);
    drawPixelLineX(YELLOW, 120, 210, 4);
    drawPixelLineX(ORANGE, 160, 210, 2);
    drawPixelLineX(RED, 180, 210, 2);

    drawPixelLineX(RED, 70, 200, 3);
    drawPixelLineX(ORANGE, 100, 200, 2);
    drawPixelLineX(YELLOW, 120, 200, 5);
    drawPixelLineX(ORANGE, 170, 200, 2);
    drawPixelLineX(RED, 190, 200, 2);

    drawPixelLineX(RED, 60, 190, 3);
    drawPixelLineX(ORANGE, 90, 190, 2);
    drawPixelLineX(YELLOW, 110, 190, 1);
    drawPixelLineX(ORANGE, 120, 190, 1);
    drawPixelLineX(YELLOW, 130, 190, 1);
    drawPixelLineX(ORANGE, 140, 190, 1);
    drawPixelLineX(YELLOW, 150, 190, 1);
    drawPixelLineX(ORANGE, 160, 190, 3);
    drawPixelLineX(RED, 190, 190, 2);
    
    drawPixelLineX(RED, 60, 180, 3);
    drawPixelLineX(ORANGE, 90, 180, 1);
    drawPixelLineX(RED, 100, 180, 1);
    drawPixelLineX(ORANGE, 110, 180, 2);
    drawPixelLineX(YELLOW, 130, 180, 1);
    drawPixelLineX(ORANGE, 140, 180, 4);
    drawPixelLineX(RED, 180, 180, 4);

    drawPixelLineX(RED, 60, 170, 1);
    drawPixelLineX(BLACK, 70, 170, 1);
    drawPixelLineX(RED, 80, 170, 1);
    drawPixelLineX(ORANGE, 90, 170, 6);
    drawPixelLineX(RED, 150, 170, 1);
    drawPixelLineX(ORANGE, 160, 170, 1);
    drawPixelLineX(RED, 170, 170, 5);
    
    drawPixelLineX(RED, 70, 160, 3);
    drawPixelLineX(ORANGE, 100, 160, 2);
    drawPixelLineX(RED, 120, 160, 1);
    drawPixelLineX(ORANGE, 130, 160, 1);
    drawPixelLineX(RED, 140, 160, 1);
    drawPixelLineX(ORANGE, 150, 160, 1);
    drawPixelLineX(RED, 160, 160, 4);
    drawPixelLineX(BLACK, 200, 160, 1);
    drawPixelLineX(RED, 210, 160, 1);

    drawPixelLineX(RED, 70, 150, 4);
    drawPixelLineX(ORANGE, 110, 150, 1);
    drawPixelLineX(RED, 120, 150, 1);
    drawPixelLineX(ORANGE, 130, 150, 1);
    drawPixelLineX(RED, 140, 150, 6);
    drawPixelLineX(BLACK, 200, 150, 1);
    drawPixelLineX(RED, 210, 150, 1);

    drawPixelLineX(RED, 70, 140, 4);
    drawPixelLineX(ORANGE, 110, 140, 1);
    drawPixelLineX(RED, 120, 140, 4);
    drawPixelLineX(ORANGE, 160, 140, 1);
    drawPixelLineX(RED, 170, 140, 4);

    drawPixelLineX(ORANGE, 70, 130, 1);
    drawPixelLineX(RED, 80, 130, 1);
    drawPixelLineX(BLACK, 90, 130, 1);
    drawPixelLineX(RED, 100, 130, 2);
    drawPixelLineX(ORANGE, 120, 130, 1);
    drawPixelLineX(RED, 130, 130, 6);

    drawPixelLineX(RED, 100, 120, 7);

    drawPixelLineX(RED, 80, 110, 1);
    drawPixelLineX(BLACK, 90, 110, 1);
    drawPixelLineX(RED, 100, 110, 1);
    drawPixelLineX(BLACK, 110, 110, 1);
    drawPixelLineX(RED, 120, 110, 5);
    drawPixelLineX(BLACK, 170, 110, 2);
    drawPixelLineX(ORANGE, 190, 110, 1);

    drawPixelLineX(RED, 80, 100, 1);
    drawPixelLineX(BLACK, 90, 100, 1);
    drawPixelLineX(RED, 100, 100, 1);
    drawPixelLineX(BLACK, 110, 100, 2);
    drawPixelLineX(RED, 130, 100, 2);
    drawPixelLineX(BLACK, 150, 100, 1);
    drawPixelLineX(RED, 160, 100, 1);

    drawPixelLineX(ORANGE, 90, 90, 1);
    drawPixelLineX(BLACK, 100, 90, 1);
    drawPixelLineX(RED, 110, 90, 1);
    drawPixelLineX(BLACK, 120, 90, 2);
    drawPixelLineX(RED, 140, 90, 1);
    drawPixelLineX(BLACK, 150, 90, 4);
    drawPixelLineX(ORANGE, 190, 90, 1);

    drawPixelLineX(RED, 150, 80, 1);
    drawPixelLineX(BLACK, 160, 80, 3);
    drawPixelLineX(ORANGE, 190, 80, 1);
}

////////////////////////////////////////////////////////////////////
// Button Zone Declarations
////////////////////////////////////////////////////////////////////

Button l(20, 80, 40, 40, "left");
Button r(180, 80, 40, 40, "right");
Button d1(DICE_X_POS, DICE_Y_POS_ONE, DICE_SIZE, DICE_SIZE, "dice-one");
Button d2(DICE_X_POS, DICE_Y_POS_TWO, DICE_SIZE, DICE_SIZE, "dice-two");
Button d3(DICE_X_POS, DICE_Y_POS_THREE, DICE_SIZE, DICE_SIZE, "dice-three");
Button s(80, 180, 160, 40, "start-game");
Button e(175, 185, 50, 50, "enter");

////////////////////////////////////////////////////////////////////
// Method header declarations
////////////////////////////////////////////////////////////////////

// Bluetooth Methods
void broadcastBleServer();

// Turn Methods
void awaitingTurn();

// Methods for rolling dice
int rollDie();
void rollDice();
void reRollDice();

// Methods for using a move
bool isDiceMove();
void doMove();
int firstActive();

// Move Specific Methods
void blind();

// Methods to Cycle Move Cards
void incrementMove();
void decrementMove();

// Rendering Methods
// Start Screen
void renderStartScreen();
// Card
void drawCard();
void printCardText();
void drawEnterArrow(int color);
// Dice
void drawDie(int x, int y, int num);
void drawDice();
void drawDiceBG();
void drawOutline();
// Stats
void drawHealths();
void drawRerolls();

// Initialize Methods
void initializeGame();
void resetRound();

// Button Handler Method
void buttonAction(Event &);

////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////

static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
    Serial.printf("Notify callback for characteristic %s of data length %d\n", pBLERemoteCharacteristic->getUUID().toString().c_str(), length);
    Serial.printf("\tData: %s", (char *)pData);
    std::string value = pBLERemoteCharacteristic->readValue();
    Serial.printf("\tValue was: %s", value.c_str());
}

class MyClientCallback : public BLEClientCallbacks
{
    void onConnect(BLEClient *pclient)
    {
        deviceConnected = true;
        Serial.println("Device connected...");
    }

    void onDisconnect(BLEClient *pclient)
    {
        deviceConnected = false;
        Serial.println("Device disconnected...");
    }
};

bool connectToServer()
{
    // Create the client
    Serial.printf("Forming a connection to %s\n", bleRemoteServer->getName().c_str());
    BLEClient *bleClient = BLEDevice::createClient();
    bleClient->setClientCallbacks(new MyClientCallback());
    Serial.println("\tClient connected");

    // Connect to the remote BLE Server.
    if (!bleClient->connect(bleRemoteServer))
        Serial.printf("FAILED to connect to server (%s)\n", bleRemoteServer->getName().c_str());
    Serial.printf("\tConnected to server (%s)\n", bleRemoteServer->getName().c_str());

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService *bleRemoteService = bleClient->getService(SERVICE_UUID);
    if (bleRemoteService == nullptr)
    {
        Serial.printf("Failed to find our service UUID: %s\n", SERVICE_UUID.toString().c_str());
        bleClient->disconnect();
        return false;
    }
    Serial.printf("Found our service UUID: %s\n", SERVICE_UUID.toString().c_str());

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    attackP2 = bleRemoteService->getCharacteristic(CHARACTERISTIC_UUID_P2);
    if (attackP2 == nullptr)
    {
        Serial.printf("Failed to find our characteristic UUID: %s\n", CHARACTERISTIC_UUID_P2.toString().c_str());
        bleClient->disconnect();
        return false;
    }
    Serial.printf("Found our characteristic UUID: %s\n", CHARACTERISTIC_UUID_P2.toString().c_str());

    // Read the value of the characteristic
    if (attackP2->canRead())
    {
        std::string value = attackP2->readValue();
        Serial.printf("\tThe initial P2 attack value was: %s \n", value.c_str());
        // drawScreenTextWithBackground("Initial characteristic value read from server:\n\n" + String(value.c_str()), TFT_GREEN);
        delay(1000);
    }

    // Check if server's characteristic can notify client of changes and register to listen if so
    // if (attackP2->canNotify())
    //     attackP2->registerForNotify(notifyCallback);

    attackP1 = bleRemoteService->getCharacteristic(CHARACTERISTIC_UUID_P1);
    if (attackP1 == nullptr)
    {
        Serial.printf("Failed to find our characteristic UUID: %s\n", CHARACTERISTIC_UUID_P1.toString().c_str());
        bleClient->disconnect();
        return false;
    }
    Serial.printf("Found our characteristic UUID: %s\n", CHARACTERISTIC_UUID_P1.toString().c_str());

    // Read the value of the characteristic
    if (attackP1->canRead())
    {
        std::string value = attackP1->readValue();
        Serial.printf("\tThe initial P1 attack was: %s \n", value.c_str());
        // drawScreenTextWithBackground("Initial characteristic value read from server:\n\n" + String(value.c_str()), TFT_GREEN);
        delay(1000);
    }

    // Check if server's characteristic can notify client of changes and register to listen if so
    // if (attackP1->canNotify())
    //     attackP1->registerForNotify(notifyCallback);

    // deviceConnected = true;
    return true;
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    /**
     * Called for each advertising BLE server.
     */
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        // Print device found
        Serial.print("BLE Advertised Device found:");
        Serial.printf("\tName: %s\n", advertisedDevice.getName().c_str());

        // More debugging print
        // Serial.printf("\tAddress: %s\n", advertisedDevice.getAddress().toString().c_str());
        // Serial.printf("\tHas a ServiceUUID: %s\n", advertisedDevice.haveServiceUUID() ? "True" : "False");
        // for (int i = 0; i < advertisedDevice.getServiceUUIDCount(); i++) {
        //    Serial.printf("\t\t%s\n", advertisedDevice.getServiceUUID(i).toString().c_str());
        // }
        // Serial.printf("\tHas our service: %s\n\n", advertisedDevice.isAdvertisingService(SERVICE_UUID) ? "True" : "False");

        // We have found a device, let us now see if it contains the service we are looking for.
        if (advertisedDevice.haveServiceUUID() &&
            advertisedDevice.isAdvertisingService(SERVICE_UUID) &&
            advertisedDevice.getName() == "Dice P1")
        {
            BLEDevice::getScan()->stop();
            bleRemoteServer = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
            doScan = true;
        }
    }
};

// Setup Method to Initialize Game
void setup()
{
    // Initialize the device
    M5.begin();
    M5.IMU.Init();

    BLEDevice::init("");

    // Retrieve a Scanner and set the callback we want to use to be informed when we
    // have detected a new device.  Specify that we want active scanning and start the
    // scan to run for 5 seconds.
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);
    // drawScreenTextWithBackground("Scanning for BLE server...", TFT_BLUE);

    // Get Width/Height
    sWidth = M5.Lcd.width();
    sHeight = M5.Lcd.height();

    // Button Handler
    M5.Buttons.addHandler(buttonAction, E_TOUCH);

    currentState = AwaitingConnection;
    renderStartScreen();

    // Button Visualizers for Testing
    // Enter, Dice One, Dice Two, Dice Three
    // int TEST_COLOR = YELLOW;
    // M5.Lcd.fillRect(175, 185, 50, 50, TEST_COLOR);
    // M5.Lcd.fillRect(DICE_X_POS, DICE_Y_POS_ONE, DICE_SIZE, DICE_SIZE, TEST_COLOR);
    // M5.Lcd.fillRect(DICE_X_POS, DICE_Y_POS_TWO, DICE_SIZE, DICE_SIZE, TEST_COLOR);
    // M5.Lcd.fillRect(DICE_X_POS, DICE_Y_POS_THREE, DICE_SIZE, DICE_SIZE, TEST_COLOR);
}

void loop()
{
    // If the flag "doConnect" is true then we have scanned for and found the desired
    // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
    // connected we set the connected flag to be false.
    if (doConnect == true)
    {
        if (connectToServer())
        {
            Serial.println("We are now connected to the BLE Server.");
            currentState = StartScreen;
            renderStartScreen();
            doConnect = false;
            delay(3000);
        }
        else
        {
            Serial.println("We have failed to connect to the server; there is nothin more we will do.");
            delay(3000);
        }
    }

    M5.update();

    // Get accelerator data from sensor
    M5.IMU.getAccelData(&accX, &accY, &accZ);

    if (deviceConnected)
    {
        // Handles Screen Buttons being Pressed
        if (M5.BtnA.wasPressed() && !awaitResponse)
        {
            // Serial.print("Button A Pressed");
            decrementMove();
            drawCard();
        }
        if (M5.BtnC.wasPressed() && !awaitResponse)
        {
            // Serial.print("Button B Pressed");
            incrementMove();
            drawCard();
        }

        if ((millis() - lastTime) > timerDelay)
        {
            // Check to see if the user shakes device
            if ((accX >= 1.5 || accY >= 1.5 || accZ >= 1.5) && currentState == StartGame)
            {
                reRollDice();
                drawRerolls();
            }

            // At the end of Turn, do Something
            if (awaitResponse)
            {
                awaitingTurn();
            }

            // Update Time
            lastTime = millis();
        }
    }
}

void initializeGame()
{
    // Change Game State
    currentState = StartGame;

    if (num_of_turns % 3 == 0)
    {
        // Clear Display
        M5.Lcd.clearDisplay(BLACK);

        // Intial Dice Rolls
        rollDice();
        isBlinded = false;
        NUM_OF_REROLLS = 2;
    }

    num_of_turns++;

    // Render UI
    drawHealths();
    drawRerolls();

    // Render Available Cards
    drawCard();

    // Render Dice
    drawDiceBG();
    drawDice();
}

void resetRound()
{
    // Set shield and dodge values when used
    if (currentMove == Shield) {
        if (PLAYER_ONE_ATTACK > 1) {
            PLAYER_TWO_SHIELD = 2;
        }
        else if (PLAYER_ONE_ATTACK == 1) {
            PLAYER_TWO_SHIELD = 1;
        }
        else {
            PLAYER_TWO_SHIELD = 0;
        }
    } else if (currentMove == Dodge) {
        PLAYER_TWO_DODGES = PLAYER_ONE_ATTACK;
    }
        
    // Update health
    PLAYER_ONE_HEALTH = (PLAYER_ONE_HEALTH - PLAYER_TWO_ATTACK) + PLAYER_ONE_DODGES + PLAYER_ONE_SHIELD;
    PLAYER_TWO_HEALTH = (PLAYER_TWO_HEALTH - PLAYER_ONE_ATTACK) + PLAYER_TWO_DODGES + PLAYER_TWO_SHIELD;

    // Debug
    Serial.printf("My dodge %d\n", PLAYER_TWO_DODGES);
    Serial.printf("Dodge %d\n", PLAYER_ONE_DODGES);
    Serial.printf("My attack %d\n", PLAYER_TWO_ATTACK);
    Serial.printf("Attack %d\n", PLAYER_ONE_ATTACK);
    Serial.printf("My new health %d\n", PLAYER_TWO_HEALTH);
    Serial.printf("New health %d\n", PLAYER_ONE_HEALTH);

    // Reset all values after mini-round
    PLAYER_ONE_ATTACK = 0;
    PLAYER_TWO_ATTACK = 0;
    PLAYER_ONE_DODGES = 0;
    PLAYER_TWO_DODGES = 0;
    PLAYER_ONE_SHIELD = 0;
    PLAYER_TWO_SHIELD = 0;

    attackP1->writeValue("-1");
    awaitResponse = false;

    if (PLAYER_TWO_HEALTH <= 0)
    {
        M5.Lcd.clearDisplay(BLACK);
        M5.Lcd.print("Why are you so bad?");

    }
    else if (PLAYER_ONE_HEALTH <= 0)
    {
        M5.Lcd.clearDisplay(BLACK);
        M5.Lcd.setTextSize(10);
        printWinner(20, 3);

        drawPixelLineX(GREEN, 0, 310, 24);

        while (true)
        {
            drawFire1();
            delay(300);
            M5.Lcd.fillRect(0, 50, 320, 240, BLACK);
            drawFire2();
            delay(300);
            M5.Lcd.fillRect(0, 50, 320, 240, BLACK);
            drawFire3();
            delay(300);
            M5.Lcd.fillRect(0, 50, 320, 240, BLACK);
        }
    }
    else
    {
        initializeGame();
    }
}

////////////////////////////////////////////////////////////////////
// Dice Methods
////////////////////////////////////////////////////////////////////

int rollDie()
{
    return 1 + (esp_random() % 6);
}

void rollDice()
{
    dice[0] = rollDie();
    dice[1] = rollDie();
    dice[2] = rollDie();
}

void reRollDice()
{
    if (NUM_OF_REROLLS > 0)
    {
        NUM_OF_REROLLS--;

        // Only rerolls dice if the dice has not been used
        dice[0] == 0 ? Serial.print("") : dice[0] = rollDie();
        dice[1] == 0 ? Serial.print("") : dice[1] = rollDie();
        dice[2] == 0 ? Serial.print("") : dice[2] = rollDie();

        drawDice();
    }
}

bool isDiceMove()
{
    switch (currentMove)
    {
    case Attack:
        return dice[selectedDice] <= 4;
        break;
    case Dodge:
        return dice[selectedDice] == 5;
        break;
    case Shield:
        return dice[selectedDice] % 2 == 0;
        break;
    case Burn:
        return dice[selectedDice] == 6;
        break;
    case Blind:
        return dice[selectedDice] == 6;
        break;
    }
    return false;
}

////////////////////////////////////////////////////////////////////
// Move Methods
////////////////////////////////////////////////////////////////////

void doMove()
{
    int damage;
    switch (currentMove)
    {
    case Attack:
        PLAYER_TWO_ATTACK = dice[selectedDice];
        attackP2->writeValue(String(PLAYER_TWO_ATTACK).c_str(), String(PLAYER_TWO_ATTACK).length());
        Serial.println("Sending attack: " + String(PLAYER_TWO_ATTACK));
        break;
    case Dodge:
        attackP2->writeValue("dodge");
        break;
    case Shield:
        attackP2->writeValue("shield");
    case Burn:
        PLAYER_TWO_ATTACK = 3;
        attackP2->writeValue(String(PLAYER_TWO_ATTACK).c_str(), String(PLAYER_TWO_ATTACK).length());
        Serial.println("Sending attack: " + String(PLAYER_TWO_ATTACK));
        attackP2->writeValue("burn");
        break;
    case Blind:
        attackP2->writeValue("blind");
        std::string valStr = attackP2->readValue();
        Serial.println("Sending attack: " + String(valStr.c_str()));
        // Blind Die
        break;
    }
    dice[selectedDice] = 0;
    awaitResponse = true;
}

// Write attack damage to BT characteristic and update other player's health value
void doDamage(int damage)
{
    attackP2->writeValue(String(damage).c_str(), String(damage).length());
    Serial.println("Sending attack: " + String(damage));
}

void blind()
{
    // Cover the first available dice position with a gray square
    switch(firstActive()) {
        case 0:
            M5.Lcd.fillRoundRect(DICE_X_POS - 5, DICE_Y_POS_ONE - 5, DICE_SIZE + 10, DICE_SIZE + 10, 10, DARKGREY);
            break;
        case 1:
            M5.Lcd.fillRoundRect(DICE_X_POS - 5, DICE_Y_POS_TWO - 5, DICE_SIZE + 10, DICE_SIZE + 10, 10, DARKGREY);
            break;
        case 2:
            M5.Lcd.fillRoundRect(DICE_X_POS - 5, DICE_Y_POS_THREE - 5, DICE_SIZE + 10, DICE_SIZE + 10, 10, DARKGREY);
            break;
    }
}

// Get attack value from other player
void awaitingTurn()
{
    std::string readValue = attackP1->readValue(); // String from other player
    String valStr = readValue.c_str();

    // Check if the characteristic value has changed to a specific attack
    // If characteristic is still -1, then continue to check every second
    // Reset to -1 after value has been read
    if (valStr == "blind")
    {
        isBlinded = true;
        resetRound();
    }
    else if (valStr == "burn")
    {

        resetRound();
    }
    else if (valStr == "shield")
    {
        if (PLAYER_TWO_ATTACK > 1)
        {
            PLAYER_ONE_SHIELD = 2;
        }
        else if (PLAYER_TWO_ATTACK == 1)
        {
            PLAYER_ONE_SHIELD = 1;
        }
        else
        {
            PLAYER_ONE_SHIELD = 0;
        }
        resetRound();
    }
    else if (valStr == "dodge")
    {
        PLAYER_ONE_DODGES = PLAYER_TWO_ATTACK;
        resetRound();
    }
    else if (valStr != "-1")
    {
        PLAYER_ONE_ATTACK = valStr.toInt();
        Serial.printf("The new attack value from P1: %s\n", valStr); // debug help
        drawHealths();
        resetRound();
    }
    else
    {
        Serial.println("Still waiting...");
    }
}

////////////////////////////////////////////////////////////////////
// Render Methods
////////////////////////////////////////////////////////////////////

void drawDice()
{
    drawDie(DICE_X_POS, DICE_Y_POS_ONE, dice[0]);
    drawDie(DICE_X_POS, DICE_Y_POS_TWO, dice[1]);
    drawDie(DICE_X_POS, DICE_Y_POS_THREE, dice[2]);
    if(isBlinded){blind();}
}

void drawDiceBG()
{
    M5.Lcd.fillRoundRect(DICE_X_POS - 5, DICE_Y_POS_ONE - 5, DICE_SIZE + 10, 200, 10, DICE_BG_COLOR);
}

void drawCard()
{
    // Helpful Size Variables
    int cardSize = 200;
    int margin = 10;

    // Draw Card
    M5.Lcd.fillRoundRect(SCREEN_X_BUFFER - 5, SCREEN_Y_BUFFER - 5, cardSize + 10, cardSize + 10, 10, CARD_BG_COLOR);
    M5.Lcd.fillRoundRect(SCREEN_X_BUFFER, SCREEN_Y_BUFFER, cardSize, cardSize, 10, CARD_COLOR);

    // Draw Empty Dice Shape
    M5.Lcd.fillRoundRect(SCREEN_X_BUFFER + 75, SCREEN_Y_BUFFER + DICE_SIZE, DICE_SIZE, DICE_SIZE, 10, BLACK);

    // Write Card Text
    printCardText();

    // Variable for placing Enter
    int bottomEdge = SCREEN_X_BUFFER + cardSize - 30;
    int middle = (cardSize / 2) - 20;
    int leftEdge = SCREEN_Y_BUFFER + 30; // Might not be Y

    // Draw Arrows for Changing Move
    // M5.Lcd.fillTriangle(bottomEdge, middle, bottomEdge + 15, middle + 10, bottomEdge, middle + 20, WHITE); // Right Triangle
    // M5.Lcd.fillTriangle(leftEdge, middle, leftEdge - 15, middle + 10, leftEdge, middle + 20, WHITE); // Left Triangle
}

void drawEnterArrow(int color)
{

    int bottomEdge = SCREEN_X_BUFFER + 170;

    // Draw Red Enter Triangle
    M5.Lcd.fillTriangle(bottomEdge, bottomEdge, bottomEdge + 15, bottomEdge + 10, bottomEdge, bottomEdge + 20, color);
}

// Draws the text on the card for the currently selected Move
void printCardText()
{

    M5.Lcd.setTextSize(TEXT_SIZE);
    M5.Lcd.setTextColor(WHITE, CARD_COLOR);

    int offset = 5;

    switch (currentMove)
    {
    case Attack:
        M5.Lcd.setCursor(SCREEN_X_BUFFER + offset, SCREEN_Y_BUFFER + 125);
        M5.Lcd.print("Do _ Damage");
        M5.Lcd.setCursor(SCREEN_X_BUFFER + offset, SCREEN_Y_BUFFER + 150);
        M5.Lcd.print("(Max 4)");
        break;
    case Dodge:
        M5.Lcd.setCursor(SCREEN_X_BUFFER + offset, SCREEN_Y_BUFFER + 125);
        M5.Lcd.print("Dodge One Attack");
        M5.Lcd.setCursor(SCREEN_X_BUFFER + offset, SCREEN_Y_BUFFER + 150);
        M5.Lcd.print("(Needs: 5)");
        break;
    case Shield:
        M5.Lcd.setCursor(SCREEN_X_BUFFER + offset, SCREEN_Y_BUFFER + 125);
        M5.Lcd.print("Create 2 Shield");
        M5.Lcd.setCursor(SCREEN_X_BUFFER + offset, SCREEN_Y_BUFFER + 150);
        M5.Lcd.print("(Needs: EVEN)");
        break;
    case Burn:
        M5.Lcd.setCursor(SCREEN_X_BUFFER + offset, SCREEN_Y_BUFFER + 125);
        M5.Lcd.print("Burn One Enemy");
        M5.Lcd.setCursor(SCREEN_X_BUFFER + offset, SCREEN_Y_BUFFER + 150);
        M5.Lcd.print("Die");
        M5.Lcd.setCursor(SCREEN_X_BUFFER + offset, SCREEN_Y_BUFFER + 175);
        M5.Lcd.print("(Needs: 6)");
        break;
    case Blind:
        M5.Lcd.setCursor(SCREEN_X_BUFFER + offset, SCREEN_Y_BUFFER + 125);
        M5.Lcd.print("Blind One Enemy");
        M5.Lcd.setCursor(SCREEN_X_BUFFER + offset, SCREEN_Y_BUFFER + 150);
        M5.Lcd.print("Die");
        M5.Lcd.setCursor(SCREEN_X_BUFFER + offset, SCREEN_Y_BUFFER + 175);
        M5.Lcd.print("(Needs: 6)");
        break;
    }
}

void drawOutline()
{
    // Remove Previous Outlines
    M5.Lcd.fillRoundRect(DICE_X_POS - 5, DICE_Y_POS_ONE - 5, DICE_SIZE + 10, DICE_SIZE + 10, 10, DICE_BG_COLOR);
    M5.Lcd.fillRoundRect(DICE_X_POS - 5, DICE_Y_POS_TWO - 5, DICE_SIZE + 10, DICE_SIZE + 10, 10, DICE_BG_COLOR);
    M5.Lcd.fillRoundRect(DICE_X_POS - 5, DICE_Y_POS_THREE - 5, DICE_SIZE + 10, DICE_SIZE + 10, 10, DICE_BG_COLOR);

    // Draw Outlines around selectedDice
    switch (selectedDice)
    {
    case 0:
        M5.Lcd.fillRoundRect(DICE_X_POS - 5, DICE_Y_POS_ONE - 5, DICE_SIZE + 10, DICE_SIZE + 10, 10, DICE_OUTLINE_COLOR);
        break;
    case 1:
        M5.Lcd.fillRoundRect(DICE_X_POS - 5, DICE_Y_POS_TWO - 5, DICE_SIZE + 10, DICE_SIZE + 10, 10, DICE_OUTLINE_COLOR);
        break;
    case 2:
        M5.Lcd.fillRoundRect(DICE_X_POS - 5, DICE_Y_POS_THREE - 5, DICE_SIZE + 10, DICE_SIZE + 10, 10, DICE_OUTLINE_COLOR);
        break;
    }
}

void drawDie(int x, int y, int num)
{

    // Draw the White Dice Background
    M5.Lcd.fillRoundRect(x, y, DICE_SIZE, DICE_SIZE, 10, WHITE);

    // Create the Length of Space between Dice Pips
    int xSpaceLength = DICE_SIZE / 4;
    int ySpaceLength = DICE_SIZE / 6;

    // Size of the Pip
    int pipSize = 4;

    // Create x, y coordinates for every possible pip position
    int leftX = x + (xSpaceLength);
    int middleX = x + (DICE_SIZE / 2);
    int rightX = x + (DICE_SIZE - xSpaceLength);
    int topY = y + (ySpaceLength);
    int middleY = y + (DICE_SIZE / 2);
    int bottomY = y + (DICE_SIZE - ySpaceLength);

    // Draw the Pips depending on the Number Rolled
    switch (num)
    {
    case 0:
        M5.Lcd.fillRoundRect(x, y, DICE_SIZE, DICE_SIZE, 10, BLACK);
        break;
    case 1:
        M5.Lcd.fillCircle(middleX, middleY, pipSize, BLACK);
        break;
    case 2:
        M5.Lcd.fillCircle(leftX, topY, pipSize, BLACK);
        M5.Lcd.fillCircle(rightX, bottomY, pipSize, BLACK);
        break;
    case 3:
        M5.Lcd.fillCircle(leftX, topY, pipSize, BLACK);
        M5.Lcd.fillCircle(middleX, middleY, pipSize, BLACK);
        M5.Lcd.fillCircle(rightX, bottomY, pipSize, BLACK);
        break;
    case 4:
        M5.Lcd.fillCircle(leftX, topY, pipSize, BLACK);
        M5.Lcd.fillCircle(rightX, topY, pipSize, BLACK);
        M5.Lcd.fillCircle(leftX, bottomY, pipSize, BLACK);
        M5.Lcd.fillCircle(rightX, bottomY, pipSize, BLACK);
        break;
    case 5:
        M5.Lcd.fillCircle(leftX, topY, pipSize, BLACK);
        M5.Lcd.fillCircle(rightX, topY, pipSize, BLACK);
        M5.Lcd.fillCircle(leftX, bottomY, pipSize, BLACK);
        M5.Lcd.fillCircle(rightX, bottomY, pipSize, BLACK);
        M5.Lcd.fillCircle(middleX, middleY, pipSize, BLACK);
        break;
    case 6:
        M5.Lcd.fillCircle(leftX, topY, pipSize, BLACK);
        M5.Lcd.fillCircle(rightX, topY, pipSize, BLACK);
        M5.Lcd.fillCircle(leftX, middleY, pipSize, BLACK);
        M5.Lcd.fillCircle(rightX, middleY, pipSize, BLACK);
        M5.Lcd.fillCircle(leftX, bottomY, pipSize, BLACK);
        M5.Lcd.fillCircle(rightX, bottomY, pipSize, BLACK);
        break;
    default:
        Serial.print("The number is not a regular dice number.");
        break;
    }
}

void drawHealths()
{
    M5.Lcd.setTextSize(TEXT_SIZE);
    M5.Lcd.setTextColor(WHITE, RED);
    M5.Lcd.fillRect(0, 0, 180, 30, BLACK);
    M5.Lcd.setCursor(5, 10);
    M5.Lcd.printf("P1:%d", PLAYER_ONE_HEALTH);
    M5.Lcd.setCursor(SCREEN_X_BUFFER + 75, 10);
    M5.Lcd.printf("P2:%d", PLAYER_TWO_HEALTH);
}

void drawRerolls()
{
    M5.Lcd.setTextSize(TEXT_SIZE);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(SCREEN_X_BUFFER + 175, 10);
    M5.Lcd.fillRect(SCREEN_X_BUFFER + 170, 0, 125, 25, BLACK);
    M5.Lcd.printf("REROLLS:%d", NUM_OF_REROLLS);
}

////////////////////////////////////////////////////////////////////
// Random Methods
////////////////////////////////////////////////////////////////////
void incrementMove()
{
    currentMove = currentMove + 1;
    if (currentMove >= NUM_OF_MOVES)
    {
        currentMove = currentMove % NUM_OF_MOVES;
    }
}

void decrementMove()
{
    currentMove = currentMove - 1;
    if (currentMove < 0)
    {
        currentMove = NUM_OF_MOVES - 1;
    }
}

int firstActive() {
    if(dice[0] != 0) {
        return 0;
    } else if(dice[1] != 0) {
        return 1;
    } else if(dice[2] != 1) {
        return 2;
    }
    return -1;
}

////////////////////////////////////////////////////////////////////
// Button Methods
////////////////////////////////////////////////////////////////////

void buttonAction(Event& e) {

    // Button Variable
    Button& b = *e.button;

    // Dice One Button
    if(b.instanceIndex() == 6) {
        selectedDice = 0;

        if(isDiceMove()) {

                if (isBlinded && firstActive() == 0) {
                    M5.Lcd.fillRoundRect(SCREEN_X_BUFFER + 75, SCREEN_Y_BUFFER + DICE_SIZE, DICE_SIZE, DICE_SIZE, 10, DARKGREY);
                } else {
                    drawDie(SCREEN_X_BUFFER + 75, SCREEN_Y_BUFFER + DICE_SIZE, dice[selectedDice]);
                }

                // Redraws the Dice, Outlines selectedDice
                drawDiceBG();
                drawOutline();
                drawDice();

                // Draw Submit Button
                drawEnterArrow(RED);
        }
    } 
    // Dice Two Button
    else if(b.instanceIndex() == 7) {
        selectedDice = 1;

        if(isDiceMove()) {

                if (isBlinded && firstActive() == 1) {
                    M5.Lcd.fillRoundRect(SCREEN_X_BUFFER + 75, SCREEN_Y_BUFFER + DICE_SIZE, DICE_SIZE, DICE_SIZE, 10, DARKGREY);
                } else {
                    drawDie(SCREEN_X_BUFFER + 75, SCREEN_Y_BUFFER + DICE_SIZE, dice[selectedDice]);
                }

                // Redraws the Dice, Outlines selectedDice
                drawDiceBG();
                drawOutline();
                drawDice();

                // Draw Submit Button
                drawEnterArrow(RED);
        }
    } 
    // Dice Three Button
    else if(b.instanceIndex() == 8) {
        selectedDice = 2;

        if(isDiceMove()) {

                if (isBlinded && firstActive() == 2) {
                    M5.Lcd.fillRoundRect(SCREEN_X_BUFFER + 75, SCREEN_Y_BUFFER + DICE_SIZE, DICE_SIZE, DICE_SIZE, 10, DARKGREY);
                } else {
                    drawDie(SCREEN_X_BUFFER + 75, SCREEN_Y_BUFFER + DICE_SIZE, dice[selectedDice]);
                }

                // Redraws the Dice, Outlines selectedDice
                drawDiceBG();
                drawOutline();
                drawDice();

                // Draw Submit Button
                drawEnterArrow(RED);
        }
    } 
    // Start Screen Button
    else if(currentState == StartScreen && b.instanceIndex() == 9) {
        initializeGame();
    } 
    // Enter Button
    else if(currentState == StartGame && !awaitResponse && b.instanceIndex() == 10) {
        doMove();
        drawEnterArrow(GREEN);
    }
}

void renderStartScreen()
{
    M5.Lcd.clearDisplay(BLACK);

    // Draw Dice
    drawDie(125, 85, 6);
    drawDie(145, 110, 6);

    // Title
    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(60, 45);
    M5.Lcd.print("Dice Attack!");

    // Draw button or connecting ellipses
    if (currentState == StartScreen)
    {
        M5.Lcd.drawRoundRect(80, 180, 160, 40, 10, WHITE);
        M5.Lcd.setTextSize(2);
        M5.Lcd.setCursor(90, 195);
        M5.Lcd.print("Start Game");
    }
    else if (currentState == AwaitingConnection)
    {
        M5.Lcd.setTextSize(2);
        M5.Lcd.setCursor(90, 195);
        M5.Lcd.print("Connecting...");
    }
}