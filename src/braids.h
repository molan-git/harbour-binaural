#pragma once

#include "braids/envelope.h"
#include "braids/macro_oscillator.h"
#include "braids/quantizer.h"
#include "braids/signature_waveshaper.h"
#include "braids/quantizer_scales.h"
#include "braids/vco_jitter_source.h"

// braids dsp

//const uint16_t decimation_factors[] = { 1, 2, 3, 4, 6, 12, 24 };
const uint16_t bit_reduction_masks[] = {
  0xffff,
  0xfff0,
  0xff00,
  0xf800,
  0xf000,
  0xe000,
  0xc000
};

#define     MI_SAMPLERATE      96000.f
#define     BLOCK_SIZE          4096 // must be <= braids::MacroOscillator::kMaxBlockSize (scratch buffers in macro_oscillator.h)
#define     SAMP_SCALE          (float)(1.0 / 32756.0)

typedef struct
{
  braids::MacroOscillator *osc;

  float       samps[BLOCK_SIZE] ;
  int16_t     buffer[BLOCK_SIZE];
  uint8_t     sync_buffer[BLOCK_SIZE];

} PROCESS_CB_DATA ;

char shared_buffer[16384];

//float a0 = (440.0 / 8.0) / kSampleRate; //48000.00;
//const size_t   kBlockSize = BLOCK_SIZE;


struct Braid {
  braids::Quantizer   *quantizer;
  braids::SignatureWaveshaper *ws;
  //braids::Envelope *envelope;

  bool            last_trig;
  // resampler
  //SRC_STATE       *src_state;

  PROCESS_CB_DATA pd;
  float           *samples;
  float           ratio;
};

static long src_input_callback(void *cb_data, float **audio);

struct Braid inst[1];

int8_t renderCount = 31;

int16_t pitch_fm;
int16_t pitch_adj = 100;


void updateBraidsControl() {

  //pitch = voct_midiBraids(CV1);


}

int pitch_in, timb_mod, timbre_in, morph_mod, morph_in, engine_in, trigger_in, harm_in;

void updateBraidsAudio(float morph, float timbre, int instrument, int pitch) {

/* for demo purposes */

    float morph_in = morph;
    float trigger_in = 0.0f; //IN(5);
    float timbre_in = timbre;
    float timb_mod = 0.0f;
    float morph_mod = 0.0f;
    trigger_in = 1.0f;
    engine_in = instrument;
    pitch_in = pitch;

    // 40 is wave paraphonic
    // 42 is twin peaks noise
    // 45 is particle noise
    // 32 is struck bell
    // 33 is struck drum
    // 27/28 is FM

  braids::MacroOscillator *osc = inst[0].pd.osc;

  //osc->set_pitch( ( pitch_in - pitch_adj)  + pitch_fm ); // << 7);
  osc->set_pitch( (int)pitch_in << 7);


  float modulation;
  modulation = timb_mod; // timbre
  timbre = (timbre_in + modulation);
  CONSTRAIN(timbre, 0.0f, 1.0f);

  modulation = morph_mod; // color
   morph = (morph_in + modulation);
  CONSTRAIN(morph, 0.0f, 1.0f);

  // this is the scale used in braids
  timbre = (timbre * 32767.0f);
  morph = (morph * 32767.0f);

  osc->set_parameters(timbre, morph);

  // set shape/model
  uint8_t shape = (int)(engine_in);
  if (shape >= braids::MACRO_OSC_SHAPE_LAST)
    shape -= braids::MACRO_OSC_SHAPE_LAST;
  osc->set_shape(static_cast<braids::MacroOscillatorShape>(shape));


  bool trigger = (trigger_in > 0.0f);
  bool trigger_flag = (trigger && (!inst[0].last_trig));

  if (trigger_flag) {
    osc->Strike();
  }

  inst[0].last_trig = trigger;

  // we render and read buffers in the generator
  // osc->Render(sync_buffer, buffer, size);
  // copy to output buffer

//  for (int count = 0; count < 32; count++) {
//    int16_t sample =   (int16_t)  inst[0].pd.buffer[count]  ;
//    out_bufferL[count] = sample;
//  }

}


// initialize macro osc
void initBraids() {

  inst[0].ratio = 48000.f / MI_SAMPLERATE;

  // init some params
  inst[0].pd.osc = new braids::MacroOscillator;
  memset(inst[0].pd.osc, 0, sizeof(*inst[0].pd.osc));

  inst[0].pd.osc->Init(48000.f);
  inst[0].pd.osc->set_pitch((48 << 7));
  //inst[0].pd.osc->set_shape(braids::MACRO_OSC_SHAPE_TWIN_PEAKS_NOISE);
  inst[0].pd.osc->set_shape(braids::MACRO_OSC_SHAPE_VOWEL_FOF);


  inst[0].ws = new braids::SignatureWaveshaper;
  inst[0].ws->Init(123774);

  inst[0].quantizer = new braids::Quantizer;
  inst[0].quantizer->Init();
  inst[0].quantizer->Configure(braids::scales[0]);

  //unit->jitter_source.Init();

  memset(inst[0].pd.buffer, 0, sizeof(int16_t)*BLOCK_SIZE);
  memset(inst[0].pd.sync_buffer, 0, sizeof(inst[0].pd.sync_buffer));
  memset(inst[0].pd.samps, 0, sizeof(float)*BLOCK_SIZE);

  inst[0].last_trig = false;

  //inst[0].envelope = new braids::Envelope;
  //inst[0].envelope->Init();

  // get some samples initially
  //updateBraidsAudio();

  /*
    // Initialize the sample rate converter
    int error;
    int converter = SRC_SINC_FASTEST;       //SRC_SINC_MEDIUM_QUALITY;


         // check resample flag
      int resamp = (int)IN0(5);
      CONSTRAIN(resamp, 0, 2);
      switch(resamp) {
          case 0:
              SETCALC(MiBraids_next);
              //Print("resamp: OFF\n");
              break;
          case 1:
              unit->pd.osc->Init(MI_SAMPLERATE);
              SETCALC(MiBraids_next_resamp);
              Print("MiBraids: internal sr: 96kHz - resamp: ON\n");
              break;
          case 2:
              SETCALC(MiBraids_next_reduc);
              Print("MiBraids: resamp: OFF, reduction: ON\n");
              break;
      }
  */
}

const braids::SettingsData kInitSettings = {
  braids::MACRO_OSC_SHAPE_CSAW,

  braids::RESOLUTION_16_BIT,
  braids::SAMPLE_RATE_96K,

  0,  // AD->timbre
  true,  // Trig source auto trigger
  1,  // Trig delay
  false,  // Meta modulation

  braids::PITCH_RANGE_440,
  2,
  0,  // Quantizer is off
  false,
  false,
  false,

  2,  // Brightness
  0,  // AD attack
  5,  // AD decay
  0,  // AD->FM
  0,  // AD->COLOR
  0,  // AD->VCA
  0,  // Quantizer root

  50,
  15401,
  2048,

  { 0, 0 },
  { 32768, 32768 },
  "GREETINGS FROM MUTABLE INSTRUMENTS *EDIT ME*",
};
