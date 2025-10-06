// TEENSY 3.2

#include <MIDI.h>

const BASE_KEY_NOTE = 50;

const LOW_REGISTER_OFFSET = -13;
const MIDDLE_REGISTER_OFFSET = 0;
const HIGH_REGISTER_OFFSET = +13;

const NUM_BLACK_KEYS = 8;
const NUM_WHITE_KEYS = 12;

const MULTIPLEX_0 = 23;
const MULTIPLEX_1 = 24;
const MP_0 = 2;
const MP_1 = 3;
const MP_2 = 4;
const MP_3 = 5;
const KEYS_PER_MULTIPLEX = 10;

const KEY_UP = 0;
const KEY_DOWN = 1;

const THRESHOLD = 200;

boolean key_state[NUM_BLACK_KEYS + MUM_WHITE_KEYS];
int register_offset = 0; // TODO: switch to change register. Remember to stop all notes on current register.

// Create and bind the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();

void setup()
{
    MIDI.begin(MIDI_CHANNEL_OMNI);  // Listen to all incoming messages
}

void loop() {
    loop_keys(MULTIPLEX_0, BASE_KEY_NOTE);
    loop_keys(MULTIPLEX_1, BASE_KEY_NOTE + KEYS_PER_MULTIPLEX);

    MIDI.read(); // TODO: forward to MIDI out?
}

void loop_keys(int touch_pin, int first_key) {
    for (int i = 0; i < KEYS_PER_MULTIPLEX; i++) {
        digitalWrite(MP_0, bitRead(i, 0));
        digitalWrite(MP_1, bitRead(i, 1));
        digitalWrite(MP_2, bitRead(i, 2));
        digitalWrite(MP_3, bitRead(i, 3));

        int current = constrain(map(touchRead(touch_pin), 1000, 5000, 0, 127), 0, 127);

        if (current >= THRESHOLD) {
            if (key_state[i] == KEY_DOWN) {
                key_states[i] = KEY_UP;
                MIDI.sendNoteOn(first_key + i + register_offset, 127, 1);
            }
        } else {
            if (key_state[i] == KEY_UP) {
                key_states[i] = KEY_DOWN;
                MIDI.sendNoteOn(first_key + i + register_offset, 0, 1);
            }
        }
    }
}
