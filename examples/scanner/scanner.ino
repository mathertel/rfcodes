
#include <Arduino.h>

#define BACKSIZE 512
#define RECORDSIZE 512
#define SAMPLESIZE (BACKSIZE + RECORDSIZE)

static int startPin = D5;
static int receiverPin = D7;
static int ledPin = D4;

static int interruptPin = 0;

volatile int status;
int lastStatus;

#define STATUS_PAUSE 0x00 // do nothing
#define STATUS_WAIT 0x01 // just fill the buffer for 512
#define STATUS_CHECK 0x02 // check for starting condition
#define STATUS_START 0x04 // record
#define STATUS_ISRECORD (STATUS_WAIT | STATUS_CHECK | STATUS_START)
#define STATUS_DONE 0x80 // done

bool done = false;

typedef uint16_t CodeTime;

// last time the interrupt was called.
volatile static unsigned long lastTime;


// Ring buffer
// A simple ring buffer is used to decouple interrupt routine.
// Static variables are used to be known in the ISR
static CodeTime *buf88; // allocated memory
static volatile CodeTime *buf88_write; // write pointer
static volatile CodeTime *buf88_read; // read pointer
static CodeTime *buf88_end; // end of buffer+1 pointer for wrapping
static volatile unsigned int buf88_cnt; // number of bytes in buffer


// This handler is attached to the change interrupt.
void ICACHE_RAM_ATTR handleInterrupt()
{
  unsigned long now = micros();
  CodeTime t = (CodeTime)(now - lastTime);
  lastTime = now;

  if (status & STATUS_ISRECORD) {
    digitalWrite(ledPin, !digitalRead(receiverPin));

    // record using the write pointer, read pointer is not valid.
    // count may exceed the buffer size
    *buf88_write++ = t;

    // reset pointer to the start when reaching end
    if (buf88_write == buf88_end)
      buf88_write = buf88;
    buf88_cnt++;

    if (status == STATUS_WAIT) {
      if (buf88_cnt >= BACKSIZE) {
        // start checking condition
        status = STATUS_CHECK;
      }

    } else if (status == STATUS_CHECK) {
      // the last timings will be in the ring buffer when looking backwards.

      // ===== CHANGE STARTING CONDITION HERE:

      if ((t > 6000) && (t < 12000)) {
        // a long time is detected, possibly a sync code
        status = STATUS_START;
        buf88_read = buf88_write;
        buf88_cnt = 0;
      } // if

    } else if (status == STATUS_START) {
      // continue recording and count.

      if (buf88_cnt == RECORDSIZE) {
        status = STATUS_DONE;
        digitalWrite(ledPin, HIGH); // off
      }

    } // if
  } // if
} // handleInterrupt

/** dump the data from a table of timings that end with a 0 time. */
void dumpBuffer(CodeTime *raw, int size)
{

  // dump timing probes
  CodeTime *p = raw;
  int len = 0;

  if (p < buf88)
    p += SAMPLESIZE;

  while (len < size) {
    Serial.print(*p);
    if (len % 32 != 31) {
      Serial.print(',');
    } else {
      Serial.println(',');
    }
    p++;
    len++;

    // reset pointer to the start when reaching end
    if (p == buf88_end)
      p = buf88;
  } // while
} // dumpBuffer()


void setup()
{
  delay(3000);
  Serial.begin(115200);

  Serial.println("RF Scanner...");

  status = STATUS_PAUSE;

  // setup ring buffer
  buf88 = (CodeTime *)malloc(SAMPLESIZE * sizeof(CodeTime)); // allocated memory
  buf88_end = buf88 + SAMPLESIZE; // end of buffer+1 pointer for wrapping
  buf88_write = buf88;
  buf88_read = buf88;
  buf88_cnt = 0;

  // setup interrupt routine
  interruptPin = digitalPinToInterrupt(receiverPin);
  attachInterrupt(interruptPin, handleInterrupt, CHANGE);

  pinMode(ledPin, OUTPUT);
  pinMode(startPin, INPUT_PULLUP);
  pinMode(receiverPin, INPUT_PULLUP);

  // show activity.
  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPin, LOW);
    delay(200);
    digitalWrite(ledPin, HIGH);
    delay(400);
  }
} // setup()


void loop()
{
  if (status == STATUS_PAUSE) {
    if (digitalRead(startPin) == LOW) {
      Serial.println("wait...");
      lastTime = micros();
      status = STATUS_WAIT; // Now interrupt will start recording any timings
    }

  } else if (status == STATUS_WAIT) {

  } else if (status == STATUS_CHECK) {
    if (lastStatus != status)
      Serial.println("check...");

  } else if (status == STATUS_START) {
    if (lastStatus != status)
      Serial.print("collect...");
    Serial.print('.'); // show some progress
    delay(30);

  } else if (status == STATUS_DONE) {
    // report timings
    Serial.println("done.");

    // print buffer before read

    dumpBuffer((CodeTime *)buf88_read - BACKSIZE, BACKSIZE);
    Serial.println("---");
    dumpBuffer((CodeTime *)buf88_read, RECORDSIZE);
    Serial.println();

    // done.
    status = STATUS_PAUSE;
  }
  lastStatus = status;
} // loop()

// End.
