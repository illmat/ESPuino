#pragma once

#include <Lpf2HubConst.h> //legoino

void DuploTrain_Init(void);
void DuploTrain_Cyclic(void);
void DuploTrain_ToggleLight(void);
void DuploTrain_SetSpeed(const int32_t speedIncrease);
void DuploTrain_Brake(void);
void DuploTrain_PlaySound(DuploTrainBaseSound sound);
bool DuploTrain_Connected();
int32_t DuploTrain_GetCurrentSpeed();
int8_t  DuploTrain_GetStepIndex();
