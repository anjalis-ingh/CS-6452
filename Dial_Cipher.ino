// dial cipher

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

// setup
void setup() {
  Serial.begin(9600);                     
  pinMode(BTN_SUBMIT, INPUT_PULLUP);
  pinMode(BTN_RESET,  INPUT_PULLUP);

  for (int i = 0; i < 3; i++) {
    pinMode(PROG_LED[i], OUTPUT);
  }
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // start new round
  resetRound();                
}

// loop
void loop() {
  unsigned long now = millis(); 

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
}