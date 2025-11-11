#include <Arduino.h>
#include "settings.h"
#include "Common.h"
#include "Led.h"
#include "Log.h"
#include "RotaryEncoder.h"
#include "System.h"
#include <Lpf2HubConst.h>

#ifdef DUPLO_TRAIN_CONTROL_ENABLE
#include "DuploTrainHub.h"
#include <Lpf2Hub.h>

struct HubState {
  DuploTrainHub hub;
  const byte motorPort = static_cast<byte>(PoweredUpHubPort::A);
};

struct ColorState {
  int lastStable = -1;
  int candidate = -1;
  unsigned long candidateSince = 0;
  const unsigned long stableDelayMs = 500;
};

struct TrainState {
  bool isMoving = false;
  bool lightOn = false;
  int8_t stepIndex = 4;
  int32_t currentSpeed = 0;
  const int8_t steps[5] = {-64, -32, 0, 32, 64};
};

static HubState gHubState;
static ColorState gColorState;
static TrainState gTrainState;
static const unsigned long kConnectDelayMs = 200;

static bool DuploTrain_IsOpMode() {
  return System_GetOperationMode() == OPMODE_DUPLO_TRAIN;
}

static bool DuploTrain_EnsureConnected() {
  if (!gHubState.hub.isConnected()) {
    Log_Println("DuploTrain: not connected", LOGLEVEL_INFO);
    return false;
  }
  return true;
}

static void DuploTrain_ColorCallback(void *hub, byte, DeviceType deviceType, uint8_t *pData) {
  DuploTrainHub *h = static_cast<DuploTrainHub *>(hub);
  if (deviceType != DeviceType::DUPLO_TRAIN_BASE_COLOR_SENSOR) return;
  int color = h->parseColor(pData);
  unsigned long now = millis();
  if (color < 0 || color >= NUM_COLORS) return;
  if (color != gColorState.candidate) {
    gColorState.candidate = color;
    gColorState.candidateSince = now;
    Log_Printf(LOGLEVEL_DEBUG, "DuploTrain: candidate color %s (%d)\n", COLOR_STRING[color], color);
  }
}

static void DuploTrain_HandleConnect() {
  if (!gHubState.hub.isConnecting()) return;
  gHubState.hub.connectHub();
  if (gHubState.hub.isConnected()) {
    Log_Println("DuploTrain: connected", LOGLEVEL_INFO);
    gHubState.hub.activateRgbLight();
    delay(kConnectDelayMs);
    gHubState.hub.activateBaseSpeaker();
    delay(kConnectDelayMs);
    gHubState.hub.activatePortDevice(static_cast<byte>(DuploTrainHubPort::COLOR), DuploTrain_ColorCallback);
    delay(kConnectDelayMs);
  } else {
    Log_Println("DuploTrain: connect failed", LOGLEVEL_INFO);
  }
}

static void DuploTrain_HandleColor(unsigned long now) {
  if (gColorState.candidate < 0) return;
  if (gColorState.candidate == gColorState.lastStable) return;
  if ((now - gColorState.candidateSince) < gColorState.stableDelayMs) return;
  gColorState.lastStable = gColorState.candidate;
  Log_Printf(LOGLEVEL_INFO, "DuploTrain: stable color %s (%d)\n",
             COLOR_STRING[gColorState.lastStable], gColorState.lastStable);
  gHubState.hub.setLedColor(static_cast<Color>(gColorState.lastStable));
}

void DuploTrain_Init() {
  if (!DuploTrain_IsOpMode()) return;
  Log_Println("DuploTrain: init", LOGLEVEL_INFO);
  gHubState.hub.init(30000);
}

void DuploTrain_Cyclic() {
  if (!DuploTrain_IsOpMode()) return;
  DuploTrain_HandleConnect();
  unsigned long now = millis();
  DuploTrain_HandleColor(now);
}

void DuploTrain_Test(int speed) {
  Log_Println("DuploTrain: test", LOGLEVEL_INFO);
  if (!DuploTrain_EnsureConnected()) return;
  if (gTrainState.isMoving) {
    gHubState.hub.setBasicMotorSpeed(gHubState.motorPort, 0);
  } else {
    gHubState.hub.setBasicMotorSpeed(gHubState.motorPort, speed);
  }
  gTrainState.isMoving = !gTrainState.isMoving;
}

void DuploTrain_PlaySound(DuploTrainBaseSound sound) {
  Log_Println("DuploTrain: play sound", LOGLEVEL_INFO);
  if (!DuploTrain_EnsureConnected()) return;
  gHubState.hub.playSound(sound);
}

bool DuploTrain_Connected() {
  return gHubState.hub.isConnected();
}

void DuploTrain_ToggleLight() {
  Log_Println("DuploTrain: toggle light", LOGLEVEL_INFO);
  if (!DuploTrain_EnsureConnected()) return;
  if (gTrainState.lightOn) {
    gHubState.hub.setLedColor(Color::BLACK);
  } else {
    gHubState.hub.setLedColor(Color::WHITE);
  }
  gTrainState.lightOn = !gTrainState.lightOn;
}

void DuploTrain_SetSpeed(const int32_t speedIncrease) {
  Log_Printf(LOGLEVEL_INFO, "DuploTrain: speedIncrease %d\n", speedIncrease);
  if (!DuploTrain_EnsureConnected()) return;
  int8_t delta = speedIncrease / 2;
  if (delta == 0) return;
  int8_t newIndex = gTrainState.stepIndex + delta;
  if (newIndex < 0) newIndex = 0;
  if (newIndex > 4) newIndex = 4;
  int32_t oldSpeed = gTrainState.currentSpeed;
  gTrainState.stepIndex = newIndex;
  gTrainState.currentSpeed = gTrainState.steps[gTrainState.stepIndex];
  if (gTrainState.currentSpeed == oldSpeed) return;
  gHubState.hub.setBasicMotorSpeed(gHubState.motorPort, gTrainState.currentSpeed);
}

void DuploTrain_Brake() {
  Log_Println("DuploTrain: brake", LOGLEVEL_INFO);
  if (!DuploTrain_EnsureConnected()) return;
  gTrainState.currentSpeed = 0;
  gTrainState.stepIndex = 4;
  gHubState.hub.playSound(DuploTrainBaseSound::BRAKE);
  delay(200);
  gHubState.hub.stopBasicMotor(gHubState.motorPort);
}

int32_t DuploTrain_GetCurrentSpeed() {
  return gTrainState.currentSpeed;
}

int8_t DuploTrain_GetStepIndex() {
  return gTrainState.stepIndex;
}

#else

void DuploTrain_Init() {}
void DuploTrain_Cyclic() {}
void DuploTrain_Test(int) {}
void DuploTrain_PlaySound(DuploTrainBaseSound) {}
bool DuploTrain_Connected() { return false; }
void DuploTrain_ToggleLight() {}
void DuploTrain_SetSpeed(const int32_t) {}
void DuploTrain_Brake() {}
int32_t DuploTrain_GetCurrentSpeed() { return 0; }
int8_t DuploTrain_GetStepIndex() { return 4; }

#endif
