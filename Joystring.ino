#include <Arduino.h>
#include <BleCombo.h>

#define DEBUG 1 // 1 - DEBUG ON, 0 - DEBUG OFF

// Button Pins
#define BUTTON_PIN_LEFT 19  // GPIO12 for left click
#define BUTTON_PIN_RIGHT 18  // GPIO14 for right click
#define BUTTON_PIN_JT 4   // GPIO2 for joystick button


// DEBUG RGB-LED Pins
#define RED_LED_PIN 13
#define GREEN_LED_PIN 12
#define BLUE_LED_PIN 14


// Joystick Pins
#define VRX_JOYSTICK 36
#define VRY_JOYSTICK 39


// Configuration Flags
#define INVERT_Y false
#define INVERT_X false

// Battery Pin
#define BATV_PIN 23


// Device and Mouse Mode Settings
// uint8_t device_mode = 1;            // 1 - Laptop, 0 - Phone
volatile uint8_t mouse_mode = 0;    // 0 - Pointer, 1 - Scroll, 2-YT Player
uint8_t range = 0;               // Analog conversion range


// Debounce variables
unsigned long lastLeftClickTime = 0;
unsigned long lastRightClickTime = 0;
unsigned long lastJTTime = 0;
unsigned long JTPressTime = 0;
const unsigned long DEBOUNCE_DELAY = 50; // 50ms debounce time
const unsigned long YT_TIME = 2000;


// Button state tracking
bool lastJTButtonState = HIGH;  // Initialize to HIGH for pull-up input
bool lastRightButtonState = HIGH; 
bool lastLeftButtonState = HIGH;


// Mouse object
BleComboKeyboard bleCombo("Joystring", "ECE 4180", 100);

// Utility functions

// Sets RGB color (0 : Off, 1 : Red, 2 : Green, 3 : Blue)
void setRGB(uint8_t in){
  switch(in){
    case 0:
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, LOW);
      digitalWrite(BLUE_LED_PIN, LOW);
      break;
    case 1:
      digitalWrite(RED_LED_PIN, HIGH);
      digitalWrite(GREEN_LED_PIN, LOW);
      digitalWrite(BLUE_LED_PIN, LOW);
      break;
    case 2:
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, HIGH);
      digitalWrite(BLUE_LED_PIN, LOW);
      break;
    case 3: 
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, LOW);
      digitalWrite(BLUE_LED_PIN, HIGH);
      break;
    default:
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, LOW);
      digitalWrite(BLUE_LED_PIN, LOW);
  }
}
void setup() {
  Serial.begin(9600);
  while(!Serial) delay(500);
  Keyboard.begin();
  Mouse.begin();

  // Configure button pins with internal pull-up resistors
  pinMode(BUTTON_PIN_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_PIN_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_PIN_JT, INPUT_PULLUP);

  if(DEBUG){
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);
  }
 
  // Initialize Bluetooth Mouse
  bleCombo.begin();
 
  // Wait for Bluetooth connection
  while(!Keyboard.isConnected()){
    Serial.printf("Waiting for BlueAtooth Peripheral...\n");
    if(DEBUG) setRGB(3);
    delay(500);
    if(DEBUG) setRGB(0);
    delay(500);
  }
  Serial.printf("Joystring Mouse connected !\n");
  if(DEBUG){
    setRGB(2); // Blink green
    delay(1000);
    setRGB(0);
    delay(400);
    setRGB(2);
    delay(1000);
    setRGB(2);
  }

  // Battery Level Transmission
  uint8_t batteryLevel;
  int bat_input = analogRead(BATV_PIN);
  delay(500);
  Serial.printf("\n%d", bat_input);
  if(bat_input >= 1023){
    batteryLevel = 100;
  }else if(bat_input <= 0){
    batteryLevel = 0;
  }else{
    batteryLevel = (bat_input * 100) / 1023;
  }
  Keyboard.setBatteryLevel(batteryLevel);
  Serial.printf("\n%u", batteryLevel);

}


void loop() {
  /************************ JT Button Polling with State Switching ************************/
  if((digitalRead(BUTTON_PIN_JT) == LOW && lastJTButtonState == HIGH)){
    lastJTButtonState = LOW;
    unsigned long currentTime = millis();
    if (currentTime - lastJTTime > DEBOUNCE_DELAY) {
      mouse_mode=!mouse_mode;
      if(mouse_mode==1){
        Serial.println("Scroll Mode");
        setRGB(3);
      }else{
        Serial.println("Pointer Mode"); 
        setRGB(2);
      }
      lastJTTime = currentTime;
      JTPressTime = currentTime;
    }
  }
  if((digitalRead(BUTTON_PIN_JT) == LOW && lastJTButtonState == LOW)){
    unsigned long currentTime = millis();
    if (currentTime - JTPressTime > YT_TIME){
      mouse_mode = 2;
      setRGB(1);
    }
  }
  if (digitalRead(BUTTON_PIN_JT) == HIGH && lastJTButtonState == LOW){
    lastJTButtonState = HIGH;
    unsigned long currentTime = millis();
    if (currentTime - lastLeftClickTime > DEBOUNCE_DELAY) {
      lastLeftClickTime = currentTime;
    }
  }
  
  /************************ Left Button Click Polling ************************/
  if (digitalRead(BUTTON_PIN_LEFT) == LOW && lastLeftButtonState == HIGH){
    lastLeftButtonState = LOW;
    unsigned long currentTime = millis();
    if (currentTime - lastLeftClickTime > DEBOUNCE_DELAY) {
      Mouse.click(MOUSE_LEFT);
      lastLeftClickTime = currentTime;
      Serial.println("Left Click");
    }
  }
  if (digitalRead(BUTTON_PIN_LEFT) == HIGH && lastLeftButtonState == LOW){
    lastLeftButtonState = HIGH;
    unsigned long currentTime = millis();
    if (currentTime - lastLeftClickTime > DEBOUNCE_DELAY) {
      Mouse.click(MOUSE_LEFT);
      lastLeftClickTime = currentTime;
    }
  }

  /************************ Right Button Click Polling ************************/
  if (digitalRead(BUTTON_PIN_RIGHT) == LOW && lastRightButtonState == HIGH){
    lastLeftButtonState = LOW;
    unsigned long currentTime = millis();
    if (currentTime - lastRightClickTime > DEBOUNCE_DELAY) {
      Mouse.click(MOUSE_RIGHT);
      lastRightClickTime = currentTime;
      Serial.println("Right Click");
    }
  }
  if (digitalRead(BUTTON_PIN_RIGHT) == HIGH && lastRightButtonState == LOW){
    lastRightButtonState = HIGH;
    unsigned long currentTime = millis();
    if (currentTime - lastRightClickTime > DEBOUNCE_DELAY) {
      Mouse.click(MOUSE_RIGHT);
      lastRightClickTime = currentTime;
    }
  }

  // Joystick movement handling
  int16_t VrxReading = analogRead(VRX_JOYSTICK) + 270;
  int16_t VryReading = analogRead(VRY_JOYSTICK) + 285;
  
  // Set range based on device mode
  switch(mouse_mode){
    case 0: range = 15;
    break;
    case 1: range = 1;
    break;
    case 2: range = 10;
    break;
  }

  int16_t VrxValue = map(VrxReading, 0, 4095, -range, range);
  int16_t VryValue = map(VryReading, 0, 4095, -range, range);


  // Invert axes if needed
  if (INVERT_Y) {
    VryValue = range - VryValue;
    if (VryValue < 0)
      VryValue = -VryValue;
  }
  if (INVERT_X) {
    VrxValue = range - VrxValue;
    if (VrxValue < 0)
      VrxValue = -VrxValue;
  }

  // Mouse movement based on device and mouse mode
  if(mouse_mode == 0){ // Pointer mode
    Mouse.move(VrxValue, VryValue, 0);
  }
  else if(mouse_mode == 1){ // Scroll mode
    Mouse.move(0, 0, VryValue, VrxValue);
  }else if(mouse_mode == 2){ // Scroll mode
    if(VrxValue > 7){
      Keyboard.write(KEY_RIGHT_ARROW);
      delay(100);
    }
    if(VrxValue < -7){
      Keyboard.write(KEY_LEFT_ARROW);
      delay(100);
    }
    if(VryValue > 7){
      Keyboard.write(KEY_DOWN_ARROW);
      delay(100);
    }
    if(VryValue < -7){
      Keyboard.write(KEY_UP_ARROW);
      delay(100);
    }
  }

  // Small delay to prevent overwhelming the system
  delay(10);
}

