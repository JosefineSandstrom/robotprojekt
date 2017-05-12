#include <Servo.h>
#include <NewPing.h>


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
int pidspeed = 100;


//Variabler i LEDcheck
unsigned long t0;
  
//Tid nu
unsigned long t1;
  
//Tiden för senaste flanken i fyrkantsvågen vi skickar.
unsigned long tsquare;

//Håller koll på om fyrkantsvågen är hög eller låg, börjar låg.
boolean high = false;

unsigned long tboll;


//Pin som styr om servo ska få matning
int servo_matas = 48;
  

//Gör egen P-regulator, detta är "målet" Den ger med nuvarande inställningar stationärt fel på +4, dvs för närvarande ca 14cm från.
int target = 16;
int target_front = 20; //<------- JS

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
const int frontTurn = 6;
const int servoset = 11;
const int ramp = 7;

//skapar delay i början
  boolean sann = true;


void setup() {
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
  state = servoset;
  //Serial.print("start");
  

  


}
//
void loop() {
  while(sann){
    digitalWrite(33, HIGH);
      digitalWrite(37, HIGH);
      digitalWrite(41, HIGH);
    delaymillis(5000);
    sann = false;
    digitalWrite(33, LOW);
      digitalWrite(37, LOW);
      digitalWrite(41, LOW);
  }
  //Serial.print("ll \n");
  switch (state) {


    case servoset:
      stanna();
      digitalWrite(servo_matas, HIGH);
      servo1.attach(6);
            
      servo1.write(90);
      delaymillis(400);
      digitalWrite(servo_matas, LOW);
      servo1.detach();

      servo1.attach(5);
      servo1.write(30);
      delaymillis(1000);
      servo1.detach();
      state = check;
      break;
  
    case forwards:
    digitalWrite(33, LOW);
      digitalWrite(37, LOW);
      digitalWrite(41, LOW);
      //Kör framåt
       turnTest(pidspeed, pidspeed);
      delaymillis(3000);
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
      Serial.print(" cm          ");
      delaymillis(50);
      cm_front = sonar_front.convert_cm(sonar_front.ping());  //<------ JS
      Serial.print(cm_front);
      Serial.print(" rumpa \n");
      //skriv ut i cm
      //Serial.print("\n Output: ");
      //Serial.print(Output);
      if(cm != 0){
        if(cm > target + 7){
          cm = target +7;
        }
        dif = cm - target;
        turnTest(pidspeed - dif, pidspeed + dif);
      }
      
        if((cm_front < target_front) && (cm_front != 0)){  //<------ JS
          state = frontTurn;
        } 


      if((cm_front < 100) && (cm_front > 60)){
        
        stanna();
        delaymillis(500);
        cm = sonar.convert_cm(sonar.ping());
        if((cm > 40) || (cm ==0)){
        state = ramp;
          } else{
            turnTest(pidspeed - dif, pidspeed + dif);
            delaymillis(200);
          }
        }
        


        LEDcheck();
      
        break;    

    case backwards:
      //Kör bakåt
      backward(500);
      //Vänta 500ms
      delaymillis(1000);
      //Kör framåt
      state = check;
      break;

     case irtest:
     LEDcheck();
     break;

     case lift:
     servo1.attach(6);
     digitalWrite(33, LOW);
      digitalWrite(37, HIGH);
      digitalWrite(41, LOW);
     stanna();
     digitalWrite(servo_matas, HIGH);
     // Serial.print("tjena");
    
      millisstart = millis();
      for(int angle = 90; angle < 180; angle+=5 ){
        servo1.write(angle);
      delaymillis(30);
      }
      digitalWrite(servo_matas, LOW);
      delaymillis(2000);
      state = sink;
      break;

    case sink:
      digitalWrite(33, LOW);
      digitalWrite(37, LOW);
      digitalWrite(41, HIGH);
         t1 = millis();
         digitalWrite(servo_matas, HIGH);
        
        //if(t1-millisstart > 1000){
          //for(int angle = 50; angle < 140; angle++){
             servo1.write(90);
             
          //}
         // delaymillis(1000);
         // millisstart = millis();
        //}
       // servo1.write(160);
        delaymillis(500);
        state = check;
        digitalWrite(servo_matas, LOW);
        servo1.detach();
        forward(50);
        delaymillis(2000);
     break;



    case frontTurn:    //<------- JS
    digitalWrite(33, LOW);
        digitalWrite(37, HIGH);
        digitalWrite(41, HIGH);
    
     
    turnRightAlt(350);
    
    stanna();
      

      forward(50);
      state = check;
    break;



    case ramp:
        digitalWrite(33, HIGH);
        digitalWrite(37, HIGH);
        digitalWrite(41, HIGH);
         turnLeftAlt(350);
         stanna();
         delaymillis(100);
       // forward(5000); //OBS! Bara en viss tid!
       // turnLeft(800);
        forward(4500);
        stanna();
        turnRightAlt(350);
        stanna();
        delaymillis(1000);
        backward(1000);
        backwardslow(7000);
        stanna();
        tilt(); //Tippar flaket
        stanna();
        backwardslow(1500);
        state = forwards;
        
    break;
     
  }
 // Serial.print("Hallå \n");
    
    

}



//Roboten kör framåt
//Kan lägga till inparameter för att styra farten, 0-255(full)
void forward(int t) {
  //Bromsar för A & B av
  digitalWrite(brkA, LOW);
  digitalWrite(brkB, LOW);
  //Riktning för A&B är framåt
  digitalWrite(dirA, LOW);
  digitalWrite(dirB, LOW);
  //Full fart för A & B
  analogWrite(3, pidspeed -10);
  analogWrite(11, pidspeed);
  delaymillis(t);
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
void backward(int t) {
  //Broms av för båda
  digitalWrite(brkA, LOW);
  digitalWrite(brkB, LOW);
  //Riktning bakåt för A&B
  digitalWrite(dirA, HIGH);
  digitalWrite(dirB, HIGH);
  //Full fart för båda
  analogWrite(3, 230 - 10);
  analogWrite(11, 230);
  delaymillis(t);
}


void backwardslow(int t) {
  //Broms av för båda
  digitalWrite(brkA, LOW);
  digitalWrite(brkB, LOW);
  //Riktning bakåt för A&B
  digitalWrite(dirA, HIGH);
  digitalWrite(dirB, HIGH);
  //Full fart för båda
  analogWrite(3, pidspeed - 10);
  analogWrite(11, pidspeed);
  delaymillis(t);
}


void rise(){
  servo1.write(140);
  delaymillis(500);

}


//void sink(){
 // servo1.write(45);
 // delaymillis(500);

//}



//void tilt(){
    
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

void tilt(){
  servo1.attach(5);
  servo1.write(100);
  delaymillis(2000);
  servo1.write(30);
  delaymillis(2000);
  servo1.detach();
}


