#ifndef __audio_output_pwm_h__
#define __audio_output_pwm_h__

#include <AudioOutput.h>
#include "pwm_audio.h"

class AudioOutputPWM : public AudioOutput {
protected:
  bool _inited;
  int8_t _gpio_left, _gpio_right;
  int16_t _samples[2], _sample_used;
public:
  AudioOutputPWM(const int8_t gpio_left, const int8_t gpio_right=-1){
    hertz = 22050;
    bps = 16;
    channels = 1;
    _sample_used = 0;
    if(gpio_right != -1)
      channels = 2;
    _gpio_left = gpio_left;
    _gpio_right = gpio_right;
    _inited = false;
  }
  ~AudioOutputPWM(){};
  virtual bool begin() {
    if(!_inited){
      _inited = true;
      pwm_audio_config_t pac;
      pac.duty_resolution = LEDC_TIMER_10_BIT;
      pac.gpio_num_left = _gpio_left;
      pac.ledc_channel_left = LEDC_CHANNEL_0;
      pac.gpio_num_right = _gpio_right;
      pac.ledc_channel_right = LEDC_CHANNEL_1;
      pac.ledc_timer_sel = LEDC_TIMER_0;
      pac.tg_num = TIMER_GROUP_0;
      pac.timer_num = TIMER_0;
      pac.ringbuf_len = 8192;
      pwm_audio_init(&pac);
      // Serial.printf("sample rate : %d, bps: %d, channels: %d\n", hertz, bps, channels);
    }
    pwm_audio_set_param(hertz, LEDC_TIMER_16_BIT, _gpio_right != -1 ? 2 : 1);
    pwm_audio_start();
    return true;
  }
  virtual bool ConsumeSample(int16_t sample[2]) {
    size_t cnt = 0;
    bool  ok = false;
    if(channels == 2){
      pwm_audio_write((uint8_t*)sample, 4, &cnt, 0);
      ok = cnt == 4 ? true : false;
    }else{
      if(_sample_used >= 2){
        pwm_audio_write((uint8_t*)_samples, 4, &cnt, 0);
        if(cnt == 4){
          _sample_used = 0;
        }
      }
      if(_sample_used < 2){
        _samples[_sample_used++] = sample[0];
        ok = true;
      }
    }
    return ok;
  }
  virtual bool stop() {
    pwm_audio_stop();
    if(_inited){
      pwm_audio_deinit();
      _inited = false;
    }
    return true;
  }
};


#endif // __audio_output_pwm_h__
