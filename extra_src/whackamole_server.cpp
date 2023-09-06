// Some useful resources on BLE and ESP32:
//      https://github.com/nkolban/ESP32_BLE_Arduino/blob/master/examples/BLE_notify/BLE_notify.ino
//      https://microcontrollerslab.com/esp32-bluetooth-low-energy-ble-using-arduino-ide/
//      https://randomnerdtutorials.com/esp32-bluetooth-low-energy-ble-arduino-ide/
//      https://www.electronicshub.org/esp32-ble-tutorial/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <M5Core2.h>

///////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////
static BLEServer *bleServer;
static BLEService *bleService;
static BLECharacteristic *bleCharacteristic;
bool deviceConnected = false;
bool previouslyConnected = false;

// device
int sWidth;
int sHeight;

// game
bool isMyTurn;
int turnsRemaining;
const int TURNS = 10;
const int DICE_DIGITS = 6;

int whackerDiceRolls;
int popperDieRoll;

bool moles[DICE_DIGITS]; // true if mole is up at INDEX

enum Player
{
    WHACKER,
    POPPER
};

enum GameState
{
    CONNECTING,
    SETUP,
    INGAME,
    END,
    ERROR
};

Player thisDevicePlayer;
GameState thisDeviceGameState;

// See the following for generating UUIDs: https://www.uuidgenerator.net/
static BLEUUID SERVICE_UUID("ceb12aed-022d-48f8-8141-f839037559b0");        // raztaz Service
static BLEUUID CHARACTERISTIC_UUID("0832a505-ddbd-4469-9082-e8e0337a88f0"); // raztaz Characteristic

///////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////
void broadcastBleServer();
void drawScreenTextWithBackground(String text, int backgroundColor);

// game methods
void gameSetUp();
void playerRolled();

// helper methods
int dieRoll();
void moleWhack(int rolls);
void molePop(int roll);

int serverReadFromBLE();
void serverWriteToBLE(int rollToWrite);

// drawing methods
void drawDie(int die);
void drawDice(int die1, int die2);
void drawGameDisplay();

///////////////////////////////////////////////////////////////
// BLE Server Callback Methods
///////////////////////////////////////////////////////////////
class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
        previouslyConnected = true;
        Serial.println("Server Device connected...");

        thisDeviceGameState = SETUP;
    }
    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
        Serial.println("Device disconnected...");
    }
};

///////////////////////////////////////////////////////////////
// Put your setup code here, to run once
///////////////////////////////////////////////////////////////
void setup()
{
    // Init device
    M5.begin();
    M5.Lcd.setTextSize(3);
    sWidth = M5.Lcd.width();
    sHeight = M5.Lcd.height();

    // game variable defaults
    isMyTurn = false;                 // true for popper, false for whacker
    thisDeviceGameState = CONNECTING; // trying to connect to another device
    for (int i = 0; i < DICE_DIGITS; i++)
    {
        moles[i] = false;
    }
    turnsRemaining = TURNS;

    // Initialize M5Core2 as a BLE server
    Serial.print("Starting BLE...");
    String bleDeviceName = "RazTaz M5Core2";
    BLEDevice::init(bleDeviceName.c_str());

    // Broadcast the BLE server
    drawScreenTextWithBackground("Initializing BLE...", TFT_CYAN);
    broadcastBleServer();
    drawScreenTextWithBackground("Broadcasting as BLE server named:\n\n" + bleDeviceName, TFT_BLUE);
}

///////////////////////////////////////////////////////////////
// Put your main code here, to run repeatedly
///////////////////////////////////////////////////////////////
void loop()
{
    // not connected to a device yet
    if (thisDeviceGameState == CONNECTING && !deviceConnected)
    {
        drawScreenTextWithBackground("SERVER waiting to connect", BLUE);
        Serial.println("device not connected");

        /////////////////////////////////////////////////////////////////////////////////////////////// TODO remove: because no tabby :(
        // deviceConnected = true;
        // thisDeviceGameState = SETUP;
        /////////////////////////////////////
    }

    // connected to another device, we can start playing!
    if (deviceConnected)
    {
        M5.update();

        // choose your role and join the game!
        if (thisDeviceGameState == SETUP)
        {
            gameSetUp(); // A: whacker, B: start, C: popper
        }

        // game is started
        if (thisDeviceGameState == INGAME)
        {
            switch (thisDevicePlayer)
            {

            case WHACKER:
                if (isMyTurn)
                {
                    if (M5.BtnC.wasPressed()) // roll the die
                    {
                        // roll the dice for new numbers
                        whackerDiceRolls = (dieRoll() * 10) + (dieRoll());

                        // update the game state
                        playerRolled();
                        moleWhack(whackerDiceRolls);

                        // update the UI
                        drawGameDisplay();
                        drawDice(whackerDiceRolls / 10, whackerDiceRolls % 10);

                        delay(1000);

                        // Serial.println("you as the WHACKER rolled a " + String(whackerDiceRolls));
                    }
                }
                else
                {
                    // Serial.println("waiting for popper move");
                    int result = serverReadFromBLE();
                    Serial.println(result);

                    int readTurnsRemaining = result / 10;
                    int readDieOne = result % 10;

                    if (readTurnsRemaining == turnsRemaining) {
                        // popper made a move!
                        molePop(readDieOne);
                        drawGameDisplay();
                        isMyTurn = true;
                    }
                }
                break;

            case POPPER:
                if (isMyTurn)
                {
                    if (M5.BtnC.wasPressed()) // roll the dice
                    {
                        // roll the dice for new numbers
                        popperDieRoll = dieRoll();

                        // update the game state
                        playerRolled();
                        molePop(popperDieRoll);

                        // update the UI
                        drawGameDisplay();
                        drawDie(popperDieRoll);

                        delay(1000);

                        // Serial.println("you as the POPPER rolled a " + String(popperDieRoll));
                    }
                }
                else
                {
                    Serial.println("waiting for whacker move");
                    int result = serverReadFromBLE();

                    int readTurnsRemaining = result / 100;
                    int readDice = result % 100;

                    if (readTurnsRemaining == turnsRemaining + 1) {
                        // whacker made a move!
                        moleWhack(readDice);
                        drawGameDisplay();
                        isMyTurn = true;
                    }
                }
                break;

            default:
                drawScreenTextWithBackground("uh oh, something went wrong. dig again!", RED);
                if (M5.BtnB.wasPressed())
                {
                    thisDeviceGameState == SETUP;
                }
                break;
            }
        }
    }

    // other device disconnected
    if (previouslyConnected && !deviceConnected && thisDeviceGameState != ERROR)
    {
        thisDeviceGameState = ERROR;
        drawScreenTextWithBackground("uh oh, somebody went too far into the hole. dig again by resetting the device!", RED);
    }
}

void gameSetUp()
{
    drawScreenTextWithBackground("A to be whacker, B to start, C to be popper", GREEN);
    if (M5.BtnA.wasPressed())
    {
        thisDevicePlayer = WHACKER;
        isMyTurn = false; // whacker goes second
    }

    else if (M5.BtnB.wasPressed())
    {
        thisDeviceGameState = INGAME;
        drawGameDisplay();
        drawDie(1); // default num
    }
    else if (M5.BtnC.wasPressed())
    {
        thisDevicePlayer = POPPER;
        isMyTurn = true; // popper goes first
    }
}

// writes the player's roll(s) to the BLE characteristic and adjusts their turns
void playerRolled()
{
    int numToWrite;
    numToWrite += turnsRemaining * (thisDevicePlayer == WHACKER ? 100 : 10);
    numToWrite += (thisDevicePlayer == WHACKER ? whackerDiceRolls : popperDieRoll);
    serverWriteToBLE(numToWrite);
    turnsRemaining -= 1;
    isMyTurn = false;
}

// randomly generated roll
// reduces mod bias for 6 (source https://stackoverflow.com/questions/10984974/why-do-people-say-there-is-modulo-bias-when-using-a-random-number-generator)
int dieRoll()
{

    int roll = esp_random();
    int sides;

    // int randMax = INT32_MAX;
    // while (roll >= (randMax - randMax % sides)) {
    //     roll = esp_random();
    // };

    Serial.printf("random %d\t\t", roll);
    roll = (abs(roll) % DICE_DIGITS) + 1;
    Serial.printf("roll %d \n", roll);

    return roll;
}

void moleWhack(int rolls)
{
    int firstRoll = (rolls / 10) - 1;  // -1 for zero indexing
    int secondRoll = (rolls % 10) - 1; // -1 for zero indexing

    // check the first die
    if (moles[firstRoll])
    { // true, that means the mole is up
        moles[firstRoll] = false;
    }

    // check the second die
    if (moles[secondRoll])
    { // true, that means the mole is up
        moles[secondRoll] = false;
    }
}

void molePop(int roll)
{
    int firstRoll = roll - 1; /// -1 for zero indexing

    // put up the mole!
    moles[firstRoll] = true;
}

int serverReadFromBLE()
{
    std::string readValue = bleCharacteristic->getValue();
    String valStr = readValue.c_str();
    int val = valStr.toInt();

    // Serial.printf("\t%d read from BLE\n", val);

    return val;
}

void serverWriteToBLE(int rollToWrite)
{
    String newRoll = String(rollToWrite);
    bleCharacteristic->setValue(newRoll.c_str());

    // Serial.printf("\t %d written to BLE\n", newRoll);
    return;
}


///////////////////////////////////////////////////////////////
// Colors the background and then writes the text on top
///////////////////////////////////////////////////////////////
void drawScreenTextWithBackground(String text, int backgroundColor)
{
    M5.Lcd.fillScreen(backgroundColor);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println(text);
}

///////////////////////////////////////////////////////////////
// This code creates the BLE server and broadcasts it
///////////////////////////////////////////////////////////////
void broadcastBleServer()
{
    // Initializing the server, a service and a characteristic
    bleServer = BLEDevice::createServer();
    bleServer->setCallbacks(new MyServerCallbacks());
    bleService = bleServer->createService(SERVICE_UUID);
    bleCharacteristic = bleService->createCharacteristic(CHARACTERISTIC_UUID,
                                                         BLECharacteristic::PROPERTY_READ |
                                                             BLECharacteristic::PROPERTY_WRITE |
                                                             BLECharacteristic::PROPERTY_NOTIFY |
                                                             BLECharacteristic::PROPERTY_INDICATE);
    bleCharacteristic->setValue("Hello BLE World from Dr. Dan!");
    bleService->start();

    // Start broadcasting (advertising) BLE service
    BLEAdvertising *bleAdvertising = BLEDevice::getAdvertising();
    bleAdvertising->addServiceUUID(SERVICE_UUID);
    bleAdvertising->setScanResponse(true);
    bleAdvertising->setMinPreferred(0x12); // Use this value most of the time
    // bleAdvertising->setMinPreferred(0x06); // Functions that help w/ iPhone connection issues
    // bleAdvertising->setMinPreferred(0x00); // Set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
    Serial.println("Characteristic defined...you can connect with your phone!");
}

void drawGameDisplay()
{
    // background and text color
    uint16_t primaryTextColor;
    M5.Lcd.fillScreen(TFT_DARKGREEN);
    primaryTextColor = TFT_WHITE;

    // variables
    int pad = 10;
    int xOffset = sWidth / 11;
    int halfXOffset = xOffset / 2;
    int holeRad = 25;
    int moleWidth = 30;
    int moleHeight = 25;
    int triWidth = sWidth / 11;
    int vSpace = 10;

    // title
    M5.Lcd.setCursor(pad, pad);
    M5.Lcd.setTextColor(primaryTextColor);
    M5.Lcd.setTextSize(3);
    M5.Lcd.println(thisDevicePlayer == WHACKER ? "Whacker" : "Popper");
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(pad, sHeight - 20);
    M5.Lcd.println("Turns Left:  " + String(turnsRemaining));

    // draw buttons and numbers
    for (int i = 0; i < DICE_DIGITS / 2; i++)
    {
        xOffset = ((i * 2) + 1) * (sWidth / 9);
        int yOffset = 80; // offset for the text

        // if(die == i+1){
        if (moles[i])
        {
            M5.Lcd.fillEllipse(xOffset, yOffset, holeRad, holeRad / 2, TFT_MAROON);
            M5.Lcd.fillRect(xOffset - 15, yOffset - 20, moleWidth, moleHeight, TFT_DARKGREY);
            M5.Lcd.fillRoundRect(xOffset - 15, yOffset - 30, moleWidth, moleHeight, 15, TFT_DARKGREY);
            M5.Lcd.fillTriangle(xOffset - 6, yOffset - 15, xOffset + 6, yOffset - 15, xOffset, yOffset - 29, TFT_LIGHTGREY);
            M5.Lcd.fillEllipse(xOffset, yOffset - 15, 6, 3, TFT_LIGHTGREY);
            M5.Lcd.fillCircle(xOffset, yOffset - 27, 2, TFT_MAGENTA);
            M5.Lcd.fillCircle(xOffset + 10, yOffset - 20, 2, TFT_BLACK);
            M5.Lcd.fillCircle(xOffset - 10, yOffset - 20, 2, TFT_BLACK);
            M5.Lcd.drawLine(xOffset - 2, yOffset - 16, xOffset + 2, yOffset - 16, TFT_BLACK);
        }
        else
        {
            // draw the up arrow
            M5.Lcd.fillEllipse(xOffset, yOffset, holeRad, holeRad / 2, TFT_MAROON);
        }

        yOffset = (yOffset * 2); //+ vSpace;

        // if(die == (i+1)+3){
        if (moles[i + 3])
        {
            M5.Lcd.fillEllipse(xOffset, yOffset, holeRad, holeRad / 2, TFT_MAROON);
            M5.Lcd.fillRect(xOffset - 15, yOffset - 20, moleWidth, moleHeight, TFT_DARKGREY);
            M5.Lcd.fillRoundRect(xOffset - 15, yOffset - 30, moleWidth, moleHeight, 15, TFT_DARKGREY);
            M5.Lcd.fillTriangle(xOffset - 6, yOffset - 15, xOffset + 6, yOffset - 15, xOffset, yOffset - 29, TFT_LIGHTGREY);
            M5.Lcd.fillEllipse(xOffset, yOffset - 15, 6, 3, TFT_LIGHTGREY);
            M5.Lcd.fillCircle(xOffset, yOffset - 27, 2, TFT_MAGENTA);
            M5.Lcd.fillCircle(xOffset + 10, yOffset - 20, 2, TFT_BLACK);
            M5.Lcd.fillCircle(xOffset - 10, yOffset - 20, 2, TFT_BLACK);
            M5.Lcd.drawLine(xOffset - 2, yOffset - 16, xOffset + 2, yOffset - 16, TFT_BLACK);
        }
        else
        {
            // draw the down arrow
            M5.Lcd.fillEllipse(xOffset, yOffset, holeRad, holeRad / 2, TFT_MAROON);
        }
    }
}

void drawDie(int die)
{

    switch (die)
    {
    case 1:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 75, 70, 70, 10, TFT_WHITE);
        M5.Lcd.fillCircle((sWidth * 0.75) + 35, 110, 4, TFT_BLACK); // y = 75 + 35
        break;

    case 2:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 75, 70, 70, 10, TFT_WHITE);
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 92, 4, TFT_BLACK);  // left
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 127, 4, TFT_BLACK); // right
        break;
    case 3:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 75, 70, 70, 10, TFT_WHITE);
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 92, 4, TFT_BLACK);  // left
        M5.Lcd.fillCircle((sWidth * 0.75) + 35, 110, 4, TFT_BLACK); // middle
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 127, 4, TFT_BLACK); // right
        break;
    case 4:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 75, 70, 70, 10, TFT_WHITE);
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 92, 4, TFT_BLACK);  // top left
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 127, 4, TFT_BLACK); // bottom right
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 92, 4, TFT_BLACK);  // top right
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 127, 4, TFT_BLACK); // bottom left
        break;
    case 5:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 75, 70, 70, 10, TFT_WHITE);
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 92, 4, TFT_BLACK);  // top left
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 127, 4, TFT_BLACK); // bottom right
        M5.Lcd.fillCircle((sWidth * 0.75) + 35, 110, 4, TFT_BLACK); // middle
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 92, 4, TFT_BLACK);  // top right
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 127, 4, TFT_BLACK); // bottom left
        break;
    case 6:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 75, 70, 70, 10, TFT_WHITE);
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 92, 4, TFT_BLACK);  // top left
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 127, 4, TFT_BLACK); // bottom right
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 109, 4, TFT_BLACK); // middle left
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 109, 4, TFT_BLACK); // middle right
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 92, 4, TFT_BLACK);  // top right
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 127, 4, TFT_BLACK); // bottom left
        break;
    default:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 100, 75, 75, 10, TFT_WHITE);
        break;
    }
}

void drawDice(int die1, int die2)
{

    switch (die1)
    {
    case 1:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 25, 70, 70, 10, TFT_WHITE);
        M5.Lcd.fillCircle((sWidth * 0.75) + 35, 59, 4, TFT_BLACK); // y = 75 + 35
        break;

    case 2:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 25, 70, 70, 10, TFT_WHITE);
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 42, 4, TFT_BLACK); // left
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 76, 4, TFT_BLACK); // right
        break;
    case 3:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 25, 70, 70, 10, TFT_WHITE);
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 42, 4, TFT_BLACK); // left
        M5.Lcd.fillCircle((sWidth * 0.75) + 35, 59, 4, TFT_BLACK); // middle
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 76, 4, TFT_BLACK); // right
        break;
    case 4:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 25, 70, 70, 10, TFT_WHITE);
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 42, 4, TFT_BLACK); // top left
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 76, 4, TFT_BLACK); // bottom right
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 42, 4, TFT_BLACK); // top right
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 76, 4, TFT_BLACK); // bottom left
        break;
    case 5:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 25, 70, 70, 10, TFT_WHITE);
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 42, 4, TFT_BLACK); // top left
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 76, 4, TFT_BLACK); // bottom right
        M5.Lcd.fillCircle((sWidth * 0.75) + 35, 59, 4, TFT_BLACK); // middle
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 42, 4, TFT_BLACK); // top right
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 76, 4, TFT_BLACK); // bottom left
        break;
    case 6:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 25, 70, 70, 10, TFT_WHITE);
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 42, 4, TFT_BLACK); // top left
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 72, 4, TFT_BLACK); // bottom right
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 59, 4, TFT_BLACK); // middle left
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 59, 4, TFT_BLACK); // middle right
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 42, 4, TFT_BLACK); // top right
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 72, 4, TFT_BLACK); // bottom left
        break;
    default:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 25, 70, 70, 10, TFT_WHITE);
        break;
    }
    switch (die2)
    {
    case 1:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 125, 70, 70, 10, TFT_WHITE);
        M5.Lcd.fillCircle((sWidth * 0.75) + 35, 159, 4, TFT_BLACK); // y = 75 + 35
        break;

    case 2:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 125, 70, 70, 10, TFT_WHITE);
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 142, 4, TFT_BLACK); // left
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 176, 4, TFT_BLACK); // right
        break;
    case 3:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 125, 70, 70, 10, TFT_WHITE);
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 142, 4, TFT_BLACK); // left
        M5.Lcd.fillCircle((sWidth * 0.75) + 35, 159, 4, TFT_BLACK); // middle
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 176, 4, TFT_BLACK); // right
        break;
    case 4:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 125, 70, 70, 10, TFT_WHITE);
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 142, 4, TFT_BLACK); // top left
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 176, 4, TFT_BLACK); // bottom right
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 142, 4, TFT_BLACK); // top right
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 176, 4, TFT_BLACK); // bottom left
        break;
    case 5:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 125, 70, 70, 10, TFT_WHITE);
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 142, 4, TFT_BLACK); // top left
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 176, 4, TFT_BLACK); // bottom right
        M5.Lcd.fillCircle((sWidth * 0.75) + 35, 159, 4, TFT_BLACK); // middle
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 142, 4, TFT_BLACK); // top right
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 176, 4, TFT_BLACK); // bottom left
        break;
    case 6:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 125, 70, 70, 10, TFT_WHITE);
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 142, 4, TFT_BLACK); // top left
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 172, 4, TFT_BLACK); // bottom right
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 159, 4, TFT_BLACK); // middle left
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 159, 4, TFT_BLACK); // middle right
        M5.Lcd.fillCircle((sWidth * 0.75) + 52, 142, 4, TFT_BLACK); // top right
        M5.Lcd.fillCircle((sWidth * 0.75) + 17, 172, 4, TFT_BLACK); // bottom left
        break;
    default:
        M5.Lcd.fillRoundRect(sWidth * 0.75, 125, 70, 70, 10, TFT_WHITE);
        break;
    }
}