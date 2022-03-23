#include <Arduino.h>
#define CONFIG_LEFT_CHANNEL_GPIO 25
#include "AudioOutputPWM.h"
#include "AudioFileSourcePROGMEM.h"
#include "AudioGeneratorWAV.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputULP.h"
#include "report.h"
#include "ring3mp3.h"
#include "AudioOutputMixer.h"

AudioGeneratorWAV *wav;
AudioGeneratorMP3 *mp3;
AudioFileSourcePROGMEM *file[2];
AudioOutputPWM *out;
// AudioOutputULP *out;

AudioOutputMixer *mixer;
AudioOutputMixerStub *stub[2];


void setup()
{
  Serial.begin(115200);
  delay(1000);

  audioLogger = &Serial;
  // out = new AudioOutputULP(1);
  out = new AudioOutputPWM(CONFIG_LEFT_CHANNEL_GPIO);
  out->SetRate(22050);
  file[0] = new AudioFileSourcePROGMEM(ring3_mp3, ring3_mp3_len);
  mp3 = new AudioGeneratorMP3();
  mixer = new AudioOutputMixer(128, out);
  stub[0] = mixer->NewInput();
  Serial.printf("mp3 start: %lu\n", millis());
  mp3->begin(file[0], stub[0]);
}

void loop()
{
  static uint32_t start = 0;
  static bool wav_go = false;

  if (mp3->isRunning()) {
    if (!mp3->loop()){
      mp3->stop();
      stub[0]->stop();
      Serial.printf("mp3 done: %lu\n", millis());
    }
  }

  if (millis() - start > 3000) {
    if (!wav_go) {
      Serial.printf("wav begin: %lu\n", millis());
      stub[1] = mixer->NewInput();
      wav = new AudioGeneratorWAV();
      file[1] = new AudioFileSourcePROGMEM( report_wav, sizeof(report_wav) );
      wav->begin(file[1], stub[1]);
      wav_go = true;
    }
    if (wav->isRunning()) {
      if (!wav->loop()) {
         wav->stop(); 
         stub[1]->stop(); 
         Serial.printf("wav done: %lu\n", millis());
      }
    }
  }
}
