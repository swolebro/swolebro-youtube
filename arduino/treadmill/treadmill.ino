#define LOOP ((1<<5)-1)

unsigned long i;
unsigned int buffer[1<<5] = {0};
int total, x, y, z;

void setup(void) {
  delay(3000);
  pinMode(3, OUTPUT);
}

void loop(void) {
  // bit banging for 85Hz PWM, before I
  // ensured the Arduino's native ~500Hz worked

/*
  if (!i) {
    x = (analogRead(A1)/93);
    y = 12 - x;
  }
  digitalWrite(3, HIGH);
  delay(x);
  digitalWrite(3, LOW);
  delay(y);
*/

  // Using the builtin PWM on pin D3.
  // Reading the potentiometer on A1.
  // Keeping a moving average in a circular buffer
  // because hell why not.
  x = analogRead(A1);
  total = total + x - buffer[i];
  buffer[i] = x;
  y = total >> 5;

  // When the moving average changes sufficiently,
  // adjust the PWM setting. The >>2 shifts from
  // the 10-bit space of the ADC to the 8bit space
  // of the PWM. Use max and -25 to ensure it only
  // goes up to 90%, since that's all the control
  // board on the treadmill itself used to allow.
  if (abs(z-y)>25) {
    z = y;
    analogWrite(3, max((z>>2)-25, 0));
  }

  i = (i+1) & LOOP;
}
