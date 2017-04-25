#include <Servo.h>
#include <NewPing.h>
#include <PID_v1.h>

// Testrad yabadabadoo

//Pins på shielden
int dirA = 12;    //vänster
int brkA = 9;
int dirB = 13;    //höger
int brkB = 8;

//Full ström till motorn
int full =255;
int fullB = 255;
int fullA = 250;

unsigned long millisstart;

//den frekvens i microsekunder då servon står still
int servostop = 1465;

//skapar servo-objekt
Servo servo1;



//definierar pins för avståndssensor
#define trig 45
#define echo 31
#define maxdist 200

//skicka in parametrar till sonar-objekt
NewPing sonar(trig, echo, maxdist);

//Variabler för avståndscheck
int duration;
float distance;
float meter;

//Här är states för state-machine
int state;
const int forwards = 1;
const int check = 2;
const int backwards = 3;
const int lift = 4;
const int sink = 5;



void setup() {
  servo1.attach(5);
  servo1.write(90);
  //serial monitor
  Serial.begin(115200);

  //definiera vilka pins som gör vad.
  pinMode(dirA, OUTPUT);
  pinMode(brkA, OUTPUT);
  pinMode(dirB, OUTPUT);
  pinMode(brkB, OUTPUT);

  //Hjul A: framåt, broms i
  digitalWrite(dirA, LOW);
  digitalWrite(brkA, LOW);
  
  //Hjul B: framåt, broms i
  digitalWrite(dirB, LOW);
  digitalWrite(brkB, LOW);

  //Vilken pin servon är fäst i
  

  //Startlägen för avståndsmätaren
  pinMode(trig, OUTPUT);
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  pinMode(echo, INPUT);
  

  //Grundstatet är check
  state = check;
  

  


}
//
void loop() {
  
  Serial.print("ll \n");
  switch (state) {
  
    case forwards:
      //Kör framåt
      forward();
      delaymillis(50);
      //Kolla om det finns något
      state = check;
      break;

    case check:
      //Stanna
      //stop();
      
      //skapa variabeln uS och cm
      //unsigned int uS;
      unsigned int cm;
      delaymillis(1000);
      
      //Spara tiden i uS
     // uS = sonar.ping();
      //convertera till cm
      cm = sonar.convert_cm(sonar.ping());
            Serial.print(cm);
      Serial.print("\n");
      
      //skriv ut i cm

      //Om vi är närmare än 10cm
//      if (cm < 10 && cm != 0) {
//        //Hoppa till bakåt
//        turnRight(500);
//        turnLeftAlt(200);
//        forward();
//        break;
//      } if(cm >15){
//        turnLeft(500);
//        turnRightAlt(150);
//        forward();
//        delay(50);
//        break;
//      }

      if (cm < 10 && cm != 0) {
        //Hoppa till bakåt
        turnTest(fullA, 225);
        delaymillis(10);
        break;
      } if(cm >12){
        turnTest(205, fullB);
        delaymillis(10);
        break;
      }
      
      //Annars hoppa till framåt
      state = forwards;
      break;

      break;
    case backwards:
      //Kör bakåt
      backward();
      //Vänta 500ms
      delaymillis(1000);
      //Kör framåt
      state = check;
      break;

     case lift:
      Serial.print("tjena");
    
      millisstart = millis();
      servo1.write(100);
      state = sink;
      break;

    case sink:
        unsigned long t1 = millis();
        
        Serial.print("DÅ");
        Serial.print(t1);
        if(t1-millisstart > 1000){
          servo1.write(85);
          Serial.print("kjhkj \n \n");
          delaymillis(1000);
          servo1.write(90);
          state = forwards;
          millisstart = millis();
        }
        Serial.print("HEJ");
     break;
  }
  Serial.print("Hallå \n");
  

}

//Roboten kör framåt
//Kan lägga till inparameter för att styra farten, 0-255(full)
void forward() {
  //Bromsar för A & B av
  digitalWrite(brkA, LOW);
  digitalWrite(brkB, LOW);
  //Riktning för A&B är framåt
  digitalWrite(dirA, LOW);
  digitalWrite(dirB, LOW);
  //Full fart för A & B
  analogWrite(3, fullA);
  analogWrite(11, fullB);
}

//Roboten stannar
void stop() {
  //Broms i för A&B
  digitalWrite(brkA, HIGH);
  digitalWrite(brkB, HIGH);
  //Riktning framåt för A&B   onödigt??
  digitalWrite(dirA, LOW);
  digitalWrite(dirB, LOW);
}

//Roboten svänger vänster
void turnLeft(int t) {
  //Broms i för A, inte för B
  digitalWrite(brkA, HIGH);
  digitalWrite(brkB, LOW);

  //Riktning för båda är framåt
  digitalWrite(dirA, LOW);
  digitalWrite(dirB, LOW);
  //Full fart för B
  analogWrite(11, full);
  delay(t);
}

void turnRight(int t){
  //Broms i för B, inte för A
  digitalWrite(brkA, LOW);
  digitalWrite(brkB, HIGH);

  //Riktning för båda är framåt
  digitalWrite(dirA, LOW);
  digitalWrite(dirB, LOW);
  //Full fart för A
  analogWrite(3, full);
  delay(t);
  
}

void turnTest(int speedA, int speedB) {
  //Bromsar för A & B av
  digitalWrite(brkA, LOW);
  digitalWrite(brkB, LOW);
  //Riktning för A&B är framåt
  digitalWrite(dirA, LOW);
  digitalWrite(dirB, LOW);
  //Full fart för A & B
  analogWrite(3, speedA);
  analogWrite(11, speedB);
}

void turnLeftAlt(int t) {
  //broms av för A&B
  digitalWrite(brkA, LOW);
  digitalWrite(brkB, LOW);
  //Riktning fram för B, bakåt för A
  digitalWrite(dirA, HIGH);
  digitalWrite(dirB, LOW);
  //full fart för båda
  analogWrite(11, full);
  analogWrite(3, full);
  delay(t);
}

//Roboten svänger höger alternativ metod än den för svänga höger. VILKEN ÄR BÄST?????
void turnRightAlt(int t) {
  //broms av för A&B
  digitalWrite(brkA, LOW);
  digitalWrite(brkB, LOW);
  //Riktning fram för A, bakåt för B
  digitalWrite(dirA, LOW);
  digitalWrite(dirB, HIGH);
  //full fart för båda
  analogWrite(3, full);
  analogWrite(11, full);
  delay(t);
}

//Roboten kör bakåt
void backward() {
  //Broms av för båda
  digitalWrite(brkA, LOW);
  digitalWrite(brkB, LOW);
  //Riktning bakåt för A&B
  digitalWrite(dirA, HIGH);
  digitalWrite(dirB, HIGH);
  //Full fart för båda
  analogWrite(3, full);
  analogWrite(11, full);
}

void rise(){
  servo1.write(140);
  delaymillis(500);

}


//void sink(){
 // servo1.write(45);
 // delaymillis(500);

//}
void delaymillis(unsigned long t){
 unsigned long t0 = millis();
 unsigned long t1 = millis();
  while(t1 - t0 < t){
    t1 = millis();
    if(t1-t1>1000){
      return;
    }
    
  }
  return;
}

