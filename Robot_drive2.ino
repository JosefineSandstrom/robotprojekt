#include <Servo.h>
#include <NewPing.h>
#include <PID_v1.h>


//Pins på shielden
int dirA = 12;    //vänster
int brkA = 9;
int dirB = 13;    //höger
int brkB = 8;

//Där fyrkantsvågen går ut.
int pin_Square = 10;
//För att interrupta när in från ir-mottagaren ändras
int interruptpin = 20;
//
boolean bollfinns;

//Full ström till motorn
int full = 255;
int fullB = 255;
int fullA = 250;
int pidspeed = 230;


//Variabler i LEDcheck
unsigned long t0;
  
//Tid nu
unsigned long t1;
  
//Tiden för senaste flanken i fyrkantsvågen vi skickar.
unsigned long tsquare;

//Håller koll på om fyrkantsvågen är hög eller låg, börjar låg.
boolean high = false;

unsigned long tboll;

  

//Gör egen P-regulator, detta är "målet" Den ger med nuvarande inställningar stationärt fel på +4, dvs för närvarande ca 14cm från.
int target = 10;

//skillnaden mellan motor A och B i hastighet
double dif = 0;

unsigned long millisstart;

//den frekvens i microsekunder då servon står still
int servostop = 1465;

//skapar servo-objekt
Servo servo1;

//definierar pins för avståndssensor
#define trig 45
#define echo 31
#define maxdist 200

#define trig_front 47 //<------ JS
#define echo_front 29 //<------ JS

//skicka in parametrar till sonar-objekt
NewPing sonar(trig, echo, maxdist);
NewPing sonar_front(trig_front, echo_front, maxdist); //<------ JS

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
const int irtest = 10;



void setup() {
  servo1.attach(6);
  servo1.write(180);
  //serial monitor
  Serial.begin(115200);


  //Vilket värde PID vill ha
  
//  Setpoint = 12;
  //turn the PID on


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
  

  //Pinen som styr fyrkanten börjar låg
  pinMode(pin_Square, OUTPUT);
  digitalWrite(pin_Square, LOW);

  //LEDS som visar state
  pinMode(33, OUTPUT);
  pinMode(37, OUTPUT);
  pinMode(41,OUTPUT);

  //Interruptpinen har input_pullup för att dra upp värdet från sensorn
  pinMode(interruptpin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptpin), interrupt, LOW);
  bollfinns = false;
  
  
  //Grundstatet är:
  state = forwards;
  //Serial.print("start");
  

  


}
//
void loop() {
  
  //Serial.print("ll \n");
  switch (state) {
  
    case forwards:
      //Kör framåt
      forward();
      delaymillis(1000);
      //Kolla om det finns något
      state = check;
      break;

    case check:
      digitalWrite(33, HIGH);
      digitalWrite(37, LOW);
      digitalWrite(41, LOW);
      //skapa variabeln cm som ger avståndet från väggen
      unsigned int cm;
      unsigned int cm_front; //<------ JS
      //unsigned int uS;
      
     
           
      //Spara tiden i uS
      //convertera till cm
      delaymillis(50);
      cm = sonar.convert_cm(sonar.ping());
      Serial.print(cm);
      Serial.print("\n cm");
     
      cm_front = sonar_front.convert_cm(sonar_front.ping());  //<------ JS
      
      //skriv ut i cm
      //Serial.print("\n Output: ");
      //Serial.print(Output);
        if(cm > target + 7){
          cm = target +7;
        }
        dif = cm - target;
        turnTest(pidspeed - 3*dif, pidspeed + 3*dif);

        LEDcheck();
        
        break;    

    case backwards:
      //Kör bakåt
      backward();
      //Vänta 500ms
      delaymillis(1000);
      //Kör framåt
      state = check;
      break;

     case irtest:
     LEDcheck();
     break;

     case lift:
     digitalWrite(33, LOW);
      digitalWrite(37, HIGH);
      digitalWrite(41, LOW);
     stanna();
     // Serial.print("tjena");
    
      millisstart = millis();
      for(int angle = 180; angle > 0; angle--){
        servo1.write(angle);
      delaymillis(15);
      }
      delaymillis(2000);
      state = sink;
      break;

    case sink:
        digitalWrite(33, LOW);
        digitalWrite(37, LOW);
        digitalWrite(41, HIGH);
        unsigned long t1 = millis();
        
      //  Serial.print("DÅ");
       // Serial.print(t1);
        if(t1-millisstart > 1000){
          for(int angle = 0; angle < 180; angle++){
             servo1.write(angle);
             delaymillis(10);
          }
       //   Serial.print("kjhkj \n \n");
          delaymillis(1000);
          //servo1.write(90);
          //state = forwards;
          millisstart = millis();
        }
     //   Serial.print("HEJ");
        delaymillis(200);
        state = check;
     break;
  }
 // Serial.print("Hallå \n");
  

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
  return;
}

//Roboten stannar
void stanna() {
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
  delaymillis(t);
  
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
  delaymillis(t);
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
  delaymillis(t);
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
void LEDcheck(){
  //Tid då vi går in i funktionen
 t0 = micros();
  
  //Tid nu
  t1 = micros();
  
  //Tiden för senaste flanken i fyrkantsvågen vi skickar.
  tsquare = t1 - 60;

  //Håller koll på om fyrkantsvågen är hög eller låg, börjar låg.
  high = false;

  tboll = t1;
  
  //Vi stannar i 100ms
  while(t1 - t0 < 100000){
    t1 = micros();
    //Om det har gått mer än 56us sedan vi skickade förra pulsen
    if(t1-tsquare > 49){

      //Om fyrkantsvågen är hög gör vi den låg
      if(high){
        
        digitalWrite(pin_Square, LOW);
        high = false;
        tsquare = micros();
        
        //Annars, om den är låg blir den hög
      } else{
        
        digitalWrite(pin_Square, HIGH);
        high = true;
        tsquare = micros();
      }   
    }
    if(!bollfinns){
      tboll = micros();
      bollfinns = true;
    }else if(t1-tboll > 90000){
      state = lift;
      return;
    }
    
  }
}
  

void interrupt(){
  bollfinns = false;

}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  //i case check: kolla om servo rakt fram ser vägg/får ett visst värde
  
  case wallTurn:
  //medan servon inte ger värde noll, sväng höger
  while(!= 0){
    
  }
  
  
  
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
