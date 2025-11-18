// DialStatus.pde
// Visual debug UI for the dial cipher puzzle
// Expects serial lines from Arduino like:
//   IDX:1,DIGIT:3,CODE:3--

import processing.serial.*;

Serial serial;
int idx = 0;          // current digit index (0–3)
int digit = 0;        // current dial reading (1–4)
String codeStr = "---"; 
String lastLine = "";

void setup() {
  size(600, 420);
  background(18);
  fill(240);
  textSize(14);

  // Print available ports so you can pick the right one
  println("Available serial ports:");
  printArray(Serial.list());

  // ⚠️ Change this index if needed:
  String portName = Serial.list()[4];  
  serial = new Serial(this, portName, 9600);
}

void draw() {
  background(18);

  fill(240);
  textAlign(LEFT, TOP);
  textSize(20);
  text("Dial Cipher Status", 20, 20);

  textSize(14);
  text("Current Dial (1–4): " + digit, 20, 60);
  text("Locked Digits: " + codeStr, 20, 80);
  text("Digit Index: " + idx + " / 3", 20, 100);

  // big current digit
  textSize(120);
  fill(240);
  textAlign(LEFT, TOP);
  text(digit, 20, 160);

  // raw last serial line (for debugging)
  textSize(12);
  fill(180);
  text("Last line: " + lastLine, 20, height - 40);
}

void serialEvent(Serial s) {
  String line = s.readStringUntil('\n');
  if (line == null) return;
  line = trim(line);
  if (line.length() == 0) return;

  lastLine = line;

  // Match format: IDX:x,DIGIT:y,CODE:zzz
  String[] vals = match(line, "IDX:(\\d+),DIGIT:(\\d+),CODE:(.+)");
  if (vals != null) {
    idx = int(vals[1]);       // 0–3
    digit = int(vals[2]);     // 1–4 (from potToDigit)
    codeStr = vals[3];        // e.g., "4-2" or "41-"
  }
}
