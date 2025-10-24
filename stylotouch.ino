// TEENSY 3.2

#include <MIDI.h>

const int BASE_KEY_NOTE = 45;

const int NUM_BLACK_KEYS = 8;
const int NUM_WHITE_KEYS = 12;
const int NUM_KEYS = NUM_BLACK_KEYS + NUM_WHITE_KEYS;

const int MP_SIGNAL = 15;
const int MP_ENABLE = 14;
const int MP_0 = 9;
const int MP_1 = 10;
const int MP_2 = 11;
const int MP_3 = 12;
const int KEYS_PER_MULTIPLEX = 16;

const int TOUCH_16 = 16;
const int TOUCH_17 = 17;
const int TOUCH_18 = 18;
const int TOUCH_19 = 19;

const int LOW_OCTAVE = 2;
const int HIGH_OCTAVE = 3;
const int VIBRATO = 4;

const int KEY_IS_UP = 0;
const int KEY_IS_DOWN = 1;

const int THRESHOLD_DIRECT = 50;
const int THRESHOLD_MUX = 100;

bool key_state[NUM_KEYS];
int register_offset = +0; // TODO: switch to change register. Remember to stop all notes on current register.
bool vibrato = false;

// Create and bind the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();

void setup()
{
    pinMode(MP_SIGNAL, INPUT);
    pinMode(MP_ENABLE, OUTPUT);
    pinMode(MP_0, OUTPUT);
    pinMode(MP_1, OUTPUT);
    pinMode(MP_2, OUTPUT);
    pinMode(MP_3, OUTPUT);

    pinMode(TOUCH_16, INPUT);
    pinMode(TOUCH_17, INPUT);
    pinMode(TOUCH_18, INPUT);
    pinMode(TOUCH_19, INPUT);

    pinMode(VIBRATO, INPUT_PULLDOWN);
    pinMode(HIGH_OCTAVE, INPUT_PULLDOWN);
    pinMode(LOW_OCTAVE, INPUT_PULLDOWN);

    MIDI.begin(MIDI_CHANNEL_OMNI);  // Listen to all incoming messages
    Serial.begin(9600);

    digitalWrite(MP_ENABLE, HIGH);
}

void loop() {
    if (digitalRead(VIBRATO) == HIGH) {
        if (!vibrato) {
            vibrato = true;
            MIDI.sendProgramChange(2, 1);
            Serial.println("Vibrato engaged!");
        }
    } else {
        if (vibrato) {
            vibrato = false;
            MIDI.sendProgramChange(1, 1);
            Serial.println("Vibrato disengaged!");
        }
    }
    
    if (digitalRead(HIGH_OCTAVE) == HIGH) {
        if (register_offset != +12) {
            cancel_all_notes();
            register_offset = +12;
            Serial.println("High octave (C4-C5) engaged!");
        }
    } else if (digitalRead(LOW_OCTAVE) == HIGH) {
        if (register_offset != -12) {
            cancel_all_notes();
            register_offset = -12;
            Serial.println("Low octave (C3-C4) engaged!");
        }
    } else {
        if (register_offset != 0) {
            cancel_all_notes();
            register_offset = 0;
            Serial.println("Medium octave (C2-C3) engaged!");
        }
    }

    multiplex_keys();

    check_key(16, TOUCH_16, THRESHOLD_DIRECT);
    check_key(17, TOUCH_17, THRESHOLD_DIRECT);
    check_key(18, TOUCH_18, THRESHOLD_DIRECT);
    check_key(19, TOUCH_19, THRESHOLD_DIRECT);
}

void multiplex_keys() {
    for (int i = 0; i < KEYS_PER_MULTIPLEX; i++) {
        digitalWrite(MP_0, bitRead(i, 0));
        digitalWrite(MP_1, bitRead(i, 1));
        digitalWrite(MP_2, bitRead(i, 2));
        digitalWrite(MP_3, bitRead(i, 3));

        digitalWrite(MP_ENABLE, LOW);
        check_key(i, MP_SIGNAL, THRESHOLD_MUX);
        digitalWrite(MP_ENABLE, HIGH);
    }
}

void check_key(int index, int pin, int threashold) {
    int current = constrain(map(touchRead(pin), 1000, 5000, 0, 127), 0, 127);

    if (current >= threashold) {
        if (key_state[index] == KEY_IS_UP) {
            key_state[index] = KEY_IS_DOWN;
            MIDI.sendNoteOn(BASE_KEY_NOTE + index + register_offset, 127, 1);
        }
    } else {
        if (key_state[index] == KEY_IS_DOWN) {
            key_state[index] = KEY_IS_UP;
            MIDI.sendNoteOn(BASE_KEY_NOTE + index + register_offset, 0, 1);
        }
    }
}

void cancel_all_notes() {
    for (int i = 0; i < NUM_KEYS; i++) {
        if (key_state[i]) {
            key_state[i] = false;
            MIDI.sendNoteOn(BASE_KEY_NOTE + i + register_offset, 0, 1);
        }
    }
}

  
