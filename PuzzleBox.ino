// PIS Final Arduino Project - Combined Puzzle Box

// FROM: Button Puzzle code
// ----- INCLUDES -----
#include <ezButton.h>                       //button library
#include <Servo.h>                          // SERVO: for final latch

// ----- DEFINING VARIABLES -----
// NEW: we still have 4 buttons, but changed pins to avoid conflicts with the dial cipher 
#define BUTTON_NUM 4                        // the number of buttons

#define BUTTON1_PIN 4                       //pin connected to button 1 (red)   // NEW: was 2
#define BUTTON2_PIN 5                       //pin connected to button 2 (yellow)// NEW: was 3
#define BUTTON3_PIN 7                       //pin connected to button 3 (green) // NEW: was 4
#define BUTTON4_PIN 11                      //pin connected to button 4 (blue)  // NEW: was 5

int buttonsPressed[BUTTON_NUM];             //array to store buttons which have been pressed
int buttonCode[BUTTON_NUM] = {1,2,3,4};     //winning code
int buttonCount = 0;                        //counting number of buttons which have been pressed
bool correct = false;                       //bool to check if button code entered is correct

ezButton buttonArray[] =                    //array to store butons
{
  ezButton(BUTTON1_PIN),                    //red
  ezButton(BUTTON2_PIN),                    //yellow
  ezButton(BUTTON3_PIN),                    //green
  ezButton(BUTTON4_PIN)                     //blue
};

// NEW: flag to remember when the button puzzle has been solved
bool sequenceSolved = false;


// ----- RESET BUTTONS -----
void resetButtons() 
{
  for (int i = 0; i < BUTTON_NUM; i++)      //loop through buttons
    buttonsPressed[i] = 0;                  //set value to 0

  buttonCount = 0;                          //reset pressed count to 0
}

// FROM: Dial Cipher code
// pins
// potentiometer - dial digits 0-9
const int POT_PIN = A0;
// submit button
const int BTN_SUBMIT = 2;     
// reset button
const int BTN_RESET = 3;    
// buzzer (success and failure) 
const int BUZZER_PIN = 6;
// pins for each digits  
const int PROG_LED[3] = {8, 9, 10};
// success light 
const int GREEN_LED = 12;

// secret code 
int SECRET[3] = {4, 1, 7}; 

// debounce 
const unsigned long DEBOUNCE_MS = 40;

// how many digits entered (0-2)
int currentDigitIndex = 0;

// digits entered so far
int entered[3] = {0,0,0};  

// flag for success
bool unlocked = false;

// debounce - treat a button press as real if the signal is stable for ~40ms
bool lastBtn = HIGH;
unsigned long lastBtnChange = 0;

// helpers
// convert analog reading from the potentiometer (0-1023) into a digit (0-9)
int potToDigit() {
  int v = analogRead(POT_PIN);
  int d = map(v, 0, 1023, 0, 9);
  if (d < 0) {
    d = 0;
  }
  if (d > 9) {
    d = 9;
  }
  return d;
}

// update the three progress LEDs to reflect how many digits are locked-in
void setProgressLeds(int lockedCount) {
  for (int i = 0; i < 3; i++) {
    if (i < lockedCount) {
      // turn this LED on
      digitalWrite(PROG_LED[i], HIGH);   
    } 
    else {
       // turn this LED off
      digitalWrite(PROG_LED[i], LOW);   
    }
  }
}

void buzz(int freq, int ms) {
  tone(BUZZER_PIN, freq, ms);
  delay(ms + 5);
}

// success melody when code is correct 
void successMelody() {
  buzz(880,120); 
  buzz(988,120); 
  buzz(1175,200); 
  delay(40);
}

// error beeps when code is wrong 
void errorTriplet() {
  for (int i = 0; i < 3; i++) { 
    buzz(220,140); 
    delay(50); 
    }
}

// for processing UI -> ex. IDX:2,DIGIT:4,CODE:41-
void sendStatus() {
  int d = potToDigit();

  // build code string
  char code[4];
  for (int i = 0; i < 3; i++) {
    if (i < currentDigitIndex) {
      // convert digit to character
      code[i] = char('0' + entered[i]);  
    } 
    else {
      // placeholder for not-yet-entered digit
      code[i] = '-';                     
    }
  }
  // end-of-string marker for printing
  code[3] = '\0'; 

  Serial.print("IDX:");
  Serial.print(currentDigitIndex);
  Serial.print(",DIGIT:");
  Serial.print(d);
  Serial.print(",CODE:");
  Serial.println(code);
}

// reset everything to the start state 
void resetRound() {
  currentDigitIndex = 0;
  // clear entered digits
  for (int i=0;i<3;i++) {
    entered[i] = 0;
  }
  // turn off progress LEDs and success LED
  setProgressLeds(0);
  digitalWrite(GREEN_LED, LOW);
  // mark as locked 
  unlocked = false;
  sendStatus();
}


// SERVO: Master latch
// SERVO: this servo horn will act as the physical lock/latch
Servo lockServo;
const int SERVO_PIN = 13;            // signal wire for servo

// SERVO: tune these to match LOCKED/UNLOCKED positions on your box
const int LOCKED_ANGLE   = 0;        // horn angle when box is locked
const int UNLOCKED_ANGLE = 90;       // horn angle when box is unlocked

// NEW: to ensure we only "unlock" once when both puzzles are done
bool masterUnlocked = false;

// NEW: helper to check combined win condition and move servo
void checkMasterUnlock() {
  if (!masterUnlocked && unlocked && sequenceSolved) {
    masterUnlocked = true;

    // move servo arm to unlocked position
    lockServo.write(UNLOCKED_ANGLE);

    // victory beeps
    buzz(1319,120);   
    buzz(1568,120);   
  }
}

// COMBINED SETUP
void setup() {
  // FROM: Button Puzzle setup
  Serial.begin(9600);                       //begin serial connection

  for (int i = 0; i < BUTTON_NUM; i++) {    //debounce all buttons
    buttonArray[i].setDebounceTime(100);    //set debounce time to 100 milliseconds
  }

  // FROM: Dial Cipher setup 
  pinMode(BTN_SUBMIT, INPUT_PULLUP);
  pinMode(BTN_RESET,  INPUT_PULLUP);

  for (int i = 0; i < 3; i++) {
    pinMode(PROG_LED[i], OUTPUT);
  }
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // SERVO: attach and start in LOCKED position
  lockServo.attach(SERVO_PIN);
  lockServo.write(LOCKED_ANGLE);

  // start new round (dial) + reset button sequence
  resetRound();                
  resetButtons();
}

// COMBINED LOOP
void loop() {
  unsigned long now = millis(); 
  // FROM: Button Puzzle loop
  for (int i = 0; i < BUTTON_NUM; i++)      //for each button
  {
    buttonArray[i].loop();                  //call loop()

    // Only handle presses if the button puzzle isn't already solved
    if (buttonArray[i].isPressed() && !sequenceSolved)   // NEW: added !sequenceSolved
    {
      Serial.print("Button ");              //print to serial
      Serial.print(i + 1);
      Serial.println(" pressed");

      if (buttonCount < BUTTON_NUM)                   //if less than 4 buttons have been pressed
      {
        buttonsPressed[buttonCount] = i + 1;          //set buttonsPressed number to the number of the button pressed
        buttonCount++;                                //increment button count
      }

      if (buttonCount == BUTTON_NUM)                  // If all 4 have been pressed, check sequence
      {
        Serial.println("Checking sequence...");       //print to serial

        correct = true;                               //set correct to true initially
        for (int j = 0; j < BUTTON_NUM; j++) {        //loop through button code
          if (buttonsPressed[j] != buttonCode[j]) {   //check each against code
            correct = false;                          //if they do not match, set to false
            break;                                    //break if proven false
          }
        }

        if (correct)                                  //if code is correct
        {          
          Serial.println("You win!");                 //print win to serial
          // ENTER WIN STATE CODE HERE                //enter win state
          sequenceSolved = true;                      // NEW: mark button puzzle as solved
        } 
        else {
          Serial.println("Wrong sequence, try again.");  //otherwise, print try again
          resetButtons();                               // clear for next attempt
        }
      }
    }
  }

  // FROM: Dial Cipher loop
  // reset button
  if (digitalRead(BTN_RESET) == LOW) {
    delay(10);
    resetRound();            
    // wait release so holding the button doesn't trigger multiple resets
    while (digitalRead(BTN_RESET) == LOW) {}  
  }

  // submit button 
  bool btn = digitalRead(BTN_SUBMIT);
  if (btn != lastBtn && (now - lastBtnChange) > DEBOUNCE_MS) {
    lastBtnChange = now;
    lastBtn = btn;

    // on press
    if (btn == LOW && !unlocked && currentDigitIndex < 3) {  
      // lock in this digit taken from the current dial position
      int chosen = potToDigit();
      entered[currentDigitIndex] = chosen;

      // visual + audio feedback for digit locked
      setProgressLeds(currentDigitIndex + 1);
      buzz(700, 80);

      // go to the next digit (0->1->2)
      currentDigitIndex++;

      // if all 3 digits are locked, compare with the secret code
      if (currentDigitIndex >= 3) {
        bool ok = true;
        for (int i = 0; i < 3;i++) {
          if (entered[i] != SECRET[i]) { 
            ok = false; break; 
          }
        }

        // success -> allow manual reset to restart new game
        if (ok) {
          digitalWrite(GREEN_LED, HIGH);  
          successMelody();            
          unlocked = true;                
          sendStatus();
        } 
        // wrong code -> reset for a new attempt
        else {
          errorTriplet();
          resetRound();  
        }
      }

      // status update
      sendStatus();  
    }
  }

  // status update but for the dial digit on-screen in Processing even when it's just rotating
  sendStatus();

  // SERVO: Master unlock check
  checkMasterUnlock();
}