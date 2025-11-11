#pragma once
#include <Lpf2Hub.h>
#include <Lpf2HubConst.h>

class DuploTrainHub : public Lpf2Hub {
public:
  void activateBaseSpeaker();
  void playSound(DuploTrainBaseSound sound);
  void activateRgbLight();
  void setLedColor(Color color);
};