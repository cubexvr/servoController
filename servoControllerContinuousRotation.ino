/*
   1 period = 0.5us
*/
// 2ms = 0.5 us * 4000 (can be adapted to needs)
const unsigned int samplePeriod = 4000;

// 25us == 40kHz (should not be changed, faster is not possible)
const unsigned int minPeriod = 49;
const unsigned int maxPeriod = samplePeriod;

// 3us (should not be changed, timing is critical)
const unsigned int pulseLength = 5;

// faster may be possible, but is not needed
const unsigned long baudRate = 115200;

// pulses needed for fullAngle (360 deg)
const unsigned int fullAngle = 10000;
const unsigned int halfAngle = fullAngle / 2;

const unsigned int startMark = 255;

const byte motors = 4;

volatile unsigned int targetPos[motors];
unsigned int currentPos[motors];
volatile int incPos[motors];
volatile boolean stopSignal[motors];
volatile boolean isRunning[motors];

unsigned int targetInput[motors];

/* Pins */

// pulse 0
const byte ocr1bPin = 12;
// pulse 1
const byte ocr3bPin = 2;
// pulse 2
const byte ocr4bPin = 7;
// pulse 3
const byte ocr5bPin = 45;

// direction
const byte dir0 = 30;
const byte dir1 = 31;
const byte dir2 = 32;
const byte dir3 = 33;

byte dir[] = {dir0, dir1, dir2, dir3};
byte pulse[] = {ocr1bPin, ocr3bPin, ocr4bPin, ocr5bPin};

// fast digital write for direction pins
#define setDirDown(b) PORTC |= (b)
#define setDirUp(b) PORTC &=~ (b)

const byte dir0bit = B10000000;
const byte dir1bit = B01000000;
const byte dir2bit = B00100000;
const byte dir3bit = B00010000;


/* clock divider for timers /8 = 2MHz*/
const byte PRESCALE = B10;

// for direction pins
const bool UP = LOW;
const bool DOWN = HIGH;


void setup() {
  noInterrupts();
  initTimers();

  for (int i = 0; i < motors; i++) {
    targetPos[i] = 0;
    currentPos[i] = 0;
    incPos[i] = 0;
    pinMode(pulse[i], OUTPUT);
    pinMode(dir[i], OUTPUT);
    digitalWrite(pulse[i], LOW);
    digitalWrite(dir[i], UP);
  }

  interrupts();
  Serial.begin(baudRate);
}


void loop() {
  while (Serial.available() < 1) {
    ;
  }
  if (Serial.read() == startMark) {
    while (Serial.available() < 1) {
      ;
    }
    if (Serial.read() == startMark) {
      while (Serial.available() < motors * 2) {
        ;
      }
      for (int i = 0; i < motors; i++) {
        targetInput[i] = mapToRange(Serial.read() << 8 | Serial.read());
      }
      move0(targetInput[0]);
      move1(targetInput[1]);
      move2(targetInput[2]);
      move3(targetInput[3]);
    }
  }
}


unsigned int mapToRange(unsigned int angle) {
  if (angle > fullAngle - 1) {
    return fullAngle - 1;
  }
  return angle;
}


void initTimers() {
  // disable everything that we don't need, probably not needed
  PCICR = 0;
  PCMSK0 = 0;
  PCMSK1 = 0;
  PCMSK2 = 0;
  WDTCSR = 0;

  // disable 8 bit timers
  TIMSK0 = 0;
  TIFR0 = 0;
  TCCR0A = 0;
  TCCR0B = 0;
  TCNT0 = 0;
  OCR0A = 0;

  TIMSK2 = 0;
  TIFR2 = 0;
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;
  OCR2A = 0;

  // The 16 bit timers that are used for generating the pulses
  TCNT1 = 0;
  OCR1A = 0;
  OCR1B = pulseLength;
  TCCR1A = 0;
  TCCR1B = 0;
  ICR1 = 0;
  TIMSK1 = 0;
  TIFR1 = 0;

  TCNT3 = 0;
  OCR3A = 0;
  OCR3B = pulseLength;
  TCCR3A = 0;
  TCCR3B = 0;
  ICR3 = 0;
  TIMSK3 = 0;
  TIFR3 = 0;

  TCNT4 = 0;
  OCR4A = 0;
  OCR4B = pulseLength;
  TCCR4A = 0;
  TCCR4B = 0;
  ICR4 = 0;
  TIMSK4 = 0;
  TIFR4 = 0;

  TCNT5 = 0;
  OCR5A = 0;
  OCR5B = pulseLength;
  TCCR5A = 0;
  TCCR5B = 0;
  ICR5 = 0;
  TIMSK5 = 0;
  TIFR5 = 0;
}

void stopTimer(byte motor) {
  switch (motor) {
    case 0:
      TCCR1B = B00011000;
      TIMSK1 = 0;
      break;
    case 1:
      TCCR3B = B00011000;
      TIMSK3 = 0;
      break;
    case 2:
      TCCR4B = B00011000;
      TIMSK4 = 0;
      break;
    case 3:
      TCCR5B = B00011000;
      TIMSK5 = 0;
      break;
  }
}

void startTimerWithCrop(byte motor, unsigned int period) {
  if (period < minPeriod) {
    period = minPeriod;
  }
  else if (period > maxPeriod) {
    period = maxPeriod;
  }
  startTimer(motor, period);
}

void startTimer(byte motor, unsigned int period) {
  switch (motor) {
    case 0:
      TCNT1 = period - 1;
      ICR1 = period; // Timer TOP
      TIMSK1 |= (1 << OCIE1A); // oca interrupt when counter = 0
      // fast PWM mode, clear ocr bits on compare match
      TCCR1A = B10101010;
      TCCR1B = B00011000 | PRESCALE;
      break;
    case 1:
      TCNT3 = period - 1;
      ICR3 = period; // Timer TOP
      TIMSK3 |= (1 << OCIE3A); // oca interrupt when counter = 0
      // fast PWM mode, clear ocr bits on compare match
      TCCR3A = B10101010;
      TCCR3B = B00011000 | PRESCALE;
      break;
    case 2:
      TCNT4 = period - 1;
      ICR4 = period; // Timer TOP
      TIMSK4 |= (1 << OCIE4A); // oca interrupt when counter = 0
      // fast PWM mode, clear ocr bits on compare match
      TCCR4A = B10101010;
      TCCR4B = B00011000 | PRESCALE;
      break;
    case 3:
      TCNT5 = period - 1;
      ICR5 = period; // Timer TOP
      TIMSK5 |= (1 << OCIE5A); // oca interrupt when counter = 0
      // fast PWM mode, clear ocr bits on compare match
      TCCR5A = B10101010;
      TCCR5B = B00011000 | PRESCALE;
      break;
  }
}

void move0(unsigned int target) {
  if (targetPos[0] != target) {
    if (isRunning[0]) {
      stopSignal[0] = true;
      while (isRunning[0]) {
        ;
      }
      stopSignal[0] = false;
    }
    unsigned int current = currentPos[0];
    if (target != current) {
      if (target > current) {
        targetPos[0] = target;
        isRunning[0] = true;
        if (target - current > halfAngle) {
          setDirDown(dir0bit);
          incPos[0] = -1;
          startTimerWithCrop(0, samplePeriod / (fullAngle - target + current));
        } else {
          setDirUp(dir0bit);
          incPos[0] = 1;
          startTimerWithCrop(0, samplePeriod / (target - current));
        }
      } else {
        targetPos[0] = target;
        isRunning[0] = true;
        if (current - target > halfAngle) {
          setDirUp(dir0bit);
          incPos[0] = 1;
          startTimerWithCrop(0, samplePeriod / (fullAngle - current + target));
        } else {
          setDirDown(dir0bit);
          incPos[0] = -1;
          startTimerWithCrop(0, samplePeriod / (current - target));
        }
      }
    }
  }
}

void move1(unsigned int target) {
  if (targetPos[1] != target) {
    if (isRunning[1]) {
      stopSignal[1] = true;
      while (isRunning[1]) {
        ;
      }
      stopSignal[1] = false;
    }
    unsigned int current = currentPos[1];
    if (target != current) {
      if (target > current) {
        targetPos[1] = target;
        isRunning[1] = true;
        if (target - current > halfAngle) {
          setDirDown(dir1bit);
          incPos[1] = -1;
          startTimerWithCrop(1, samplePeriod / (fullAngle - target + current));
        } else {
          setDirUp(dir1bit);
          incPos[1] = 1;
          startTimerWithCrop(1, samplePeriod / (target - current));
        }
      } else {
        targetPos[1] = target;
        isRunning[1] = true;
        if (current - target > halfAngle) {
          setDirUp(dir1bit);
          incPos[1] = 1;
          startTimerWithCrop(1, samplePeriod / (fullAngle - current + target));
        } else {
          setDirDown(dir1bit);
          incPos[1] = -1;
          startTimerWithCrop(1, samplePeriod / (current - target));
        }
      }
    }
  }
}

void move2(unsigned int target) {
  if (targetPos[2] != target) {
    if (isRunning[2]) {
      stopSignal[2] = true;
      while (isRunning[2]) {
        ;
      }
      stopSignal[2] = false;
    }
    unsigned int current = currentPos[2];
    if (target != current) {
      if (target > current) {
        targetPos[2] = target;
        isRunning[2] = true;
        if (target - current > halfAngle) {
          setDirDown(dir2bit);
          incPos[2] = -1;
          startTimerWithCrop(2, samplePeriod / (fullAngle - target + current));
        } else {
          setDirUp(dir2bit);
          incPos[2] = 1;
          startTimerWithCrop(2, samplePeriod / (target - current));
        }
      } else {
        targetPos[2] = target;
        isRunning[2] = true;
        if (current - target > halfAngle) {
          setDirUp(dir2bit);
          incPos[2] = 1;
          startTimerWithCrop(2, samplePeriod / (fullAngle - current + target));
        } else {
          setDirDown(dir2bit);
          incPos[2] = -1;
          startTimerWithCrop(2, samplePeriod / (current - target));
        }
      }
    }
  }
}

void move3(unsigned int target) {
  if (targetPos[3] != target) {
    if (isRunning[3]) {
      stopSignal[3] = true;
      while (isRunning[3]) {
        ;
      }
      stopSignal[3] = false;
    }
    unsigned int current = currentPos[3];
    if (target != current) {
      if (target > current) {
        targetPos[3] = target;
        isRunning[3] = true;
        if (target - current > halfAngle) {
          setDirDown(dir3bit);
          incPos[3] = -1;
          startTimerWithCrop(3, samplePeriod / (fullAngle - target + current));
        } else {
          setDirUp(dir3bit);
          incPos[3] = 1;
          startTimerWithCrop(3, samplePeriod / (target - current));
        }
      } else {
        targetPos[3] = target;
        isRunning[3] = true;
        if (current - target > halfAngle) {
          setDirUp(dir3bit);
          incPos[3] = 1;
          startTimerWithCrop(3, samplePeriod / (fullAngle - current + target));
        } else {
          setDirDown(dir3bit);
          incPos[3] = -1;
          startTimerWithCrop(3, samplePeriod / (current - target));
        }
      }
    }
  }
}


// timing is critical, change nothing here
ISR(TIMER1_COMPA_vect, ISR_BLOCK) {
  currentPos[0] += incPos[0];
  if (currentPos[0] == 65535) {
    currentPos[0] = fullAngle - 1;
  } else if (currentPos[0] > fullAngle - 1) {
    currentPos[0] = 0;
  }
  if (stopSignal[0] || currentPos[0] == targetPos[0]) {
    TCCR1B = B00011000;
    TIMSK1 = 0;
    isRunning[0] = false;
  }
}

ISR(TIMER3_COMPA_vect, ISR_BLOCK) {
  currentPos[1] += incPos[1];
  if (currentPos[1] == 65535) {
    currentPos[1] = fullAngle - 1;
  } else if (currentPos[1] > fullAngle - 1) {
    currentPos[1] = 0;
  }
  if (stopSignal[1] || currentPos[1] == targetPos[1]) {
    TCCR3B = B00011000;
    TIMSK3 = 0;
    isRunning[1] = false;
  }
}

ISR(TIMER4_COMPA_vect, ISR_BLOCK) {
  currentPos[2] += incPos[2];
  if (currentPos[2] == 65535) {
    currentPos[2] = fullAngle - 1;
  } else if (currentPos[2] > fullAngle - 1) {
    currentPos[2] = 0;
  }
  if (stopSignal[2] || currentPos[2] == targetPos[2]) {
    TCCR4B = B00011000;
    TIMSK4 = 0;
    isRunning[2] = false;
  }
}

ISR(TIMER5_COMPA_vect, ISR_BLOCK) {
  currentPos[3] += incPos[3];
  if (currentPos[3] == 65535) {
    currentPos[3] = fullAngle - 1;
  } else if (currentPos[3] > fullAngle - 1) {
    currentPos[3] = 0;
  }
  if (stopSignal[3] || currentPos[3] == targetPos[3]) {
    TCCR5B = B00011000;
    TIMSK5 = 0;
    isRunning[3] = false;
  }
}
