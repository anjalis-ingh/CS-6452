// ------------------------------------------
// PIS Final Arduino Project - Button Puzzle
// ------------------------------------------

// ----- INCLUDES -----
#include <ezButton.h>                       //button library

// ----- DEFINING VARIABLES -----
#define BUTTON_NUM 4                        // the number of buttons

#define BUTTON1_PIN 2                       //pin connected to button 1 (red)
#define BUTTON2_PIN 3                       //pin connected to button 2 (yellow)
#define BUTTON3_PIN 4                       //pin connected to button 3 (green)
#define BUTTON4_PIN 5                       //pin connected to button 4 (blue)

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


// ----- RESET BUTTONS -----
void resetButtons() 
{
  for (int i = 0; i < BUTTON_NUM; i++)      //loop through buttons
    buttonsPressed[i] = 0;                  //set value to 0

  buttonCount = 0;                          //reset pressed count to 0
}


// ----- SETUP -----
void setup() {
  Serial.begin(9600);                       //begin serial connection

  for (int i = 0; i < BUTTON_NUM; i++) {    //debounce all buttons
    buttonArray[i].setDebounceTime(100);    //set debounce time to 100 milliseconds
  }
}


// ----- LOOP -----
void loop() {
  for (int i = 0; i < BUTTON_NUM; i++)      //for each button
  {
    buttonArray[i].loop();                  //call loop()

    if (buttonArray[i].isPressed())         //if a button is pressed
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
        } 
        else Serial.println("Wrong sequence, try again.");  //otherwise, print try again

        resetButtons();                               // clear for next attempt
      }
    }
  }
}