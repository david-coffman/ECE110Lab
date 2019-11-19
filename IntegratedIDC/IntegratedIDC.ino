#include <Wire.h> // I2C library, required for MLX90614
#include <SparkFunMLX90614.h> // SparkFunMLX90614 Arduino library
#define Rx 17
#define Tx 16
#include <Servo.h>

Servo left;
Servo right;

int QTIpins[] = {47, 51, 52}; // left, center, right
int QTIdurations[3];
int QTIvalues[3];

int threshold = 100;

// Tracks the number of hashes encountered so far.
int encountered = 0;

int bottleLocation = -1;

IRTherm therm;

// Threshold temperature for "cold" -- probably need to update before IDC.
float thresholdTemperature = 66.0;

// ALL BOT and NON-DINO variables.
int teamResults[4];
int minIndex = 0; 
char finalRoutine;

void setup() {
  // Configure everything.
  configureSensing();
  configureCommunication();
  setUpQTI();
}

void loop() {
  // Stop after all 5 hash marks encountered.
  while(encountered < 5) {
    // Find the next hash mark.
    lineFollow();
    // Record that a hash mark has been found.
    encountered++;
    // Check object temperature.
    if(getTemperature() < thresholdTemperature){
      // Light the LED.
      digitalWrite(2,HIGH);
      // Record bottle location.
      bottleLocation = encountered;
      // Update the LCD if object found.
      updateLCD();
      // Send location of bottle.
      sendCharacter((char) ('e'+encountered));
    }
    // Turn all LEDs (onboard and breadboard) off.
    resetLED();
    if(encountered == 5) {
      while(!receiveCharacter()) {}
      compute();
      Serial3.write(12);
      Serial3.write(13);
      Serial3.write("Lowest bot: " + (char) minIndex);
      while(!receiveFinalRoutine()) {}
    }
  }
}

// Uses the integrated temperature sensor to determine bottle temp.
float getTemperature() {
  therm.read();
  return therm.object();
}

// Configures the temp sensor.
void configureSensing() {
  Serial3.begin(9600);
  therm.begin();
  therm.setUnit(TEMP_F);
  pinMode(2,OUTPUT);
  digitalWrite(2,LOW);
}

// Update the LCD after each temp measurement.
void updateLCD() {
  String topLine = (bottleLocation > 0) ? "COLD @ " + String(bottleLocation) : "NOT FOUND";
  Serial3.write(12);
  Serial3.write(13);
  Serial3.println(topLine);
}

// Send a character with the Xbee.
void sendCharacter(char c) {
  char outgoing = c;
    Serial2.write(outgoing);
    digitalWrite(45, LOW);
    delay(500);
}

// Check for characters received on Xbee.
char receiveCharacter() {
  if(Serial2.available()){
    char incoming = Serial2.read();
    // This will need to change depending on team comm protocol.
    if(incoming == '5'){
      digitalWrite(46, LOW);
      return incoming;
    }
  }
  // Return 0 if nothing to be received.
  return '0';
}

// Turns off onboard and breadboard LEDs.
void resetLED(){
  digitalWrite(2, LOW); 
  digitalWrite(45, HIGH);
  digitalWrite(46, HIGH);
}

// Configures LED and Xbee. 
void configureCommunication(){
  pinMode(45, OUTPUT);
  pinMode(46, OUTPUT);
  pinMode(5, OUTPUT);
  Serial2.begin(9600);
  Serial3.begin(9600);
}

// Configures servos.
void setUpQTI(){
  left.attach(11);
  right.attach(12);
}
  
// Method for checking QTI sensors.
void checkSensors() {
  for(int i = 0; i < sizeof(QTIpins)/2; i++) { 
    long duration = 0;                      
    pinMode(QTIpins[i], OUTPUT);                 
    digitalWrite(QTIpins[i], HIGH);           
    delay(1);                               
    pinMode(QTIpins[i], INPUT);              
    digitalWrite(QTIpins[i], LOW);             
    while(digitalRead(QTIpins[i])){
      // Wait for RC circuit to decay.            
      duration++;                          
    }
    // Convert read value to Boolean.
    QTIdurations[i] = duration;               
    QTIvalues[i] = (duration > threshold);   
  }
}    

// Finds the next hash.
void lineFollow(){
  left.attach(11);
  right.attach(12);
  left.write(100);
  right.write(85);
  delay(200);
  
  while(true) {
    checkSensors();
    // Turn handling: right turn.
    if (QTIvalues[0] and not QTIvalues[2]) { // if left
      left.write(80); // left backwards
      right.write(80); // right forwards
    }
    // Turn handling: left turn.
    else if (QTIvalues[2] and not QTIvalues[0]) {
      left.write(100); // left forwards
      right.write(100); // right backwards
    }
    // Stop handling: reached a hashmark. 
    else if (QTIvalues[0] and QTIvalues[1] and QTIvalues[2]) {
      left.detach();
      right.detach();
      delay(500);
      // Break out of line following.
      return;
    }
    // Nothing to do -- keep moving forward.
    else {
      left.write(100);
      right.write(85);
    } 
  }
}

// Needed for NON-DINO bots.
boolean receiveFinalRoutine() {
  if(Serial2.available()) {
    char incoming = Serial2.read();
    if(incoming >= 'X' && incoming <= 'Z') {
      finalRoutine = incoming;
      return true;
    }
    return false;
  }
}

// receiveCharacter needed only for ALL BOTS.
boolean receiveCharacter() {
  // Team 1: a-e
  // Team 2: f-j
  // Team 3: k-o
  // Team 4: p-t
  // Team 5: not needed since team 5 computes.
    
  if(Serial2.available()){
    char incoming = Serial2.read();

    if(incoming >= 'a' && incoming <= 'e') {
      teamResults[0] = incoming-96;
    }
    if(incoming >= 'f' && incoming <='j') {
      teamResults[1] = incoming-101;
    }
    if(incoming >= 'k' && incoming <='o') {
      teamResults[2] = incoming-106;
    }
    if(incoming >= 'p' && incoming <='t') {
      teamResults[3] = incoming-111;
    }
    for(int i = 0; i < 4; i++) {
      // If some data not yet received, return false.
      if(teamResults[i] == 0) {return false;}
    }
    // If we have all data, return true;
    return true;
  }
}

// compute() needed only for NON-DINO.
void compute() {
  for(int i = 0; i < 4; i++) {
    if(teamResults[i] < teamResults[minIndex]) {
      minIndex = i;
    }
  }
}
