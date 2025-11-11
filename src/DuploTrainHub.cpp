// copied from: https://github.com/corneliusmunz/legoino/issues/44#issuecomment-985384328
// TODO: add explanation
// TODO: move to fork
// TODO: fork in github
#include "DuploTrainHub.h"

void DuploTrainHub::activateBaseSpeaker() {
  byte payload[8] = {0x41, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01};
  WriteValue(payload, 8);
}

void DuploTrainHub::playSound(DuploTrainBaseSound sound) {
  byte payload[6] = {0x81, 0x01, 0x11, 0x51, 0x01, static_cast<byte>(sound)};
  WriteValue(payload, 6);
}

void DuploTrainHub::activateRgbLight() {
  byte port = getPortForDeviceType(static_cast<byte>(DeviceType::HUB_LED));
  byte payload[8] = {0x41, port, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
  WriteValue(payload, 8);
}

void DuploTrainHub::setLedColor(Color color) {
  byte port = getPortForDeviceType(static_cast<byte>(DeviceType::HUB_LED));
  byte payload[6] = {0x81, port, 0x11, 0x51, 0x00, static_cast<byte>(color)};
  WriteValue(payload, 6);
}
