#include <Servo.h>
#include <NewPing.h>

//Pins på shielden
int dirA = 12;
int brkA = 9;
int dirB = 13;
int brkB = 8;

//Full ström till motorn
int full = 255;

//den frekvens i microsekunder då servon står still
int servostop = 1465;

//skapar servo-objekt
Servo servon;



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



void setup() {

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
  servon.attach(5);

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
  switch (state) {
  
    case forwards:
      //Kör framåt
      forward();
      //Vänta i 1000ms
      delay(1000);
      //Kolla om det finns något
      state = check;
      break;

    case check:
      //Stanna
      stanna();
      
      //skapa variabeln uS och cm
      //unsigned int uS;
      unsigned int cm;
      delay(1000);
      
      //Spara tiden i uS
     // uS = sonar.ping();
      //convertera till cm
      cm = sonar.convert_cm(sonar.ping());
            Serial.print(cm);
      Serial.print("\n");
      
      //skriv ut i cm

      //Om vi är närmare än 10cm
      if (cm < 10 && cm != 0) {
        //Hoppa till bakåt
        state = backwards;
        break;
      } if(cm == 0){
        backward();
        delay(300);
        stanna();
        delay(100);
        turnLeft();
        delay(1000);
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
      delay(1000);
      //Kör framåt
      state = check;
      break;
  }

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
  analogWrite(3, full);
  analogWrite(11, full);
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
void turnLeft() {
  //Broms i för A, inte för B
  digitalWrite(brkA, HIGH);
  digitalWrite(brkB, LOW);

  //Riktning för båda är framåt
  digitalWrite(dirA, LOW);
  digitalWrite(dirB, LOW);
  //Full fart för B
  analogWrite(11, full);
}
//Roboten svänger vänster alternativ metod än den för svänga höger. VILKEN ÄR BÄST?????
void turnRight() {
  //broms av för A&B
  digitalWrite(brkA, LOW);
  digitalWrite(brkB, LOW);
  //Riktning fram för A, bakåt för B
  digitalWrite(dirA, LOW);
  digitalWrite(dirB, HIGH);
  //full fart för båda
  analogWrite(3, full);
  analogWrite(11, full);
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

