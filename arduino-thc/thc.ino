/* A braindead plasma torch THC. Goes along with the board schematic in this repo.

The gist of how this is works is, on startup, you read the potentiometer and
pick a set point. Then it reads the analog pin for the plasma voltage, does
comparisons to that setpoint, and triggers the optocouplers accordingly.

There's a little bit of smoothing that goes into to the ADC reads to improve
the signal's stability a bit, but we can't do too much, since otherwise, the
LinuxCNC can't respond fast enough when the torch height really is changing.

I do a few tricks in here to make sure the loop() function runs quickly. Stuff
like only reading the setpoint on startup (so if you want to adjust it, hit the
reset button), using bitwise shifts for division, etc. Last I checked, this was
able to do about 6000 samples per second, whereas the maximum you could get
from simply looping analogRead() is supposed to be 9000 samples per second
(with 10 bit precision).

Two more noteworthy quirks:

Firstly, while we let the signals for the plasma change rather rapidly, we only
update the display several times a second and give it a much, much longer
average. This just makes it easier on the eyes.

Secondly, this is going to be giving the "UP" signal whenever the plasma
is not cutting (since the voltage will be 0), so it is important to use
LinuxCNC's HAL configs to only respect the THC's signals once the torch is
cutting and is not piercing or cornering. The "thcud" component should help
with that, as well as additional YouTube videos and files coming soon.

(c) The Swolesoft Development Group, 2019
License: http://250bpm.com/blog:82
*/


// Need this for my common-cathod sevseg LCD.
// The pins follow the schematic and were picked to try to make it lay out nicely.
#include <SevSeg.h>

SevSeg sevseg;
byte numDigits = 4;
byte digitPins[4] = {12, 10, 9, 2};
byte segmentPins[8] = {13, 8, 4, 6, 7, 11, 3, 5};

// Setting the scale for the converting analogRead values to volts.
// 4.450 AREF voltage * 50 built-in voltage divider / 1023 resolution = 0.21749755 ADC counts per volt
// As far as I can tell, the arithmetic below *does* get optimized out by the compiler.
#define SCALE (4.450*50/1023)

// Threshold in ADC counts for when we say the torch is out of range.
// Multiply by SCALE for the threshold in volts.
#define THRESH 5

// Adjustment range for the knob.
// Minimum value, the width of each increment (in ADC counts),
// and the size of each increment step.
#define MINSET 110
#define SETWID 12
#define SETINC 0.5

// Naming other pins.
#define ADJUST A0
#define PLASMA A1
#define UP A2
#define DOWN A3

#define BUFSIZE 512  // Would technically let us do running averages up to 512 samples.
#define SAMPSHIFT 4  // Use 2^x samples in the average; see note at end re:modular arithmetic.

unsigned int values[BUFSIZE]; // buffer for ADC reads
unsigned long total = 0; // for keeping a rolling total of the buffer
unsigned long disp = 0;  // for separately tracking ADC reads for the display
unsigned long target = 0; // voltage target, in ADC counts

// for tracking when to set opto pins
int diff = 0;
int mode = -1;

// generic temp vars
unsigned long tmp = 0;
float ftmp = 0;

// generic looping vars
int i = 0;
int j = 0;

unsigned long timestep = 0;
unsigned long ms = 0;


void setup() {

  // The usual.
  pinMode(ADJUST, INPUT);
  pinMode(PLASMA, INPUT);
  pinMode(UP, OUTPUT);
  pinMode(DOWN, OUTPUT);

  // Set the reference voltage to the external linear regulator
  // Do a few throwaway reads so the ADC stabilizes, as recommended by the docs.
  analogReference(EXTERNAL);
  analogRead(PLASMA); analogRead(PLASMA); analogRead(PLASMA); analogRead(PLASMA); analogRead(PLASMA);


  // Set up the LCD. Set an easily identifiable string (looks like "ABCD" all caps)
  // and show it for a moment. This makes it easy to see when the arduino reboots.
  sevseg.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins, 0, 0, 1);
  sevseg.setBrightness(50);
  sevseg.setChars("a8c0");
  for (i = 0; i < 500; i++) {
    sevseg.refreshDisplay();
    delay(1);
  }

  // Now enter the period where you can set the voltage via the potentiomenter.
  // Default 5s period, plus an extension 2s as long as you keep adjusting it.
  // By fixing this after boot, we save cycles from needing to do two ADC reads per loop(),
  // avoid any nonsense from potentiometer drift, and don't need to think about the
  // capacitance of the ADC muxer.
  ms = millis();
  timestep = ms + 5000;
  while (ms < timestep) {
    tmp = analogRead(ADJUST);

    // Keep a rotating total, buffer, and average.
    // Since this value doesn't really move, 10 samples is fine.
    total = total + tmp - values[i];
    values[i] = tmp;
    target = total / 10;

    // We use a different scale here, so we get nice 0.5V
    // increments in the range that matters for our plasma.
    if (ftmp != ((MINSET + (target / SETWID) * SETINC))) {

      // can't get fancy and assign it in the comparison, womp womp
      ftmp = ((MINSET + (target / SETWID) * SETINC));
      timestep = max(timestep, ms + 2000);
      sevseg.setNumber(ftmp, 1);
    }

    i = (i + 1) % 10;
    ms = millis();

    // Need to continuously refresh the display for it to stay lit.
    sevseg.refreshDisplay();
  }

  // Convert the voltage target back into an int, for ADC comparison, with the scale the plasma pin uses.
  target = ftmp / SCALE;

  // Before carrying on, we now reset some of those variables.
  for (i = 0; i < BUFSIZE; i++)
    values[i] = 0;

  total = 0;
  i = 0;
  j = 1; // Keeps display from triggering until we've done BUFSIZE samples.
}


void loop() {

  tmp = analogRead(PLASMA);
  disp += tmp; // non-rolling tally for the lower sample rate display

  // Rolling window for a smaller sample
  total = total + tmp - values[i];
  values[i] = tmp;

  // This mean truncates downwards. Oh well. At least it's fast.
  diff = ((total >> SAMPSHIFT) - target);

  // Set pins as per reading. Set lows first to turn off one direction before turning on reverse.
  // Checking for current setting before flipping saves a few cycles.
  if (diff > THRESH) {
    if (mode != 2) {
      mode = 2;
      digitalWrite(UP, LOW);
      digitalWrite(DOWN, HIGH);
    }

  } else if (diff < -THRESH) {
    if (mode != 1) {
      mode = 1;
      digitalWrite(DOWN, LOW);
      digitalWrite(UP, HIGH);
    }

  } else {
    if (mode != 0) {
      mode = 0;
      digitalWrite(UP, LOW);
      digitalWrite(DOWN, LOW);
    }

  }

  // Every 1024 reads, update what's displayed on the screen with a slower average.
  // This would be roughly 5 or 6 times per second at our current speeds.
  if (!j) {
    sevseg.setNumber((float) ((disp / 1024) * SCALE), 1);
    disp = 0;
  }

  // Need this to keep the display lit up.
  sevseg.refreshDisplay();

  // Faster than mod 16 and mod 1024.
  // This 15 should be based on SAMPSHIFT, in case you want to change the rolling mean window...
  // Need to double-check that the Arduino compiler will optimize that out if I write it that way.
  i = (i + 1) & 15;
  j = (j + 1) & (1023);
}
