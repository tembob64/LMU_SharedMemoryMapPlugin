/*
Author: The Iron Wolf (vleonavicius@hotmail.com)
Website: thecrewchief.org
*/

#pragma once

class DirectMemoryReader
{
public:
  DirectMemoryReader() {}

  bool Initialize();
  bool Read(LMU_Extended& extended, int mID);
  bool ReadOnNewSession(LMU_Extended& extended);
  //bool ReadOnLSIVisible(LMU_Extended& extended);

  bool IsSCRPluginEnabled() const { return mSCRPluginEnabled; }
  long GetSCRPluginDoubleFileType() const { return mSCRPluginDoubleFileType; }
  //void ClearLSIValues(LMU_Extended& extended);
  void DirectMemoryReader::ResetFounds();

private:
  void ReadSCRPluginConfig();
  void ReadSCRPluginConfigValues(char* const pluginConfig);
  

private:
  
  const unsigned long long  mCutsPointsOffset = 0x4055E50uLL; // mCutsMaxOffset ofsett 0x1557718uLL  
  const unsigned long long  mPenaltyCountOffset = 0x4069F7CuLL; // Byte mPenaltyCountOffset ofsett 0x156B468uLL           //step 0x11608uLL  79A38   40321E0
  const unsigned long long  mPenaltyLeftLapsOffset =  0x4069F84uLL; // Byte mPenaltyLeftLapsOffset ofsett 0x156B470uLL
  const unsigned long long  mPenaltyTypeCurrentOffset = 0x4069F88uLL; //Byte mPenaltyTypeCurrent 0x156B474uLL
  const unsigned long long  mPenaltyType1Offset = 0x4069F8CuLL; //Byte mPenaltyType1 0x  156B478uLL
  const unsigned long long  mCutsOffset = 0x406A188uLL; //  2 mCutsOffset 0x156B678uLL
  
  const unsigned long long mFuelAndEnergyOffset = 0x1A20348uLL; //0x1A1A6A0uLL; // mChangedParamValueOffset + 0xFE88
  const unsigned long long mFuelLastLapOffset = 0x1A26A94uLL; //0x1A20DECuLL;   // mFuelAndEnergyOffset + 0x64FC

  const unsigned long long mChangedParamTypeOffset = 0x1A10408uLL;  // = mChangedParamValueOffset -0x10
  const unsigned long long mChangedParamValueOffset = 0x1A10418uLL; //0x1A0A880uLL;  //0x1A06890uLL; // offset 0x00 
  const unsigned long long mChangeParamSlotStep = 0x1F170uLL; //0x11DB8uLL;  1CCDD38 2C7568
  const unsigned long long mChangeParamSlotStepInt = 0x7C5CuLL;
  const unsigned long long mChangeParamSlotStepDouble = 0x3E2EuLL;
  const unsigned long long mFront_ABR_Offset = 0x1A158F4uLL; //0x1A148F4uLL; //0x1A0FC4CuLL; //mChangedParamValueOffset + 0x5434
  const unsigned long long mRear_ABR_Offset = 0x1A15984uLL; //0x1A0FCDCuLL; // mFront_ABR_Offset + 0x90 
  
  
 
  int* mCutsPoints = nullptr;
 
  int* mPenaltyCount = nullptr;
  bool mPenaltyCountFound = false;
  bool SeesionLive = false;
  
  unsigned long long mPenaltyCountFoundOffset = 0uLL;
 /* int* mPenaltyLeftLaps = nullptr;
  int* mPenaltyTypeCurrent = nullptr;
  int* mPenaltyType1 = nullptr;*/

  char* mMotorMap = nullptr;
  int* mChangedParamType = nullptr;
  int* mFront_ABR = nullptr;
  int* mRear_ABR = nullptr;
  char* mChangedParamValue = nullptr;
  double* mFuelAndEnergy = nullptr;
  float* mFuelLastLap = nullptr;
  char* mRegenLevel = nullptr;
  float* mCuts = nullptr;
  bool mCutsFound = false;
  unsigned long long mCutsFoundOffset = 0uLL;
  char* mpTC = nullptr;
  char* mpBR = nullptr;
  char* mpStatusMessage = nullptr;
  char** mppMessageCenterMessages = nullptr;
  float* mpCurrPitSpeedLimit = nullptr;
  char* mpLSIMessages = nullptr;

  bool mSCRPluginEnabled = false;
  long mSCRPluginDoubleFileType = -1L;
};