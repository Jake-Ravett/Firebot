#include <Wire.h>
#include <Adafruit_MLX90614.h> 
#include <SoftwareSerial.h>
#define Left 8         
#define Forward 9
#define Right 10
#define BUTTON_PIN 7
#define COMMAND_INTERVAL 200 // Time interval in milliseconds to check for command combinations

unsigned long lastCommandTime = 0;
char lastCommand = '\0';

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
int LM1 = 2;
int LM2 = 3;
int RM1 = 4;
int RM2 = 5;
int enA = 11;
int enB = 12;
int pump = 13;
int buttonState;         
int lastButtonState = LOW;   
bool toggleState = false;
bool fire = false;
bool event = false;
int lock = 0;
int fwd = 0;
int left = 0;
int right = 0;
int sensor_triggered = 0;
float Objtemp;
float Ambtemp;

void setup() {
  pinMode(LM1, OUTPUT);
  pinMode(LM2, OUTPUT);
  pinMode(RM1, OUTPUT);
  pinMode(RM2, OUTPUT);
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(pump, OUTPUT);
  pinMode(Left, INPUT);
  pinMode(Forward, INPUT);
  pinMode(Right, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  mlx.begin();
  Serial.begin(9600);
}

void check_fire() {
  float Objtemp = mlx.readObjectTempF(); 
  float Ambtemp = mlx.readAmbientTempF();

  if((Objtemp > Ambtemp + 10) && fire == false){
    lock = 1;
    fire = true;
    Serial.println("Fire");
    analogWrite(enA, 0);
	  analogWrite(enB, 0);
    digitalWrite(LM1, HIGH);
    digitalWrite(LM2, HIGH);
    digitalWrite(RM1, HIGH);
    digitalWrite(RM2, HIGH);
    digitalWrite(pump, HIGH);
    delay(5000);
    digitalWrite(pump, LOW);
  }
  else if((Objtemp < Ambtemp + 2) && fire == true){
    fire = false;
    lock = 0;
    fwd = 0;
    right = 0;
    left = 0;
     Serial.println(" NOFire");
    analogWrite(enA, 0);
	  analogWrite(enB, 0);
    digitalWrite(LM1, HIGH);
    digitalWrite(LM2, HIGH);
    digitalWrite(RM1, HIGH);
    digitalWrite(RM2, HIGH);
  }
}

void move_forward() {
  lock = 1;
  fwd = 1;
  Serial.println("Moving Forward");
  analogWrite(enA, 130);
	analogWrite(enB, 130);
  digitalWrite(LM1, HIGH);
  digitalWrite(LM2, LOW);
  digitalWrite(RM1, HIGH);
  digitalWrite(RM2, LOW);
}

void move_backward() { 
  Serial.println("Moving Backward");
  analogWrite(enA, 55);
	analogWrite(enB, 55);
  digitalWrite(LM1, LOW);
  digitalWrite(LM2, HIGH);
  digitalWrite(RM1, LOW);
  digitalWrite(RM2, HIGH);
}

void turn_left() {
  lock = 1;
  left = 1;
  Serial.println("Turning Left");
  analogWrite(enA, 130);
	analogWrite(enB, 130);
  digitalWrite(LM1, HIGH);
  digitalWrite(LM2, LOW);
  digitalWrite(RM1, LOW);
  digitalWrite(RM2, HIGH);
}

void turn_right() {
  lock = 1;
  right = 1;
  Serial.println("Turning Right");
  analogWrite(enA, 130);
	analogWrite(enB, 130);
  digitalWrite(LM1, LOW);
  digitalWrite(LM2, HIGH);
  digitalWrite(RM1, HIGH);
  digitalWrite(RM2, LOW);
}

void loop() {
  int sensorL = digitalRead(Left);
  int sensorR = digitalRead(Right);
  int sensorF = digitalRead(Forward);

  buttonState = digitalRead(BUTTON_PIN);
  if (buttonState != lastButtonState) {
    if (buttonState == HIGH) {
      toggleState = !toggleState;
      Serial.println(toggleState);
    }
    delay(50);
  }
  lastButtonState = buttonState;
  if(toggleState == 0){
    if(sensor_triggered == 1){
      check_fire();
    }
    if(fire == false){
      if (!lock) {
        if (sensorF == 0) {
          sensor_triggered = 1;
          move_forward();
        } 
        else if (sensorL == 0){
          sensor_triggered = 1;
          turn_left();
        } 
        else if (sensorR == 0) {
          sensor_triggered = 1;
          turn_right();
        } 
      } 
      else {
        if (fwd) {
          if (sensorL == 0) {
            lock = 0;
            fwd = 0;
            turn_left();
          } 
          else if (sensorR == 0) {
            lock = 0;
            fwd = 0;
            turn_right();
          } 
        } 
        else if (left) {
          if ((sensorF == 0) || (sensorL == 0 && sensorR == 0 && sensorF == 0)) {
            lock = 0;
            left = 0;
            move_forward();
          } 
          else if (sensorR == 0) {
            lock = 0;
            left = 0;
            turn_right();
          }
        }
        else if (right) {
          if ((sensorF == 0) || (sensorL == 0 && sensorR == 0 && sensorF == 0)) {
            lock = 0;
            right = 0;
            move_forward();
          } 
          else if (sensorL == 0) {
            lock = 0;
            right = 0;
            turn_left();
          }
        }
      }
    }
  }

  if(toggleState == 1) {
    fire = false;
    lock = 0;
    fwd = 0;
    right = 0;
    left = 0;
    sensor_triggered = 0;

    if (Serial.available() > 0) { 
      char data = Serial.read(); 
      Serial.print(data);

      unsigned long currentTime = millis();

      if (millis() - lastCommandTime > COMMAND_INTERVAL && event == true) {
        event = false;    
      }
    
    if ((lastCommand == 'F' && data == 'B' || lastCommand == 'B' && data == 'F') && (currentTime - lastCommandTime <= COMMAND_INTERVAL)) {
          if(event == false){
            event = true;
            digitalWrite(pump, HIGH);
            analogWrite(enA, 0);
	          analogWrite(enB, 0);
            digitalWrite(LM1, HIGH);
            digitalWrite(LM2, HIGH);
            digitalWrite(RM1, HIGH);
            digitalWrite(RM2, HIGH);
            delay(5000);
            digitalWrite(pump, LOW);
          }
    }
    else{
    if(event == false){
      if (data == 'S') {
        analogWrite(enA, 0);
	      analogWrite(enB, 0);
        digitalWrite(LM1, HIGH);
        digitalWrite(LM2, HIGH);
        digitalWrite(RM1, HIGH);
        digitalWrite(RM2, HIGH);
      }
      else if (data == 'F') {
        Serial.println("for");
        analogWrite(enA, 255);
	      analogWrite(enB, 255);
        digitalWrite(LM1, HIGH);
        digitalWrite(LM2, LOW);
        digitalWrite(RM1, HIGH);
        digitalWrite(RM2, LOW);
      }
      else if (data == 'B') {
        Serial.println("back");
        analogWrite(enA, 255);
	      analogWrite(enB, 255);
        digitalWrite(LM1, LOW);
        digitalWrite(LM2, HIGH);
        digitalWrite(RM1, LOW);
        digitalWrite(RM2, HIGH);
      }
      else if (data == 'L') {
        analogWrite(enA, 130);
	      analogWrite(enB, 130);
        digitalWrite(LM1, HIGH);
        digitalWrite(LM2, LOW);
        digitalWrite(RM1, LOW);
        digitalWrite(RM2, HIGH);
      }
      else if (data == 'R') {
        analogWrite(enA, 130);
	      analogWrite(enB, 130);
        digitalWrite(LM1, LOW);
        digitalWrite(LM2, HIGH);
        digitalWrite(RM1, HIGH);
        digitalWrite(RM2, LOW);
      }
    }
    } 
    

    lastCommand = data;
    lastCommandTime = currentTime;
    }
  }
  }
  

  



  
