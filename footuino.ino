#include <Debounce.h> // https://github.com/wkoch/Debounce
#include <ResponsiveAnalogRead.h>
#include <MIDIUSB.h>

#define EXP_1 3
#define EXP_2 2
#define SW_1 2
#define SW_2 3
#define SW_3 4
#define R_LED_1 6
#define R_LED_2 7
#define G_LED_1 8
#define G_LED_2 9

Debounce sw_1(SW_1, 80, true); // PIN number, debounce delay (ms), INPUT_PULLUP (bool)
Debounce sw_2(SW_2, 80, true); // PIN number, debounce delay (ms), INPUT_PULLUP (bool)
Debounce sw_3(SW_3, 80, true); // PIN number, debounce delay (ms), INPUT_PULLUP (bool)
ResponsiveAnalogRead exp_1(EXP_1, true, 0.01); // PIN number, enable sleep (bool)
ResponsiveAnalogRead exp_2(EXP_2, true, 0.01); // PIN number, enable sleep (bool)

const int MIDI_CH = 0; // MIDI Channel (0-15)
const int MIDI_CC_1[2][2] = { // Switch 1 CC# (≤63 off, ≥64 on), Expression 1 CC# (0-127)
  {82, 11},
  {83, 7}
};
const int MIDI_CC_2 = 4; // Expression 2 CC# (0-127)
const int MIDI_CC_SW_2 = 80; // Switch 2 CC# (≤63 off, ≥64 on)
const int MIDI_CC_SW_3 = 81; // Switch 3 CC# (≤63 off, ≥64 on)

bool previous_sw_1 = false;
bool previous_sw_2 = false;
bool previous_sw_3 = false;

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = { 0x0B, 0xB0 | channel, control, value };
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}

void setup() {
  Serial.begin(9600);
  
  pinMode(EXP_1, INPUT);
  pinMode(EXP_2, INPUT);
  pinMode(SW_1, INPUT);
  pinMode(SW_2, INPUT_PULLUP);
  pinMode(SW_3, INPUT_PULLUP);
  pinMode(R_LED_1, OUTPUT);
  pinMode(R_LED_2, OUTPUT);
  pinMode(G_LED_1, OUTPUT);
  pinMode(G_LED_2, OUTPUT);
}

void loop() {
  exp_1.update(); // Update the ResponsiveAnalogRead object every loop
  exp_2.update(); // Update the ResponsiveAnalogRead object every loop

  // Check buttons state
  if(sw_1.read() != previous_sw_1) {
    // SWITCH LED COLOR
    digitalWrite(R_LED_1, sw_1.read());
    digitalWrite(R_LED_2, sw_1.read());
    digitalWrite(G_LED_1, !sw_1.read());
    digitalWrite(G_LED_2, !sw_1.read());

    // SEND MIDI CC FOR SWITCH AND FLUSH
    controlChange(MIDI_CH, MIDI_CC_1[!sw_1.read()][0], 0);
    controlChange(MIDI_CH, MIDI_CC_1[sw_1.read()][0], 127);
  }

  if(sw_2.read() != previous_sw_2) {
    // SEND MIDI CC FOR SWITCH AND FLUSH
    controlChange(MIDI_CH, MIDI_CC_SW_2, sw_2.read()*127);
  }

  if(sw_3.read() != previous_sw_3) {
    // SEND MIDI CC FOR SWITCH AND FLUSH
    controlChange(MIDI_CH, MIDI_CC_SW_3, sw_3.read()*127);
  }
  
  // Check expressions state
  if(exp_1.hasChanged()) {
    // SEND MIDI CC FOR EXP AND FLUSH
    controlChange(MIDI_CH, MIDI_CC_1[sw_1.read()][1], map(exp_1.getValue(), 0, 1023, 0, 127));
  }
  
  if(exp_2.hasChanged()) {
    // SEND MIDI CC FOR EXP AND FLUSH
    controlChange(MIDI_CH, MIDI_CC_2, map(exp_2.getValue(), 0, 1023, 0, 127));
  }

  // CONSOLE
  Serial.println((String) "EXP#" + MIDI_CC_1[digitalRead(SW_1)][1] + " (" + analogRead(EXP_1) + ") " + "SW#" + MIDI_CC_1[digitalRead(SW_1)][0] + " (" + digitalRead(SW_1) + ")");
  Serial.println((String) "EXP#" + MIDI_CC_2 + " (" + analogRead(EXP_2) + ")");
  Serial.println((String) "SW#" + MIDI_CC_SW_2 + " (" + digitalRead(SW_2) + ")");
  Serial.println((String) "SW#" + MIDI_CC_SW_3 + " (" + digitalRead(SW_3) + ")");
  
  previous_sw_1 = sw_1.read();
  previous_sw_2 = sw_2.read();
  previous_sw_3 = sw_3.read();

  delay(10);
}