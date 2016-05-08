#include <Arduino.h>
#include <avr/wdt.h>

#define OUT_PIN 3
#define INTERRUPT_NUMBER 0

const int debounceDelay = 200;
volatile int tapCount = 0;
const int tempoCountConst = 4;
float cas = 15.624;
int counterForDelay = 0;
volatile int turnOnDelayInterval = 600;
volatile boolean turnOnSequence = false;
volatile int sequenceCounter = 0;
long tempos[4];
long timeNow = 0;
long timeForPause = 2000;
volatile long timePre = 0;
long timeSequencePre = 0;
long timeSequnceActualTime = 0;
long const delaHigLow = 50;

void delayBetweenTapsProtection();

void setupTimer();

void setupInterruptRoutin();

void setupWatchDog();

void setupPins();

void setupSerial();

void clearTemposArray();

void delaySequence();

void setup() {

    setupSerial();

    setupPins();

    setupInterruptRoutin();

    setupWatchDog();
}

void setupSerial() { Serial.begin(9600); }

void setupPins() {
    pinMode(OUT_PIN, OUTPUT);
}

void setupWatchDog() { wdt_enable(WDTO_1S); }

void setupInterruptRoutin() { attachInterrupt(INTERRUPT_NUMBER, tapDetect, HIGH); }

int calculateInterval() {
    int countTempos = 0;
    for (int i = 0; i < tempoCountConst / 2; ++i) {
        countTempos += tempos[i + 1] - tempos[i];
    }
    int average = countTempos / 2; //1/4 notes
    average = (average / 4) * 3;
    return average - delaHigLow;
}

void tapDetect() {
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_time > debounceDelay) {
        if (tapCount < tempoCountConst) {
            tempos[tapCount] = interrupt_time;
            timePre = interrupt_time;
            tapCount++;
        }
        if (tapCount == tempoCountConst) {
            turnOnDelayInterval = calculateInterval();
            clearTemposArray();
            turnOnSequence = true;
        }
    }
    last_interrupt_time = interrupt_time;
}

void clearTemposArray() {
    for (int i = 0; i < tempoCountConst; ++i) {
        tempos[i] = 0;
    }
}

void loop() {
    delayBetweenTapsProtection();
    delaySequence();
    wdt_reset();
}

void delaySequence() {
    if (turnOnSequence) {
        timeSequnceActualTime = millis();
        if (timeSequnceActualTime - timeSequencePre >= turnOnDelayInterval) {

            if (sequenceCounter == 6) {
                sequenceCounter = 0;
                tapCount = 0;
                turnOnSequence = false;
            } else {
                PORTD = 0b0001000;
                delay(delaHigLow);
                PORTD = 0b0000000;
                sequenceCounter++;
            }
            timeSequencePre = timeSequnceActualTime;
        }
    }
}

void delayBetweenTapsProtection() {
    timeNow = millis();
    if (tapCount > 0) {
        if (timeNow - timePre >= timeForPause) {
            timePre = timeNow;
            tapCount = 0;
            sequenceCounter = 0;
        }
    }
}

