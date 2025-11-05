// just to see the digit number for now!

import processing.serial.*;

Serial serial;
int idx = 0, digit = 0;
String codeStr = "---";
String lastLine = "";

void setup() {
  size(600, 420);
  printArray(Serial.list());

  String portName = Serial.list()[4];
  serial = new Serial(this, portName, 9600);
}

void draw() {
  background(18);
  fill(240);
  textAlign(LEFT, TOP);
  textSize(20);
  text("Dial Status", 20, 20);

  textSize(14);
  text("Current Dial: " + digit, 20, 60);
  text("Locked Digits: " + codeStr, 20, 80);
  text("Digit Index: " + idx + " / 3", 20, 100);

  textSize(120);
  fill(240);
  text(digit, 20, 150);
}

void serialEvent(Serial s) {
  String line = s.readStringUntil('\n');
  if (line == null) return;
  line = trim(line);
  if (line.length() == 0) return;

  lastLine = line;

  String[] vals = match(line, "IDX:(\\d+),DIGIT:(\\d+),CODE:(.+)");
  if (vals != null) {
    idx = int(vals[1]);
    digit = int(vals[2]);
    codeStr = vals[3];
  }
}
