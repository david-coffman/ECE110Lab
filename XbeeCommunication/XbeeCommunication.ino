#define Rx 17
#define Tx 16

void setup() {
  pinMode(45, OUTPUT);
  pinMode(46, OUTPUT);
 
  pinMode(4, INPUT);
  Serial2.begin(9600);
  Serial.begin(9600);
  delay(500);
  Serial.println("Init");
}

void loop() {
  if(Serial2.available()){
    char incoming = Serial2.read();
    if( incoming == '5'){
      digitalWrite(45, LOW);
      Serial.println(incoming);
      delay(500);
    }
  }
  if(digitalRead(4)){
    char outgoing = '5';
    Serial2.write(outgoing);
    digitalWrite(46,LOW);
    Serial.println(outgoing);
    delay(500);
  }
digitalWrite(45, HIGH);
digitalWrite(46, HIGH);
}
