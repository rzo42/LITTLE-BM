/*******************************************************************************

TITLE;        LITTLE BM 
SKETCH BY;    RYAN LISTON
DATE;         JULY 3, 2025
DESCRIPTION;  mini portable bitwise frequency modulation synthesizer
FEATURES;     -or/xor bitwise frquency modulation
              -modulation  frequency sync mode
              -lfo controlled tremelo and vibrato
              -4 lfo wave shapes at 2 speeds + lfo frequency control
                waveforms=square,triangle,saw,ramp,square*2,triangle*2,saw*2,ramp*2
              -tremelo waveform inversion
              -carrier frequency range of 5 octaves
              -modulation frequency range of 2 octaves
              -4 bit linear pulse amplitude modulation
              -ldr for optic theramin
              -earphone/line level output
              -speaker output with lm386 amplifier
              -9v power input
              -5v usb power through arduino (if not using the lm386)

PARTS LIST    -arduino nano, uno or compatable clones
              -lm386 audio power amplifier
              -8 leg ic socket 
              -8 10k potentiometers
              -6 spdt switches
              -3 .33 mm jacks
              -prototyping board
              -wire
              -220 uf capacitor
              -22 uf capacitor
              -10 uf capacitor
              -2* 100 nf capacitors
			  -50 nf capacitor
              -6* 470 ohm resistors
              -ldr              
              -2* 10 ohm resistor
              -100 ohm resistor
              -5* 2k resistors
              -4* 1k resistors
              -47k resistor                                                   

******************************************************************************/

#include <avdweb_AnalogReadFast.h>

/******************************************************************************/

//  DEFINE CONTROL INPUTS

#define notePot A2        //  analog input for carrier frequency control
#define fmPot A3          //  analog input for modulation frequency control
#define lfoSpeedPot A5    //  analog input for lfo frequency speed control
#define lfoShapePot A6    //  analog input for lfo shape control
#define vibratoPot A7     //  analog input for vibrato depth control
#define tremeloPot A4     //  analog input for tremelo depth control
#define orXorSwitch 5     //  digital input for or/xor modulation mode control
#define syncSwitch 6      //  digital input for modulation syncronization mode control
#define inverterSwitch 7  //  digital input for tremelo depth inversion control

/******************************************************************************/

//  DECLARE VOLATILES SHARED BY THE MAIN LOOP AND THE ISR

volatile uint8_t frequency;  //  carrier frequency pointer
volatile uint8_t fm;         //  modulation frequency pointer
volatile uint8_t gain;       //  lfo amplitude modulation differential gail level
volatile bool orXor;         //  or/xor fm mode state
volatile bool sync;          //  fm sync state

/******************************************************************************/

//  SETUP

void setup() {

  DDRB = ((DDRB & 0b11110000) | 0b00001111);  //set pins d8-d11 to output
  DDRD = (DDRD & 0b00011111) | 0b00000000;    //set pins d5-d7 to intput
  PORTD = (PORTD & 0b00011111) | 0b11100000;  //set pins d5-7 to input pullup

  cli();                                        //clear interrupts
  TCCR2A = (TCCR2A & 0b00111100) | 0b00000010;  // set timer to compare timer on match (CTC) mode
  TCCR2B = (TCCR2B & 011111000) | 0b00000010;   // set timer presccaler
  OCR2A = 18;                                   // set compare register clock value
  TIMSK2 = 0b00000010;                          // enable timer 2 A for CTC mode
  sei();                                        // start interrupts
}

/******************************************************************************/
//  LFO WAVE SHAPE TABLE
uint8_t lfoShapeTable[8][16]{
  //  step          0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f
  /*  shape 0 */ { 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 },  // square    speed*1
  /*  shape 1 */ { 0xf, 0xd, 0xb, 0x9, 0x7, 0x5, 0x3, 0x1, 0x0, 0x1, 0x3, 0x5, 0x7, 0x9, 0xb, 0xd },  // triangle  speed*1
  /*  shape 2 */ { 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0 },  // saw       speed*1
  /*  shape 3 */ { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf },  // ramp      speed*1
  /*  shape 4 */ { 0xf, 0xf, 0xf, 0xf, 0x0, 0x0, 0x0, 0x0, 0xf, 0xf, 0xf, 0xf, 0x0, 0x0, 0x0, 0x0 },  // square    speed*2
  /*  shape 5 */ { 0xf, 0x8, 0x4, 0x2, 0x0, 0x2, 0x4, 0x8, 0xf, 0x8, 0x4, 0x2, 0x0, 0x2, 0x4, 0x8 },  // triangle  speed*2
  /*  shape 6 */ { 0xf, 0xc, 0xa, 0x8, 0x6, 0x3, 0x2, 0x0, 0xf, 0xc, 0xa, 0x8, 0x6, 0x3, 0x2, 0x0 },  // saw       speed*2
  /*  shape 7 */ { 0x0, 0x2, 0x4, 0x6, 0x8, 0xa, 0xc, 0xf, 0x0, 0x2, 0x4, 0x6, 0x8, 0xa, 0xc, 0xf }   // ramp      speed*2
};

/******************************************************************************/

//  variables used by the main loop

uint8_t lfoSpeedInput;   // 8 bit interger for lfo speed
uint8_t lfoShapeInput;   // 8 bit interger for lfo shape
uint8_t fmInput;         // 8 bit interger for modulation frequency
uint8_t frequencyInput;  // 8 bit interger for carrier frequency
uint8_t vibratoInput;    // 8 bit interger for vibrato depth
uint8_t tremeloInput;    // 8 bit interger for tremalo depth
int8_t inverterInput;    // 8 bit signe interger for tremelo inversion
uint8_t lfoStep = 0;     // 8 bit interger for tracking lfo step
uint8_t lfoSlice = 0;    // 8 bit interger for lfo slice pointer
uint32_t lfoTimer = 0;   // 32 bit interger for lfo timing


/******************************************************************************/

//  MAIN LOOP

void loop() {
  lfoSpeedInput = analogReadFast(lfoSpeedPot) >> 4;  //  retrieve lfo speed input

  if (lfoTimer >= lfoSpeedInput) {                                           // check lfo timer
    lfoShapeInput = analogReadFast(lfoShapePot) >> 7;                        //  retrieve shape from shape input
    lfoSlice = lfoShapeTable[lfoShapeInput][lfoStep & 0b0001111];            //  retrieve shape step slice value from shape table
    tremeloInput = 16 - (analogReadFast(tremeloPot) >> 6);                   //  retrieve tremelo depth input (0-16)
    vibratoInput = map(analogReadFast(vibratoPot), 0, 1023, 1, 15);          //  retrieve vibrato depth input (1-15)
    inverterInput = ((PIND >> inverterSwitch) << 1) - 1;                     //  retrieve tremelo invrter input (-1,1)
    frequencyInput = 15 + analogReadFast(notePot) / 17;                      //  retrieve carrier frequency input (15-75)
    frequency = frequencyInput + (lfoSlice / tremeloInput) * inverterInput;  //  set carrier (frequency + tremelo)*inverter
    orXor = 1 & ((PIND >> orXorSwitch));                                     //  retrieve and set or/xor mode bit
    fmInput = analogReadFast(fmPot) / 42;                                    //  retrieve modulation frequency input (0-24)
    fm = frequency + fmInput;                                                //  add carrier to mudulation and set modulation
    sync = 1 & (PIND >> syncSwitch);                                         //  retrieve and set sync mode bit
    gain = 15 - (lfoSlice / vibratoInput);                                   //  set current slice gain level
    lfoStep++;                                                               //  increment lfo step
    lfoTimer = 0;                                                            //  clear lfo timer
 
  } else {
    lfoTimer++;  //  increment lfo timer
  }
}

/******************************************************************************/

// isr variables
uint16_t frequencyTimer = 0;  //  clocks carrier freqency
uint16_t fmTimer = 0;         //  clocks modulation freqency
uint16_t frequencyFlag = 0;   //  compared to frequency clock to check for new frequency
uint16_t fmFlag = 0;          //  compared to frequency clock to check for new frequency
bool outputBit;               //Inverts against oscMask and ANDs against volume to generate frequency oscilation between 0 and volume
bool frequencyBit = 0;        // oscilation mask
bool fmBit = 0;               // oscilation mask

/******************************************************************************/

const uint16_t frequencyTable[108]{
  //        note0  note1  note2  note3  note4  note5  note6  note7  note8  note9  noteA  noteB
  /*oct 0*/ 0x100b, 0x0f25, 0x0e4b, 0x0d7e, 0x0cbc, 0x0c05, 0x0b58, 0x0ab5, 0x0a1b, 0x098a, 0x0901, 0x0880,
  /*oct 1*/ 0x0805, 0x0792, 0x0725, 0x06bf, 0x065e, 0x0602, 0x05ac, 0x055a, 0x050d, 0x04c5, 0x0480, 0x0440,
  /*oct 2*/ 0x0402, 0x03c9, 0x0392, 0x035f, 0x032f, 0x0301, 0x02d6, 0x02ad, 0x0286, 0x0262, 0x0240, 0x0220,
  /*oct 3*/ 0x0201, 0x01e4, 0x01c9, 0x01af, 0x0197, 0x0180, 0x016b, 0x0156, 0x0143, 0x0131, 0x0120, 0x0110,
  /*oct 4*/ 0x0100, 0x00f2, 0x00e4, 0x00d7, 0x00cb, 0x00c0, 0x00b5, 0x00ab, 0x00a1, 0x0098, 0x0090, 0x0088,
  /*oct 5*/ 0x0080, 0x0079, 0x0072, 0x006b, 0x0065, 0x0060, 0x005a, 0x0055, 0x0050, 0x004c, 0x0048, 0x0044,
  /*oct 6*/ 0x0040, 0x003c, 0x0039, 0x0035, 0x0032, 0x0030, 0x002d, 0x002a, 0x0028, 0x0026, 0x0024, 0x0022,
  /*oct 7*/ 0x0020, 0x001e, 0x001c, 0x001a, 0x0019, 0x0018, 0x0016, 0x0015, 0x0014, 0x0013, 0x0012, 0x0011,
  /*oct 8*/ 0x0010, 0x000f, 0x000e, 0x000d, 0x00c, 0x000b, 0x000b, 0x000a, 0x000a, 0x0009, 0x0009, 0x0008
};

/******************************************************************************/

//    TIMER 2 A ISR

ISR(TIMER2_COMPA_vect) {
  frequencyFlag = frequencyTable[frequency];  //  retrieve carrier frequency from frequency table
  fmFlag = frequencyTable[fm];                //  retrieve modulatio frequency from frequency table
  if (frequencyTimer >= frequencyFlag) {      //  check carrier frequency timer
    frequencyBit ^= 1;                        //  oscillate carrier bit
    frequencyTimer = 0;                       //  reset carrier timer
    if (sync) {                               //  check if sync mode
      fmBit = frequencyBit;                   //  sync mudulation to carrier bit
      fmTimer = 0;                            //  reset modulation timer
    }
  }
  if (fmTimer >= fmFlag) {  //  check modulation frequency timer
    fmBit ^= 1;             //  oscillate modulation bit
    fmTimer = 0;            //  reset modulation timer
  }
  outputBit = (frequencyBit | fmBit) && orXor;                      // or carrier and modulation bit if orxor
  outputBit |= (frequencyBit ^ fmBit) && !orXor;                    // xor carrier and modulation bit if not orxor
  PORTB = (PORTB & 0b11100000) | ((outputBit * 0b0001111) & gain);  //  modulate frequency output with gain amplitude level and push to port b

  frequencyTimer++;  //  increment carrier timer
  fmTimer++;         //  increment modulation timer
}

/****************************************************************************************************/

/* C++ note table generator

//equal tempered note scale
#include <iostream>
#include <math.h>
#include <iomanip>
using namespace std;

int main() {
int _base=2;
int _frequencys=120;
int _interval=12;
int _root=17;
int _frequency_table[_frequencys];

for (float x=0;x<_frequencys;x++){
    _frequency_table[int(x)]=_root*pow(_base,x/_interval);
}
for (int x=119;x>=0;x--){
    cout<<"0x"<<setfill('0')<<setw(4)<<hex<<_frequency_table[x]<<",";
    if(x%12==0){
        cout<<endl;
    }
}
    return 0;
}
****************************************************************/