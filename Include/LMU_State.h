/*
Definition of mapped rF2 structures and related types.

Author: The Iron Wolf (vleonavicius@hotmail.com)
Website: thecrewchief.org

Description:
  This file contains structures that are written to memory mapped files.  Those
  essentially mirror ISI's Internals Plugin #7 structures defined in InternalsPlugin.hpp,
  except for pointer types, which are replaced with dummy char arrays.  Where game's
  structure contains pointer to the array that we'd like to expose, it is exposed as separate
  member variable with fixed array size.

  Those exposed structures are mostly memcpy'ed one to one from ISI types, so it is critical
  for layout, padding, pack and size to match exactly.

  I've kept comments to reflect relationships to the ISI types.

  Parts of types different from ISI types are tagged with comments:
    - MM_NEW - added members
    - MM_NOT_USED - present in ISI type, but not in mapped type
*/
#pragma once

// Use 4 to match ISI pack.
#pragma pack(push, 4)
#pragma warning(disable : 4121)   // Alignment sensitivity (ISI sets 4 byte pack)

// 0 Before session has begun
// 1 Reconnaissance laps (race only)
// 2 Grid walk-through (race only)
// 3 Formation lap (race only)
// 4 Starting-light countdown has begun (race only)
// 5 Green flag
// 6 Full course yellow / safety car
// 7 Session stopped
// 8 Session over
enum class LMU_GamePhase
{
  Garage = 0,
  WarmUp = 1,
  GridWalk = 2,
  Formation = 3,
  Countdown = 4,
  GreenFlag = 5,
  FullCourseYellow = 6,
  SessionStopped = 7,
  SessionOver = 8
};

enum class LMU_PenaltyTypes
{
  NoPenalty = 0,
  DT = 1,
  DTsGpEnding = 2,
  DTdTpending = 3,
  SG = 4,
  SGdTPending = 5,
  SGsGPending = 6,
};


struct LMU_MappedBufferVersionBlock
{
  unsigned long mVersionUpdateBegin;      // Incremented right before buffer is written to.
  unsigned long mVersionUpdateEnd;        // Incremented after buffer write is done.
};

struct LMU_MappedBufferHeader
{
  static int const MAX_MAPPED_VEHICLES = 128;
};


struct LMU_MappedBufferHeaderWithSize : public LMU_MappedBufferHeader
{
  int mBytesUpdatedHint;              // How many bytes of the structure were written during the last update.
  // 0 means unknown (whole buffer should be considered as updated).
};

struct LMU_MappedInputBufferHeader : public LMU_MappedBufferHeader
{
  long mLayoutVersion;
};

struct LMU_Extended : public LMU_MappedBufferHeader
{
  static int const MAX_MAPPED_IDS = 512;
  static int const MAX_STATUS_MSG_LEN = 128;
  static int const MAX_RULES_INSTRUCTION_MSG_LEN = 96;

  char mVersion[12];                           // API version
  bool is64bit;                                // Is 64bit plugin?

  // Function call based flags:
  bool mInRealtimeFC;                         // in realtime as opposed to at the monitor (reported via last EnterRealtime/ExitRealtime calls).
  //bool mMultimediaThreadStarted;              // multimedia thread started (reported via ThreadStarted/ThreadStopped calls).
  //bool mSimulationThreadStarted;              // simulation thread started (reported via ThreadStarted/ThreadStopped calls).

  bool mSessionStarted;                       // True if Session Started was called.
  ULONGLONG mTicksSessionStarted;             // Ticks when session started.
  ULONGLONG mTicksSessionEnded;               // Ticks when session ended.

 // // FUTURE: It might be worth to keep the whole scoring capture as a separate double buffer instead of this.
 //// rF2SessionTransitionCapture mSessionTransitionCapture;  // Contains partial internals capture at session transition time.

 // // Captured non-empty MessageInfoV01::mText message.
 // char mDisplayedMessageUpdateCapture[sizeof(decltype(MessageInfoV01::mText))];

  // Direct Memory access stuff
  bool mDirectMemoryAccessEnabled;

 // ULONGLONG mTicksStatusMessageUpdated;             // Ticks when status message was updated;
 // char mStatusMessage[LMU_Extended::MAX_STATUS_MSG_LEN];

 // ULONGLONG mTicksLastHistoryMessageUpdated;        // Ticks when last message history message was updated;
 // char mLastHistoryMessage[LMU_Extended::MAX_STATUS_MSG_LEN];

 // float mCurrentPitSpeedLimit;                      // speed limit m/s.

 // bool mSCRPluginEnabled;                           // Is Stock Car Rules plugin enabled?
 // long mSCRPluginDoubleFileType;                    // Stock Car Rules plugin DoubleFileType value, only meaningful if mSCRPluginEnabled is true.

 // ULONGLONG mTicksLSIPhaseMessageUpdated;           // Ticks when last LSI phase message was updated.
 // char mLSIPhaseMessage[LMU_Extended::MAX_RULES_INSTRUCTION_MSG_LEN];

 // ULONGLONG mTicksLSIPitStateMessageUpdated;        // Ticks when last LSI pit state message was updated.
 // char mLSIPitStateMessage[LMU_Extended::MAX_RULES_INSTRUCTION_MSG_LEN];

 // ULONGLONG mTicksLSIOrderInstructionMessageUpdated;     // Ticks when last LSI order instruction message was updated.
 // char mLSIOrderInstructionMessage[LMU_Extended::MAX_RULES_INSTRUCTION_MSG_LEN];

 // ULONGLONG mTicksLSIRulesInstructionMessageUpdated;     // Ticks when last FCY rules message was updated.  Currently, only SCR plugin sets that.
 // char mLSIRulesInstructionMessage[LMU_Extended::MAX_RULES_INSTRUCTION_MSG_LEN];

 long mUnsubscribedBuffersMask;                  // Currently active UnsbscribedBuffersMask value.  This will be allowed for clients to write to in the future, but not yet.

 // bool mHWControlInputEnabled;                    // HWControl input buffer is enabled.
 // bool mWeatherControlInputEnabled;               // Weather Control input buffer is enabled.
 // bool mRulesControlInputEnabled;                 // Rules Control input buffer is enabled.
 // bool mPluginControlInputEnabled;                // Plugin Control input buffer is enabled.

  int mpBrakeMigration;
  int mpBrakeMigrationMax;
  int mpTractionControl;
  char mpMotorMap[16];
  int mChangedParamType;
  char mChangedParamValue[16];
  int mFront_ABR;
  int mRear_ABR;
  int mPenaltyType;
  int mPenaltyCount;
  int mPenaltyLeftLaps;
  int mPendingPenaltyType1;
  int mPendingPenaltyType2;
  int mPendingPenaltyType3;
  float mCuts;
  int mCutsPoints;
  double mCurrentBatteryValue;
  double mMaxBatteryValue;
  double mCurrentEnergyValue;
  double mMaxEnergyValue;
  double mCurrentFuelValue;
  double mMaxFuelValue;
  float mEnergyLastLap;
  float mFuelLastLap;

};




#pragma pack(pop)
