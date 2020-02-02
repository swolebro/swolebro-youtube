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
// FYI: Some degree of buffer is needed to prevent awful see-sawing.
#define THRESH 5

// Adjustment range for the knob.
#define MINSET 110
#define MAXSET 150

// Naming other pins.
#define ADJUST A0
#define PLASMA A1
#define UP A2
#define DOWN A3

#define BUFSIZE 512  // Would technically let us do running averages up to BUFSIZE samples. In testing, shorter averages seemed better.
#define SAMP 16  // Use this many samples in the average; must be a power of 2 and no larger than BUFSIZE.
#define DISP 1024 // The number of samples to use in calculating a slower average for the display. Must also be a power of 2.

unsigned int shift = 0;

unsigned int values[BUFSIZE] = {0}; // buffer for ADC reads
unsigned long total = 0; // for keeping a rolling total of the buffer
unsigned long disp = 0;  // for separately tracking ADC reads for the display
unsigned long target = 0; // voltage target, in ADC counts

// for tracking when to set opto pins
int diff = 0;
int mean = 0;
int mode = -1;

// generic temp vars
unsigned long tmp = 0;
float ftmp = 0;
float ftmp2 = 0;

// generic looping vars
int i = 0;
int j = 0;

// for the startup adjustment period
unsigned long timelimit = 0;
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

  // We need to calculate how big the shift must be, for a given sample size.
  // Since we are using bitshifting instead of division, I'm using a != here,
  // so your shit will be totally broke if you don't set SAMP to a power of 2.
  while((1 << shift) != SAMP)
    shift++;

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
  i=0;
  ms = millis();
  timelimit = ms + 5000;

  while (ms < timelimit) {
    tmp = analogRead(ADJUST);

    // Keep a rotating total, buffer, and average.  Since this value only moves
    // a small amount due to noise in the AREF voltage and the physical
    // potentiometer itself, 10 samples is fine.
    total = total + tmp - values[i];
    values[i] = tmp;
    target = total / 10;

    // Calculate the setpoint, based on min/max, and chop it to one decimal point.
    ftmp2 = MINSET + ((MAXSET-MINSET) * (target/1023.0));
    ftmp2 = ((int) (ftmp2*10))/10.0;

    if (ftmp != ftmp2) {
      ftmp = ftmp2;
      timelimit = max(timelimit, ms + 2000);
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
  mean = total >> shift;
  diff = mean - target;

  // If the mean is very low, then the plasma is turned off - it's just ADC
  // noise you're seeing and it and should be ignored.
  // This effectively checks if it's less than 2^4, ie. 16 counts, or ~3V with my scale factor.
  if (!(mean>>4)) {
      mode = 0;
      digitalWrite(UP, 0);
      digitalWrite(DOWN, 0);
  }

  // Otherwise, set pins as per reading.
  // Set 0's first to turn off one direction before turning on reverse.
  // We should never have both the UP and DOWN pins set to 1 - that would be nonsense.
  // Checking for current setting before flipping saves a few cycles.
  else if (diff > THRESH) {
    if (mode != 2) {
      mode = 2;
      digitalWrite(UP, 0);
      digitalWrite(DOWN, 1);
    }
  }

  else if (diff < -THRESH) {
    if (mode != 1) {
      mode = 1;
      digitalWrite(DOWN, 0);
      digitalWrite(UP, 1);
    }
  }

  else {
      mode = 0;
      digitalWrite(UP, 0);
      digitalWrite(DOWN, 0);
  }


  // Every DISP reads, update what's displayed on the screen with a slower average.
  // This would be roughly 5 or 6 times per second at our current speeds.
  if (!j) {
    sevseg.setNumber((float) ((disp / DISP) * SCALE), 1);
    disp = 0;
  }

  // Need this to keep the display lit up.
  sevseg.refreshDisplay();

  // Faster than modular arithmetic, by far. Doing that drops us down to ~3kS/sec.
  i = (i + 1) & (SAMP - 1);
  j = (j + 1) & (DISP - 1);
}
