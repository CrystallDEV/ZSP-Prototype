#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 53
#define RST_PIN 35
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

//counter for defining a dynamic array
int id_counter = 0;
//array of rfids
String ids[4];

//change to false
bool isRunning = true;

MFRC522::MIFARE_Key key; 

int sensor1 = 0;
int sensor2 = 0;
int sensor3 = 0;
int sensor4 = 0;

// motor one
int enA = 3;
int in1 = 1;
int in2 = 2;
// motor two
int enB = 6;
int in3 = 4;
int in4 = 5;

void setup() {
  Serial.begin(9600);  
  setupMotors(); 
  setupRFID();
  setupSensors();
}

void loop() {
  if(isRunning){
    readRFID();
    checkSensors(); 
  }
}

void setupMotors(){
  // set all the motor control pins to outputs
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  
  //left
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  
  //right
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  
  analogWrite(enA, 255);
  analogWrite(enB, 255);
}

void setupRFID() {
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  } 
}

  //infrared sensor setup
void setupSensors() { 
  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
  pinMode(sensor3, INPUT);
  pinMode(sensor4, INPUT); 
}

//if sensor values are LOW they detected a black line
void checkSensors(){
  sensor1 = digitalRead(7); //left
  sensor2 = digitalRead(8); //left inner
  sensor3 = digitalRead(9); //right inner
  sensor4 = digitalRead(10); //right


  //single sensors
  if((sensor1 == HIGH || sensor2 == HIGH) && sensor3 == LOW && sensor4 == LOW){
    left();
  }else if(sensor1 == LOW && sensor2 == LOW && (sensor3 == HIGH || sensor4 == HIGH)){
    right();
  }

  //multiple sensors

  else if(sensor1 == LOW && sensor2 == HIGH && sensor3 == HIGH && sensor4 == LOW){
    forward();
  }else if (sensor1 == HIGH && sensor2 == HIGH && sensor3 == HIGH && sensor4 == HIGH){ 
    forward();
  }else if (sensor1 == LOW && sensor2 == LOW && sensor3 == LOW && sensor4 == LOW){ 
    backward();
  }else if (sensor1 == HIGH && sensor2 == LOW && sensor3 == LOW && sensor4 == HIGH){ 
    forward(); //change to right?
  }else{
    reset();
  }
  
}

void backward(){
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void forward(){
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

void left(){
  digitalWrite(in1, HIGH);
  digitalWrite(in2, HIGH);
  
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

void right(){
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  
  digitalWrite(in3, HIGH);
  digitalWrite(in4, HIGH);  
}

void reset(){
  analogWrite(enA, 255);
  analogWrite(enB, 255);
  
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);  
}

void rotateRight(){
  right();
  delay(3000);
}

void rotateLeft(){
  left();
  delay(3000);
}

void turnAround(){
  right();
  delay(6000);
}

void readRFID(){
    // Look for new cards
    if ( ! rfid.PICC_IsNewCardPresent())
      return;
  
    // Verify if the NUID has been readed
    if ( ! rfid.PICC_ReadCardSerial())
      return;
      
    // Read sector 1 block 4 
    byte trailerBlock   = 7;
    byte sector         = 1;
    byte blockAddr      = 4;
    
    MFRC522::StatusCode status;
    byte buffer[18];
    byte size = sizeof(buffer);

    // Authenticate 
    status = (MFRC522::StatusCode) rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(rfid.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(rfid.GetStatusCodeName(status));
        return;
    }

    // Read data from the block 
    status = (MFRC522::StatusCode) rfid.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(rfid.GetStatusCodeName(status));
    }
    
    if(id_counter <= 4){
      ids[id_counter] = buffer[0] + ":" + buffer[1];
      id_counter++;
    }else{
      executeId(buffer[0] + ":" + buffer[1]);
    }
    
    Serial.print (buffer[0]);
    Serial.print (":");
    Serial.println (buffer[1]);    
    
    // Halt PICC
    rfid.PICC_HaltA();
    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();
}

void executeId(String result){
  if(result == ids[0]){
    isRunning = true;
  }else if(result == ids[1]){
    rotateRight();
  }else if(result == ids[2]){
    turnAround();
  }else if(result == ids[3]){
    isRunning = false;
  }
}



