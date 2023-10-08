// Pins controling shift registers
const int pedestrianWalkDataPin = 2;
const int pedestrianWalkClockPin = 3;
const int pedestrianWalkLatchPin = 4;

const int pedestrianStopDataPin = 5;
const int pedestrianStopClockPin = 6;
const int pedestrianStopLatchPin = 7;

const int redYellowDataPin = 8;
const int redYellowClockPin = 9;
const int redYellowLatchPin = 10;

const int greenLeftDataPin = 11;
const int greenLeftClockPin = 12;
const int greenLeftLatchPin = 13;

// All buttons are connected to this one pin
const int pedestrianWalkButtonPin = A0;

// All motion sensors are connected to this one pin
const int motionSensorPin = A1;

// Transistors controling the LEDs' access to ground
const int pedestrianWalkGroundControl = A2;
const int pedestrianStopGroundControl = A3;
const int trafficLightGroundControl = A4;

const int potPin = A5; // Potentiometer pin

int potValue = 0; // Value of potentiometer

// There's no RTC (Real Time Clock) library in tinkercad so
// I just made an int to act like a timer, each loop it 
// increments one
int DIYClock = 0; 

// Flags to indicate what is going on the intersection
bool NSFlag = true; // NorthSouth (which side of the junction is being affected), if true, top and bottom
bool goFlag = false; // if true, it is a green light & pedestrians can walk
bool walkFlag = true; // if false, green light & pedestrians can't walk
bool yellowFlag = false; // if true, yellow light
bool stopFlag = false; // if true, all light are red
bool leftFlag = true; //  if true, left turn light is on
bool switchFlag = false; // if true, it allows NSFlag to change (changes intersection from red light to green light)
bool switchFlagTrigger = false; // if true, it changes switchFlag
bool pedestrianFlag = false; // if true, it means a pedestrian has pressed the walk button

int walkTime = 0; // amount of second deducted from DIYClock, allows for longer walk time for pedestrians

// Same function from my Name in Lights assignment, allows
// shift register to be updated by setting latch pin to LOW,
// using shiftOut function to write a bit to the shift register,
// and finally setting latch pin to HIGH to register the changes
void updateShiftRegister(byte inputByte, int dataPin, int clockPin, int latchPin){  
	byte inputBit = 0;
    for (int i = 0; i <= 8; i++) {
     digitalWrite(latchPin, LOW); 
      if (bitRead(inputByte, i) == 1) {
        inputBit = 0b10000000;
      } else {
      	inputBit = 0b0;
      }
      shiftOut(dataPin, clockPin, LSBFIRST, inputBit);
      digitalWrite(latchPin, HIGH); 
    }
}

class TrafficLight {
  byte redYellow = 0b0; // byte for red and yellow light LEDs
  byte greenLeft = 0b0; // byte for green and left turn LEDs
  
  // Update each shift register for LEDs and open respective
  // transistor, the transitor is so that you don't see the bits
  // moving from one LED to another, otherwise it makes it seem
  // like LEDs that shouldn't be on are flickering
  void run() {
  	updateShiftRegister(redYellow, redYellowDataPin, redYellowClockPin, redYellowLatchPin);
  	updateShiftRegister(greenLeft, greenLeftDataPin, greenLeftClockPin, greenLeftLatchPin);
  	analogWrite(trafficLightGroundControl, 255);
    delay(5);
    analogWrite(trafficLightGroundControl, 0);
  }
  public:
  // The NS (NorthSouth) parameter is to decide wheather the 
  // function should affect the top and bottom or left and right
  // sides of the intersection

  void go(bool NS) { // Green light
    if (NS == true) {
      redYellow = 0b00100010;
      greenLeft = 0b10001000;
    } else {
      redYellow = 0b10001000;
      greenLeft = 0b00100010;
    }
    run();
  }
  void yellow(bool NS) { // Yellow light
    greenLeft = 0b0;
    if (NS == true) {
      redYellow = 0b01100110;	
    } else {
      redYellow = 0b10011001;
    }
    run();
  }
  void left(bool NS) { // Left turn
    redYellow = 0b10101010;
    if (NS == true) {
       greenLeft = 0b01000100;
    } else {
       greenLeft = 0b00010001;
    }
    run();
  }
  void stop() { // All lights are red
  	redYellow = 0b10101010;
    greenLeft = 0b00;
    run();
  }
};

class Pedestrian {
  byte pedestrianWalk = 0b0; // byte for white LEDs representing walking man
  byte pedestrianStop = 0b0; // byte for orange LEDs representing orange hand
  
  void run() {
    updateShiftRegister(pedestrianWalk, pedestrianWalkDataPin, pedestrianWalkClockPin, pedestrianWalkLatchPin);
  	updateShiftRegister(pedestrianStop, pedestrianStopDataPin, pedestrianStopClockPin, pedestrianStopLatchPin);
  	analogWrite(pedestrianWalkGroundControl, 255);
    delay(5);
    analogWrite(pedestrianWalkGroundControl, 0);
    analogWrite(pedestrianStopGroundControl, 255);
    delay(5);
    analogWrite(pedestrianStopGroundControl, 0);
  }
  public:
  void walk(bool NS) {
    if (NS == true) {
      pedestrianWalk = 0b10011001;
      pedestrianStop = 0b01100110;
    } else {
      pedestrianWalk = 0b01100110;
      pedestrianStop = 0b10011001;
    }
    run();
  }
  void stop() {
    pedestrianWalk = 0b0;
    pedestrianStop = 0b11111111;
    run();
  }
};

// Taking the other two classes and putting them together
class Junction {
  TrafficLight tf;
  Pedestrian p;	
  public:
  void go(bool NS) {
  	tf.go(NS);
    p.walk(NS);
  }
  void left(bool NS) {
  	tf.left(NS);
    p.stop();
  }
  void yellow(bool NS) {
  	tf.yellow(NS);
    p.stop();
  }
  void stop() {
  	tf.stop();
    p.stop();
  }
  void stopWalking(bool NS) {
  	tf.go(NS);
    p.stop();
  }
};

Junction j;

void setup() {
  for (int i = 2; i <= 13; i++) {
  	pinMode(i, OUTPUT);
  }
}

void loop() {
  if (goFlag == true) { // Green light & walk
  	j.go(NSFlag);
  } else if (walkFlag == false) { // Green light & no walking
    j.stopWalking(NSFlag);
  } else if (yellowFlag == true) { // yellow light
  	j.yellow(NSFlag);
  } else if (stopFlag == true) { // all red lights
    j.stop();
  } else if (leftFlag == true) { // left turn
  	j.left(NSFlag);
  }
  
  // Sets flags to true or flase depending on DIYClock time
  if (DIYClock <= 50) {
    switchFlagTrigger == false;
    leftFlag = true;
  } else if (DIYClock - walkTime <= 120 - potValue) {
  	leftFlag = false;
    goFlag = true;
  } else if (DIYClock - walkTime <= 160 - potValue) {
    goFlag = false;
    walkFlag = false;
  } else if (switchFlag == true) { // if switchFlag is not true, then there are neither cars or pedestrians waiting, hence no reason to change lights
    if (switchFlagTrigger == false) { // A trigger is needed to prevent a forever loop of the clock being stuck at 161   
      DIYClock = 161; // Reset the clock incase it runs away
      switchFlagTrigger = true;
    }
    if (DIYClock <= 190) {
      walkFlag = true;
      yellowFlag = true;
    } else if (DIYClock <= 210) {
      yellowFlag = false;
      stopFlag = true;
    } else if (DIYClock <= 260) {
      stopFlag = false;   
    } else {
      if (NSFlag == true) {
        NSFlag = false;
      } else {
        NSFlag = true;
      }
      walkTime = 0; // Reset walk time
      if (pedestrianFlag == true) { // if there is a pedestrian
        walkTime = 10;
        pedestrianFlag = false;
      }
      DIYClock = 0; // Reset the clock
      switchFlag = false;
      potValue = map(analogRead(potPin), 0, 1023, 0, 20);
    }
  }
  
  // If motion sensor detects movement or if a pedestrian
  // presses the walk button, switch flag is set to true
  // also, if a walk button is pressed, set the pedestrian
  // flag to true
  if ((analogRead(motionSensorPin) == 1003 && NSFlag == false) or analogRead(pedestrianWalkButtonPin) > 0) {
    if (analogRead(pedestrianWalkButtonPin) > 0) {
    	pedestrianFlag = true;
    }
    switchFlag = true;
  } else if ((analogRead(motionSensorPin) == 984 && NSFlag == true) or analogRead(pedestrianWalkButtonPin) > 0) {
  	if (analogRead(pedestrianWalkButtonPin) > 0) {
    	pedestrianFlag = true;
    }
    switchFlag = true;
  } else if (DIYClock == 500) {
  	switchFlag = true;
  }
  DIYClock ++;
}
