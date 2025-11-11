// copied from: https://github.com/corneliusmunz/legoino/issues/44#issuecomment-985384328
// TODO: add explanation

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