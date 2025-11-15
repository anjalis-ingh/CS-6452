// Combined Puzzle Box
// Puzzle 1: Dial cipher (3 digits via potentiometer + submit)
// Puzzle 2: Button sequence (4 colored buttons)
// Final unlock: servo rotates when BOTH puzzles are solved

#include <ezButton.h>   // for button puzzle
#include <Servo.h>      // for servo latch

// puzzle 1: dial cipher
// pins
// potentiometer 
const int POT_PIN = A1;
// submit button
const int BTN_SUBMIT = 8;     
// buzzer (success and failure) 
const int BUZZER_PIN = 9;

// rgb led pins for dial puzzle
const int RGB_R = 5;
const int RGB_G = 6;
const int RGB_B = 7;

// secret code 
int SECRET[3] = {4, 1, 2}; 

// debounce 
const unsigned long DEBOUNCE_MS = 40;

// how many digits entered (0-2)
int currentDigitIndex = 0;

// digits entered so far
int entered[3] = {0,0,0};  

// debounce - treat a button press as real if the signal is stable for ~40ms
bool lastBtn = HIGH;
unsigned long lastBtnChange = 0;

// flag to track if dial puzzle is solved
bool dialSolved = false;

// convert analog reading from the potentiometer (0-1023) into a digit (1-4)
int potToDigit() {
  int v = analogRead(POT_PIN);

  if (v < 256) {
    return 1;
  } 
  else if (v < 512) {
    return 2;
  } 
  else if (v < 768) {
    return 3;
  } 
  else {
    return 4;
  }
}

// set rgb led color for dial
// HIGH = on, LOW = off 
void setRgb(bool rOn, bool gOn, bool bOn) {
  digitalWrite(RGB_R, rOn ? HIGH : LOW);
  digitalWrite(RGB_G, gOn ? HIGH : LOW);
  digitalWrite(RGB_B, bOn ? HIGH : LOW);
}

// play sound
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

// for processing UI 
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

// flash rgb once when a digit is locked in
void flashDigitConfirm() {
  setRgb(false, false, true);   
  delay(300);
  setRgb(false, false, false);  
}

// reset everything to the start state for the dial puzzle
// used only on wrong code
void resetRound() {
  currentDigitIndex = 0;

  // clear entered digits
  for (int i = 0; i < 3; i++) {
    entered[i] = 0;
  }

  // dial puzzle is not solved yet
  dialSolved = false;

  // turn off rgb led completely
  setRgb(false, false, false);
  sendStatus();
}

// Puzzle 2: Button puzzle

// variables
// the number of buttons
#define BUTTON_NUM 4                        
// red
#define BUTTON1_PIN 10       
// yellow               
#define BUTTON2_PIN 11        
// green              
#define BUTTON3_PIN 12        
// blue               
#define BUTTON4_PIN 13      
// rgb red                
#define BUTTONLED_RED_PIN 2        
// rgb green                         
#define BUTTONLED_GREEN_PIN 3      
// rgb blue                         
#define BUTTONLED_BLUE_PIN 4                

// array to store buttons which have been pressed
int buttonsPressed[BUTTON_NUM];             
// winning code
int buttonCode[BUTTON_NUM] = {1,3,4,1};     
// counting number of buttons which have been pressed
int buttonCount = 0;                        
// bool to check if button code entered is correct
bool buttonCorrect = false;        
// cooldown to have a delay          
unsigned long buttonCooldownUntil = 0;  

// array to store butons
ezButton buttonArray[] = {
  // red
  ezButton(BUTTON1_PIN),   
  // yellow                 
  ezButton(BUTTON2_PIN),         
  // green           
  ezButton(BUTTON3_PIN),    
  // blue                 
  ezButton(BUTTON4_PIN)                    
};

// reset button 
void resetButtons() {
  for (int i = 0; i < BUTTON_NUM; i++)      
    buttonsPressed[i] = 0;                  

  buttonCount = 0;                          
}

// rgb color 
void RGB_color(bool side, int red_light_value, int green_light_value, int blue_light_value) {
  if (side) {
    analogWrite(BUTTONLED_RED_PIN, red_light_value);      
    analogWrite(BUTTONLED_GREEN_PIN, green_light_value);  
    analogWrite(BUTTONLED_BLUE_PIN, blue_light_value);   
  }
}

// servo latch (final lock)
Servo lockServo;
// servo signal on A0 
const int SERVO_PIN = A0;        

// blocks box
const int LOCKED_ANGLE = 65;    
// moves out of the way
const int UNLOCKED_ANGLE = 0;   
// make sure we only unlock once
bool servoUnlocked = false;      

void checkMasterUnlock() {
  // unlock when both puzzles are solved, in any order
  if (!servoUnlocked && dialSolved && buttonCorrect) {
    servoUnlocked = true;
    lockServo.write(UNLOCKED_ANGLE);

    // extra final unlock beep
    buzz(1319,120);
    buzz(1568,120);
  }
}

// setup 
void setup() {
  Serial.begin(9600);                       

  // dial puzzle setup
  pinMode(BTN_SUBMIT, INPUT_PULLUP);

  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  // make sure dial rgb led is off at the very start
  setRgb(false, false, false);

  // button puzzle setup
  for (int i = 0; i < BUTTON_NUM; i++) {    
    buttonArray[i].setDebounceTime(100);    
  }

  pinMode(BUTTONLED_RED_PIN, OUTPUT);
  pinMode(BUTTONLED_GREEN_PIN, OUTPUT);
  pinMode(BUTTONLED_BLUE_PIN, OUTPUT);

   // turn off button puzzle RGB at start
  RGB_color(true, 0, 0, 0);

  // servo setup
  lockServo.attach(SERVO_PIN);
  // start locked
  lockServo.write(LOCKED_ANGLE);            

  // start new rounds
  resetRound();                
  resetButtons();
}

// loop 
void loop() {
  unsigned long now = millis(); 

  // puzzle 2: button sequence handling
  // win state for button code
  if(buttonCorrect) {
    // green side LED when solved
    RGB_color(true, 0, 255, 0);             
  }

  // ongoing state for button code
  else {
    for (int i = 0; i < BUTTON_NUM; i++) {
      buttonArray[i].loop();                  

      // if we're in cooldown after a wrong sequence, ignore presses
      if (now < buttonCooldownUntil) {
        continue;
      }

      if (buttonArray[i].isPressed()) {
        Serial.print("Button ");              
        Serial.print(i + 1);
        Serial.println(" pressed");

        // only record a press if there is room (< 4) 
        if (buttonCount < BUTTON_NUM) {
          if (buttonCount == 0 || buttonsPressed[buttonCount - 1] != (i + 1)) {
            // record this button
            buttonsPressed[buttonCount] = i + 1;
            buttonCount++;

            // blue flash 
            RGB_color(true, 0, 0, 255);  
            buzz(700, 80);                
            delay(300);                   
            RGB_color(true, 0, 0, 0);     
          }
        }

        // check the sequence after 4 buttons clicked 
        if (buttonCount == BUTTON_NUM) {
          Serial.println("Checking sequence...");       

          buttonCorrect = true;                         
          for (int j = 0; j < BUTTON_NUM; j++) {        
            if (buttonsPressed[j] != buttonCode[j]) {   
              buttonCorrect = false;                    
              break;                                    
            }
          }

          if (buttonCorrect) {          
            Serial.println("Button puzzle: You win!");  

            // green + success
            RGB_color(true, 0, 255, 0);   
            successMelody();              
            // buttonCorrect stays true so LED stays green
          } 
          else {
            Serial.println("Wrong sequence, try again.");

            // red + error then reset
            RGB_color(true, 255, 0, 0);   
            errorTriplet();               
            delay(800);                   
            RGB_color(true, 0, 0, 0);   
            buttonCooldownUntil = millis() + 300;   
            resetButtons();               
          }
        }
      }
    }
  }

  // puzzle 1: dial cipher handling 
  // submit button 
  bool btn = digitalRead(BTN_SUBMIT);
  if (btn != lastBtn && (now - lastBtnChange) > DEBOUNCE_MS) {
    lastBtnChange = now;
    lastBtn = btn;

    // on press (LOW = pressed)
    if (btn == LOW && currentDigitIndex < 3 && !dialSolved) {  
      // lock in this digit taken from the current dial position
      int chosen = potToDigit();
      entered[currentDigitIndex] = chosen;

      // visual + audio feedback for digit locked
      flashDigitConfirm();
      buzz(700, 80);

      // go to the next digit
      currentDigitIndex++;

      // if all 3 digits are locked, compare with the secret code
      if (currentDigitIndex >= 3) {
        bool ok = true;
        for (int i = 0; i < 3; i++) {
          if (entered[i] != SECRET[i]) { 
            ok = false; 
            break; 
          }
        }

        if (ok) {
          // success: green + melody, and KEEP status (no reset)
          setRgb(false, true, false);   
          successMelody();       
          // record that this puzzle is solved     
          dialSolved = true;           
        } 
        else {
          // wrong code: red + error beeps, then auto-reset
          setRgb(true, false, false);   
          errorTriplet();
          delay(800);
          // back to all off + fresh attempt
          resetRound();                 
        }
      }

      // status update
      sendStatus();  
    }
  }

  // status update but for the dial digit on-screen in Processing even when it's just rotating
  sendStatus();

  // final servo unlock check
  checkMasterUnlock();
}