#include "LMU_SharedMemoryMap.hpp"
#include <stdlib.h>
#include <cstddef>                              // offsetof
#include "DirectMemoryReader.h"
#include "Utils.h"
#include <string>

int TimerCount = 0;

bool DirectMemoryReader::Initialize()
{
  __try {
    DEBUG_MSG(DebugLevel::DevInfo, DebugSource::DMR, "Initializing DMR.");

    //auto const startTicks = TicksNow();


      auto const module = ::GetModuleHandle(nullptr);

      //mPenaltyCount = reinterpret_cast<int*>(Utils::GetValueFromMemory(module, mPenaltyCountOffset ));
      //if (mPenaltyCount == nullptr) {
      //  DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "mPenaltyCount cannot created");
      //  return false;
      //}
      //else
      //{
      //  DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mPenaltyCount  created at 0x%p", mPenaltyCount);
      //}

      //mCuts = reinterpret_cast<float*>(Utils::GetValueFromMemory(module, mCutsOffset));
      //if (mCuts == nullptr) {
      //  DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "Not Resolve cuts status message");
      //  return false;
      //}
      //else
      //{
      //  //auto mPenaltyType= mPenaltyLeftLaps - 4uLL;
      //  DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "Created Float Adress 0x%p", mCuts);
      //}

      mMotorMap = reinterpret_cast<char*>(Utils::GetValueFromMemory(module, mMotorMapOffset));
      if (mMotorMap == nullptr) {
        DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "Not Resolve mMotorMap status message");
        return false;
      }
      else
      {
        //auto mPenaltyType= mPenaltyLeftLaps - 4uLL;
        DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "Created Float Adress A2 0x%p", mMotorMap);
      }

      mFront_ABR = reinterpret_cast<int*>(Utils::GetValueFromMemory(module, mFront_ABR_Offset));
      if (mFront_ABR == nullptr) {
          DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "Not Resolve mChangedParamType status message");
          return false;
      }
      else
      {
          //auto mPenaltyType= mPenaltyLeftLaps - 4uLL;
          DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "Created mChangedParamType Adress A2 0x%p", mChangedParamType);
      }

      mRear_ABR = reinterpret_cast<int*>(Utils::GetValueFromMemory(module, mRear_ABR_Offset));
      if (mRear_ABR == nullptr) {
          DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "Not Resolve mChangedParamType status message");
          return false;
      }
      else
      {
          //auto mPenaltyType= mPenaltyLeftLaps - 4uLL;
          DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "Created mChangedParamType Adress A2 0x%p", mChangedParamType);
      }

      mChangedParamType = reinterpret_cast<int*>(Utils::GetValueFromMemory(module, mChangedParamTypeOffset));
      if (mChangedParamType == nullptr) {
        DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "Not Resolve mChangedParamType status message");
        return false;
      }
      else
      {
        //auto mPenaltyType= mPenaltyLeftLaps - 4uLL;
        DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "Created mChangedParamType Adress A2 0x%p", mChangedParamType);
      }

      mChangedParamValue = reinterpret_cast<char*>(Utils::GetValueFromMemory(module, mChangedParamValueOffset));
      if (mChangedParamValue == nullptr) {
        DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "Not Resolve mChangedParamValue status message");
        return false;
      }

      mFuelAndEnergy = reinterpret_cast<double*>(Utils::GetValueFromMemory(module, mFuelAndEnergyOffset));
      if (mFuelAndEnergy == nullptr) {
          DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "Not Resolve mFuelAndEnergy status message");
          return false;
      }
      else
      {
        //auto mPenaltyType= mPenaltyLeftLaps - 4uLL;
        DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "Created mChangedParamValue Adress A2 0x%p", mChangedParamValue);
      }
      mFuelLastLap = reinterpret_cast<float*>(Utils::GetValueFromMemory(module, mFuelLastLapOffset));
      if (mFuelLastLap == nullptr) {
          DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "Not Resolve mFuelAndEnergy status message");
          return false;
      }
      else
      {
          //auto mPenaltyType= mPenaltyLeftLaps - 4uLL;
          DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "Created mChangedParamValue Adress A2 0x%p", mChangedParamValue);
      }
      

   //auto const endTicks = TicksNow();

  
  }
  __except (::GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
    DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "Exception while reading memory, disabling DMA1.");
    return false;
  }

  return true;
}

void DirectMemoryReader::ResetFounds()
{
  mPenaltyCountFound = false;
  SeesionLive = false;
  mCutsFound = false;
  mCutsFoundOffset = 0uLL;
  mPenaltyCountFoundOffset = 0uLL;
}

bool DirectMemoryReader::Read(LMU_Extended& extended, int mID)
{
  __try {



    //if (mPenaltyCount != nullptr)
    //{

    //TimerCount--;
    ///*  if (TimerCount <= 0 && SeesionLive)
    //  {
    //    DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "Penalties Start - mID: %d mPlace %d", mID, (int)mPlace);
    //    for (int ind = 0; ind < 105; ind++) {
    //      if ((int)*(mPenaltyCount + 0x4582uLL * ind) > 0 || (float)*(mCuts + 0x4582uLL * ind) > 0) {
    //        DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "Index %d mPenaltyCount %d mCuts %.2f Addr P: 0x%p Addr C: 0x%p", ind, (int)*(mPenaltyCount + 0x4582uLL * ind), (float)*(mCuts + 0x4582uLL * ind), (mPenaltyCount + 0x4582uLL * ind), (mCuts + 0x4582uLL * ind));
    //      }
    //    }
    //    DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "Penalties End");
    //    TimerCount = 50;
    //  }*/
    //    

    //  if (mID != -1)
    //  {
    //    auto mPenaltyCountCurrent = (mPenaltyCount + (unsigned long long)(0x4582uLL * mID));
    //    extended.mPenaltyCount = (int)*mPenaltyCountCurrent;
    //    auto mPenaltyLeftLaps = mPenaltyCountCurrent + 2uLL;
    //    extended.mPenaltyLeftLaps = (int)*mPenaltyLeftLaps;
    //    //auto mPenaltyType2 = mPenaltyLeftLaps + 20uLL;
    //    auto mPenaltyTypeCurrent = mPenaltyCountCurrent + 3uLL;
    //    auto mPendingPenaltyType1 = mPenaltyCountCurrent + 4uLL;
    //    auto mPendingPenaltyType2 = mPenaltyCountCurrent + 7uLL;
    //    auto mPendingPenaltyType3 = mPenaltyCountCurrent + 10uLL;

    //    if (TimerCount < 0)

    //    {
    //      DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mSlot %d mPlace %d", mID, mPlace);
    //      DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mPenaltyCount %d at 0x%p", (int)*(mPenaltyCountCurrent), mPenaltyCountCurrent);
    //      DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mPenaltyLeftLaps %d at 0x%p", *mPenaltyLeftLaps, mPenaltyLeftLaps);
    //      DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mPenaltyTypeCurrent %d at 0x%p", *mPenaltyTypeCurrent, mPenaltyTypeCurrent);
    //      DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mPenaltyType1 %d at 0x%p", *mPendingPenaltyType1, mPendingPenaltyType1);
    //      DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mPenaltyType2 %d at 0x%p", *mPendingPenaltyType2, mPendingPenaltyType2);
    //      DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mPenaltyType3 %d at 0x%p", *mPendingPenaltyType3, mPendingPenaltyType3);
    //    }
    //    
    //    extended.mPenaltyType = 0;
    //    extended.mPendingPenaltyType1 = 0;
    //    extended.mPendingPenaltyType2 = 0;
    //    extended.mPendingPenaltyType3 = 0;

    //    if (extended.mPenaltyCount > 0)
    //    {
    //      extended.mPenaltyType = (int)*mPenaltyCountCurrent;
    //      if (extended.mPenaltyCount > 1) extended.mPendingPenaltyType1 = (int)*mPendingPenaltyType1;
    //      if (extended.mPenaltyCount > 2) extended.mPendingPenaltyType2 = (int)*mPendingPenaltyType2;
    //      if (extended.mPenaltyCount > 3 )extended.mPendingPenaltyType3 = (int)*mPendingPenaltyType3;
    //    }
    //    //if (*mPenaltyTypeCurrent == 1 && *mPenaltyCount == 1)  extended.mPenaltyType = 1; //DT Only
    //    //if (*mPenaltyTypeCurrent == 0 && *mPenaltyCount == 1)  extended.mPenaltyType = 2; // SG Only
    //    //if (*mPenaltyTypeCurrent == 0 && *mPendingPenaltyType1 == 1 && *mPenaltyCount >= 2)  extended.mPenaltyType = 3; // SGAndDTPendings
    //    //if (*mPenaltyTypeCurrent == 1 && *mPendingPenaltyType1 == 1 && *mPenaltyCount >= 2)  extended.mPenaltyType = 4; // DTAndDTPendings
    //    //if (*mPenaltyTypeCurrent == 1 && *mPendingPenaltyType1 == 10 && *mPenaltyCount >= 2)  extended.mPenaltyType = 5; // DTAndSGPendings
    //    //if (*mPenaltyTypeCurrent == 0 && *mPendingPenaltyType1 == 10 && *mPenaltyCount >= 2)  extended.mPenaltyType = 6; // SGAndSGPendings
    //    //if (extended.mPenaltyCount > 0 && extended.mPenaltyLeftLaps == 0) extended.mPenaltyType = 7; // DQ
    //  }
    //  //else
    //  //{
    //  //auto mPenaltyCountCurrent = mPenaltyCount;
    //  //for (int ind = 1; ind <= 105; ind++) {
    //  //  if ((int)*mPenaltyCountCurrent > 0 && (int)*(mPenaltyCountCurrent - 8uLL) == (int)mPlace)
    //  //  {
    //  //    mPenaltyCountFoundOffset = mPenaltyCountCurrent - mPenaltyCount;
    //  //    mCutsFoundOffset = mPenaltyCountFoundOffset;
    //  //    mPenaltyCountFound = true;
    //  //    mCutsFound = true;
    //  //    DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "Found mPenaltyCount = %d at addr: 0x%p ind: %d mSlot %d", *mPenaltyCountCurrent, mPenaltyCountCurrent, ind, mID);
    //  //    break;
    //  //  }
    //  //  else
    //  //  {
    //  //    mPenaltyCountCurrent += 0x4582uLL;
    //  //  }
    //  //}
    //  //}
    //}


    //  if (mCuts != nullptr)
    //  {
    //      if(mID != -1)
    //      { 
    //        auto mCutsCurrent = (mCuts + (unsigned long long)(0x4582uLL * mID));
    //        extended.mCuts = (float)*mCutsCurrent;
    //        if (TimerCount < 0)

    //        {
    //          DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mCuts %.2f  at addr: 0x%p \r\n", extended.mCuts, mCuts);
    //          TimerCount = 50;
    //        }
    //      }
    //  }
if (mID != -1)
{

 /*     if (mMotorMap != nullptr)
      {
        char motorMap[16] = {0};
        for (int i = 0; i < 16; i++)
        {
          unsigned long long Online = 0x0uLL;
          auto mMapChar = mMotorMap + ((mID > 0) ? (Online + (mChangeParamSlotStep * (unsigned long long)mID)) : 0) + (unsigned long long)i;
          
          if (*mMapChar != 0)
          {
               motorMap[i] = *mMapChar;
          }
          else
          {
               motorMap[i] = 0;
               break;
          }
        }
        strcpy_s(extended.mpMotorMap, motorMap);
        extended.mpMotorMap[15] = 0;
       
        DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mpMotorMap %s  at addr: 0x%p \r\n", motorMap, mMotorMap);
      }*/




        if (mChangedParamType != nullptr)
        {
          auto mChangedParamTypeCurrent = (mChangedParamType + (mChangeParamSlotStepInt * (unsigned long long)mID));
         extended.mChangedParamType = (int)*mChangedParamTypeCurrent;
          DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mChangedParamTypeCurrent %d  at addr: 0x%p slot %d \r\n", (int)*mChangedParamTypeCurrent, mChangedParamTypeCurrent, mID);
        }

        if (mFront_ABR != nullptr)
        {
            auto mFront_ABRCurrent = (mFront_ABR + (mChangeParamSlotStepInt * (unsigned long long)mID));
            extended.mFront_ABR = (int)*mFront_ABRCurrent;
            DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mChangedParamTypeCurrent %d  at addr: 0x%p slot %d \r\n", (int)*mFront_ABRCurrent, mFront_ABRCurrent, mID);
        }

        if (mRear_ABR != nullptr)
        {
            auto mRear_ABRCurrent = (mRear_ABR + (mChangeParamSlotStepInt * (unsigned long long)mID));
            extended.mRear_ABR = (int)*mRear_ABRCurrent;
            DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mChangedParamTypeCurrent %d  at addr: 0x%p slot %d \r\n", (int)*mRear_ABRCurrent, mRear_ABRCurrent, mID);
        }

        if (mFuelLastLap != nullptr)
        {
            auto mFuelLastLapValue = (mFuelLastLap + (unsigned long long)(mChangeParamSlotStepInt * mID));
            auto mEnergyLastLapValue = (mFuelLastLap + 4 + (unsigned long long)(mChangeParamSlotStepInt * mID));
            extended.mFuelLastLap = (float)*mFuelLastLapValue;
            extended.mEnergyLastLap = (float)*mEnergyLastLapValue;
            DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mFuelLastLapValue %f  at addr: 0x%p \r\n", *mFuelLastLapValue, mFuelLastLapValue);
            DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mEnergyLastLapValue %f  at addr: 0x%p \r\n", *mEnergyLastLapValue, mEnergyLastLapValue);
        }

        if (mFuelAndEnergy != nullptr)
        {
            auto mCurrentBatteryValue = (mFuelAndEnergy + (unsigned long long)(mChangeParamSlotStepDouble * mID));
            auto mMaxBatteryValue = (mFuelAndEnergy + 2 + (unsigned long long)(mChangeParamSlotStepDouble * mID));

            auto mCurrentEnergyValue = (mFuelAndEnergy + 6 + (unsigned long long)(mChangeParamSlotStepDouble * mID));
            auto mMaxEnergyValue = (mFuelAndEnergy + 8 + (unsigned long long)(mChangeParamSlotStepDouble * mID));

            auto mCurrentFuelValue = (mFuelAndEnergy - 22 + (unsigned long long)(mChangeParamSlotStepDouble * mID));
            auto mMaxFuelValue = (mFuelAndEnergy - 20 + (unsigned long long)(mChangeParamSlotStepDouble * mID));

            extended.mCurrentBatteryValue = (double)*mCurrentBatteryValue;
            extended.mMaxBatteryValue = (double)*mMaxBatteryValue;
            extended.mCurrentEnergyValue = (double)*mCurrentEnergyValue;
            extended.mMaxEnergyValue = (double)*mMaxEnergyValue;
            extended.mCurrentFuelValue = (double)*mCurrentFuelValue;
            extended.mMaxFuelValue = (double)*mMaxFuelValue;
           

            DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "CurrentBatteryValue %lf  at addr: 0x%p \r\n", *mCurrentBatteryValue, mCurrentBatteryValue);
            DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mMaxBatteryValue %lf  at addr: 0x%p \r\n", *mMaxBatteryValue, mMaxBatteryValue);
            DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mCurrentEnergyValue %lf  at addr: 0x%p \r\n", *mCurrentEnergyValue, mCurrentEnergyValue);
            DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mMaxEnergyValue %lf  at addr: 0x%p \r\n", *mMaxEnergyValue, mMaxEnergyValue);
            DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mCurrentFuelValue %lf  at addr: 0x%p \r\n", *mCurrentFuelValue, mCurrentFuelValue);
            DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "mMaxFuelValue %lf  at addr: 0x%p \r\n", *mMaxFuelValue, mMaxFuelValue);

        }

        if (mChangedParamValue != nullptr)
        {
          auto mChangedParamValueCurrent = (mChangedParamValue + (unsigned long long)(mChangeParamSlotStep * mID));
          char changedParamValue[16];
          for (int i = 0; i < 16; i++)
          {

            auto mChangedParamValueChar = mChangedParamValueCurrent + (unsigned long long)i;

            if (*mChangedParamValueChar != 0)
            {
              changedParamValue[i] = *mChangedParamValueChar;
            }
            else
            {
              changedParamValue[i] = 0;
              break;
            }
          }
          strcpy_s(extended.mChangedParamValue, changedParamValue);
          extended.mChangedParamValue[15] = 0;

          //DEBUG_MSG(DebugLevel::DevInfo, DebugSource::General, "RegenLevel %s  at addr: 0x%p \r\n", changedParamValue, mChangedParamValueCurrent);
        }
      }
      
  }
  __except (::GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
    DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "Exception while reading memory, disabling DMA2.");
    return false;
  }


  return true;
}

bool DirectMemoryReader::ReadOnNewSession(LMU_Extended& extended)
{
  __try {
    SeesionLive = true;
  }
  __except (::GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
    DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "Excepction while reading memory, disabling DMA.");
    return false;
  }

  return true;
}

//bool DirectMemoryReader::ReadOnLSIVisible(LMU_Extended& extended)
//{
//  __try {
//    //if (mpStatusMessage == nullptr || mppMessageCenterMessages == nullptr || mpCurrPitSpeedLimit == nullptr || mpLSIMessages == nullptr) {
//    //  assert(false && "DMR not available, should not call.");
//    //  //return false;
//    //}
//
//    //auto const pPhase = mpLSIMessages + 0x50uLL;
//    //if (pPhase[0] != '\0'
//    //  && strncmp(mPrevLSIPhaseMessage, pPhase, rF2Extended::MAX_RULES_INSTRUCTION_MSG_LEN) != 0) {
//    //  strcpy_s(extended.mLSIPhaseMessage, pPhase);
//    //  strcpy_s(mPrevLSIPhaseMessage, extended.mLSIPhaseMessage);
//    //  extended.mTicksLSIPhaseMessageUpdated = ::GetTickCount64();
//
//    //  DEBUG_MSG(DebugLevel::DevInfo, DebugSource::DMR, "LSI Phase message updated: '%s'", extended.mLSIPhaseMessage);
//    //}
//
//    //auto const pPitState = mpLSIMessages + 0xD0uLL;
//    //if (pPitState[0] != '\0'
//    //  && strncmp(mPrevLSIPitStateMessage, pPitState, rF2Extended::MAX_RULES_INSTRUCTION_MSG_LEN) != 0) {
//    //  strcpy_s(extended.mLSIPitStateMessage, pPitState);
//    //  strcpy_s(mPrevLSIPitStateMessage, extended.mLSIPitStateMessage);
//    //  extended.mTicksLSIPitStateMessageUpdated = ::GetTickCount64();
//
//    //  DEBUG_MSG(DebugLevel::DevInfo, DebugSource::DMR, "LSI Pit State message updated: '%s'", extended.mLSIPitStateMessage);
//    //}
//
//    //auto const pOrderInstruction = mpLSIMessages + 0x150uLL;
//    //if (pOrderInstruction[0] != '\0'
//    // && strncmp(mPrevLSIOrderInstructionMessage, pOrderInstruction, rF2Extended::MAX_RULES_INSTRUCTION_MSG_LEN) != 0) {
//    //  strcpy_s(extended.mLSIOrderInstructionMessage, pOrderInstruction);
//    //  strcpy_s(mPrevLSIOrderInstructionMessage, extended.mLSIOrderInstructionMessage);
//    //  extended.mTicksLSIOrderInstructionMessageUpdated = ::GetTickCount64();
//
//    //  DEBUG_MSG(DebugLevel::DevInfo, DebugSource::DMR, "LSI Order Instruction message updated: '%s'", extended.mLSIOrderInstructionMessage);
//    //}
//
//    //auto const pRulesInstruction = mpLSIMessages + 0x1D0uLL;
//    //if (mSCRPluginEnabled
//    //  && pRulesInstruction[0] != '\0'
//    //  && strncmp(mPrevLSIRulesInstructionMessage, pRulesInstruction, rF2Extended::MAX_RULES_INSTRUCTION_MSG_LEN) != 0) {
//    //  strcpy_s(extended.mLSIRulesInstructionMessage, pRulesInstruction);
//    //  strcpy_s(mPrevLSIRulesInstructionMessage, extended.mLSIRulesInstructionMessage);
//    //  extended.mTicksLSIRulesInstructionMessageUpdated = ::GetTickCount64();
//
//    //  DEBUG_MSG(DebugLevel::DevInfo, DebugSource::DMR, "LSI Rules Instruction message updated: '%s'", extended.mLSIRulesInstructionMessage);
//    //}
//  }
//  __except (::GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
//  {
//    DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "Exception while reading memory, disabling DMA.");
//    return false;
//  }
//
//  return true;
//}


void DirectMemoryReader::ReadSCRPluginConfig()
{
  char wd[MAX_PATH] = {};
  ::GetCurrentDirectory(MAX_PATH, wd);

  auto const configFilePath = lstrcatA(wd, R"(\UserData\player\CustomPluginVariables.JSON)");

  auto configFileContents = Utils::GetFileContents(configFilePath);
  if (configFileContents == nullptr) {
    DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "Failed to load CustomPluginVariables.JSON file");
    return;
  }

  auto onExit = Utils::MakeScopeGuard([&]() {
    delete[] configFileContents;
  });

  ReadSCRPluginConfigValues(configFileContents);
}


void DirectMemoryReader::ReadSCRPluginConfigValues(char* const configFileContents)
{
  // See if plugin is enabled:
  auto curLine = strstr(configFileContents, "StockCarRules.dll");
  while (curLine != nullptr) {
    // Cut off next line from the current text.
    auto const nextLine = strstr(curLine, "\r\n");
    if (nextLine != nullptr)
      *nextLine = '\0';

    auto onExitOrNewIteration = Utils::MakeScopeGuard([&]() {
      // Restore the original line.
      if (nextLine != nullptr)
        *nextLine = '\r';
    });

    auto const closingBrace = strchr(curLine, '}');
    if (closingBrace != nullptr) {
      // End of {} for a plugin.
      return;
    }

    if (!mSCRPluginEnabled) {
      // Check if plugin is enabled.
      auto const enabled = strstr(curLine, " \" Enabled\":1");
      if (enabled != nullptr)
        mSCRPluginEnabled = true;
    }

    if (mSCRPluginDoubleFileType == -1L) {
      auto const dft = strstr(curLine, " \"DoubleFileType\":");
      if (dft != nullptr) {
        char value[2] = {};
        value[0] = *(dft + sizeof("\"DoubleFileType\":"));
        mSCRPluginDoubleFileType = atol(value);
      }
    }

    if (mSCRPluginEnabled && mSCRPluginDoubleFileType != -1L)
      return;

    curLine = nextLine != nullptr ? (nextLine + 2 /*skip \r\n*/) : nullptr;
  }

  // If we're here, consider SCR plugin as not enabled.
  mSCRPluginEnabled = false;
  mSCRPluginDoubleFileType = -1L;

  return;
}

//void DirectMemoryReader::ClearLSIValues(LMU_Extended& extended)
//{
//  DEBUG_MSG(DebugLevel::DevInfo, DebugSource::DMR, "Clearing LSI values.");
//
//  //mPrevLSIPhaseMessage[0] = '\0';
//  //extended.mLSIPhaseMessage[0] = '\0';
//  //extended.mTicksLSIPhaseMessageUpdated = ::GetTickCount64();
//
//  //mPrevLSIPitStateMessage[0] = '\0';
//  //extended.mLSIPitStateMessage[0] = '\0';
//  //extended.mTicksLSIPitStateMessageUpdated = ::GetTickCount64();
//
//  //mPrevLSIOrderInstructionMessage[0] = '\0';
//  //extended.mLSIOrderInstructionMessage[0] = '\0';
//  //extended.mTicksLSIOrderInstructionMessageUpdated = ::GetTickCount64();
//
//  //mPrevLSIRulesInstructionMessage[0] = '\0';
//  //extended.mLSIRulesInstructionMessage[0] = '\0';
//  //extended.mTicksLSIRulesInstructionMessageUpdated = ::GetTickCount64();
//}
