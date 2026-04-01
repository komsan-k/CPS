/*
 ESP32 Traffic Light Controller
 Red    -> GPIO 23
 Yellow -> GPIO 19
 Green  -> GPIO 18
*/

const int redPin = 23;
const int yellowPin = 19;
const int greenPin = 18;

void setup() {

  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);

}

void loop() {

  // GREEN ON (Cars go)
  digitalWrite(greenPin, HIGH);
  digitalWrite(yellowPin, LOW);
  digitalWrite(redPin, LOW);
  delay(5000);   // 5 seconds

  // YELLOW ON (Prepare to stop)
  digitalWrite(greenPin, LOW);
  digitalWrite(yellowPin, HIGH);
  digitalWrite(redPin, LOW);
  delay(2000);   // 2 seconds

  // RED ON (Stop)
  digitalWrite(greenPin, LOW);
  digitalWrite(yellowPin, LOW);
  digitalWrite(redPin, HIGH);
  delay(5000);   // 5 seconds

}
