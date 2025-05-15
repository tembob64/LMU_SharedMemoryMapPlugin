/*
Implementation of rFactor 2 internal state mapping into shared memory buffers.

Author: The Iron Wolf (vleonavicius@hotmail.com)
Website: thecrewchief.org


Acknowledgements:
  This work is based on:
    - rF2 Internals Plugin sample #7 by ISI/S397 found at: https://www.studio-397.com/modding-resources/
  Was inspired by:
    - rF1 Shared Memory Map Plugin by Dan Allongo found at: https://github.com/dallongo/rFactorSharedMemoryMap
  With contributions by: Morten Roslev and Tony Whitley


Shared resources:

  Shared resources use the following naming convention:
    - $rFactor2SMMP_<BUFFER_TYPE>$
      or
    - $rFactor2SMMP_<BUFFER_TYPE>$PID where PID is dedicated server process PID.
      or
    - Global\\$rFactor2SMMP_<BUFFER_TYPE>$PID if running in dedicated server and DedicatedServerMapGlobally option is set to 1.

  where <BUFFER_TYPE> is one of the following:
  Output buffers:
    * Telemetry - mapped view of rF2Telemetry structure
    * Scoring - mapped view of rF2Scoring structure
    * Rules - mapped view of rF2Rules structure
    * MultiRules - mapped view of rF2MultiRules structure
    * ForceFeedback - mapped view of rF2ForceFeedback structure
    * PitInfo - mapped view of rF2PitInfo structure
    * Weather - mapped view of rF2Weather structure
    * Extended - mapped view of rF2Extended structure

  Input buffers:
    * HWControl - mapped view of rF2HWControl structure
    * WeatherControl - mapped view of rF2WeatherControl structure
    * RulesControl - mapped view of rF2RulesControl structure
    * PluginControl - mapped view of rF2PluginControl structure

  Aside from Extended (see below), output buffers are (with few exceptions) exact mirror of ISI structures, plugin constantly memcpy's them
  from game to memory mapped files.

  Plugin offers optional weak synchronization by using version variables on each of the output buffers.

  Input buffers are meant to be filled out by the clients.  To avoid complex locking input buffers they use version variables as well, and were
  designed with a single client in mind.  For more high level overview of the input buffers see "Input Buffers" section in the README.md.
 
Output buffer refresh rates:
  Telemetry - updated every 10ms, but in practice only every other update contains updated data, so real update rate is around 50FPS (20ms).
  Scoring - every 200ms (5FPS).
  Rules - every 300ms (3FPS).
  MultiRules - updated only on session change.
  ForceFeedback - approximately every 2.5ms (400FPS).
  Graphics - approximately 400FPS.
  PitInfo - 100FPS.
  Weather - 1FPS.
  Extended - every 200ms (5FPS) or on tracked function call.

  The Plugin does not add artificial delays, except:
    - game calls UpdateTelemetry in bursts every 10ms.  However, as of 02/18 data changes only every 20ms, so one of those bursts is dropped.
    - telemetry updates with same game time are skipped

  The Plugin supports unsubscribing from buffer updates via UnsubscribedBuffersMask CustomPluginVariables.json flag.  Clients can also subscribe
  to the currently unsubscribed buffers via rF2PluginControl input buffer.

Input buffer refresh rates:

  HWControl - Read at 5FPS with 500ms boost to 50FPS once update is received.  Applied at 100FPS.
  WeatherControl - Read at 5FPS.  Applied at 1FPS.
  RulesControl - Read at 5FPS.  Applied at 3FPS.
  PluginControl - Read at 5FPS.  Applied on read.


Telemetry state:
  rF2 calls UpdateTelemetry for each vehicle.  The Plugin tries to guess when all vehicles have received an update, and only after
  that the buffer write is marked as complete.


Extended state:
  Exposed extended state consists of:

  * Non periodically updated game state:
      Physics settings updates and various callback based properties are tracked.

  * Heuristic data exposed as an attempt to compensate for values not currently available from the game:
      Damage state is tracked, since game provides no accumulated damage data.  Tracking happens on _every_ telemetry/scoring
      update for full precision.

  * Captures parts of rF2Scoring contents when SessionEnd/SessionStart is invoked.  This helps callers to last update information
    from the previous session.  Note: In future, might get replaced with the full capture of rF2Scoring.

  See SharedMemoryPlugin::ExtendedStateTracker struct for details.

  Extended state exposes values obtained via Direct Memory access.  This functionality is enabled via "EnableDirectMemoryAccess"
  plugin variable.  See DirectMemoryReader class for more details.

  Lastly, active plugin configuration is exposed with the intent that clients will be able to detect missing features dynamically.


Output buffer synchronization:
  The Plugin does not offer hard guarantees for mapped buffer synchronization, because using synchronization primitives opens door for misuse 
  and eventually, way of harming game FPS as the number of clients grows.

  However, each of shared memory buffers begins with rF2MappedBufferVersionBlock structure.  If you would like to make sure you're not
  reading a torn (partially overwritten) frame, you can check rF2MappedBufferVersionBlock::mVersionUpdateBegin and
  rF2MappedBufferVersionBlock::mVersionUpdateEnd values. If they are equal, buffer is either not torn, or, in an extreme case,
  currently being written into.
  
  Note: $rFactor2SMMP_ForceFeedback$ buffer consists of a single double variable.  Since write into double is atomic, a version block
  is not used (I assume compiler aligned double member correctly for x64, and I am too lazy atm to check).

  Most clients (HUDs, Dashes, visualizers) won't need synchronization.  There are many ways on detecting torn frames,
  Monitor app contains sample approach used in the Crew Chief app.
  * For basic reading from C#, see: rF2SMMonitor.MappedBuffer<>.GetMappedDataUnsynchronized.
  * To see one of the ways to avoid torn frames, see: rF2SMMonitor.MappedBuffer<>.GetMappedData.


Input buffer synchronization:
  Input buffers are designed with a single client in mind.  The Plugin detects that input buffer has changed if the last saved value of
  rF2MappedBufferVersionBlock::mVersionUpdateBegin has changed.  The Plugin will attempt to detect "torn frame" situations, but won't 
  attempt any recovery.

  See rF2SMMonitor.MappedBuffer<>.PutMappedData for sample client code.


Dedicated server use:
  If run in a dedicated server process, each shared memory buffer name has the server PID appended.  If DedicatedServerMapGlobally
  preference is set to 1, the plugin will attempt to create shared memory buffers in the Global section.  Note that "Create Global Objects"
  permission is needed on the user account running the dedicated server.


Configuration:
  Standard rF2 plugin configuration is used.  See: SharedMemoryPlugin::AccessCustomVariable.


Limitations/Assumptions:
  - Negative mID is not supported.
  - Distance between max(mID) and min(mID) in a session cannot exceed rF2MappedBufferHeader::MAX_MAPPED_IDS.
  - Max mapped vehicles: rF2MappedBufferHeader::MAX_MAPPED_VEHICLES.
  - The Plugin assumes that delta Elapsed Time in a telemetry update frame cannot exceed 20ms (which effectively limits telemetry refresh rate to 50FPS).


Sample consumption:
  For sample C# client, see Monitor\rF2SMMonitor\rF2SMMonitor\MainForm.cs
*/
#include "LMU_SharedMemoryMap.hpp"          // corresponding header file
#include <stdlib.h>
#include <cstddef>                              // offsetof

long SharedMemoryPlugin::msDebugOutputLevel = static_cast<long>(DebugLevel::Off);
static_assert(sizeof(long) <= sizeof(DebugLevel), "sizeof(long) <= sizeof(DebugLevel)");

long SharedMemoryPlugin::msDebugOutputSource = static_cast<long>(DebugSource::General);
static_assert(sizeof(long) <= sizeof(DebugSource), "sizeof(long) <= sizeof(DebugSource)");

bool SharedMemoryPlugin::msDebugISIInternals = false;
bool SharedMemoryPlugin::msDedicatedServerMapGlobally = false;
bool SharedMemoryPlugin::msDirectMemoryAccessRequested = true;

long SharedMemoryPlugin::msUnsubscribedBuffersMask = 0L;
static_assert(sizeof(long) <= sizeof(SubscribedBuffer), "sizeof(long) <= sizeof(SubscribedBuffer)");

bool SharedMemoryPlugin::msHWControlInputRequested = false;
bool SharedMemoryPlugin::msWeatherControlInputRequested = false;
bool SharedMemoryPlugin::msRulesControlInputRequested = false;
bool SharedMemoryPlugin::mGameModeInitialized = false;

FILE* SharedMemoryPlugin::msDebugFile;
FILE* SharedMemoryPlugin::msIsiTelemetryFile;
FILE* SharedMemoryPlugin::msIsiScoringFile;

//char const* const SharedMemoryPlugin::MM_TELEMETRY_FILE_NAME = "$rFactor2SMMP_Telemetry$";
//char const* const SharedMemoryPlugin::MM_SCORING_FILE_NAME = "$rFactor2SMMP_Scoring$";
//char const* const SharedMemoryPlugin::MM_RULES_FILE_NAME = "$rFactor2SMMP_Rules$";
//char const* const SharedMemoryPlugin::MM_MULTI_RULES_FILE_NAME = "$rFactor2SMMP_MultiRules$";
//char const* const SharedMemoryPlugin::MM_FORCE_FEEDBACK_FILE_NAME = "$rFactor2SMMP_ForceFeedback$";
//char const* const SharedMemoryPlugin::MM_GRAPHICS_FILE_NAME = "$rFactor2SMMP_Graphics$";
char const* const SharedMemoryPlugin::MM_EXTENDED_FILE_NAME = "$LMU_SMMP_Extended$";
//char const* const SharedMemoryPlugin::MM_PIT_INFO_FILE_NAME = "$rFactor2SMMP_PitInfo$";
//char const* const SharedMemoryPlugin::MM_WEATHER_FILE_NAME = "$rFactor2SMMP_Weather$";
//
//char const* const SharedMemoryPlugin::MM_HWCONTROL_FILE_NAME = "$rFactor2SMMP_HWControl$";
//char const* const SharedMemoryPlugin::MM_WEATHER_CONTROL_FILE_NAME = "$rFactor2SMMP_WeatherControl$";
//char const* const SharedMemoryPlugin::MM_RULES_CONTROL_FILE_NAME = "$rFactor2SMMP_RulesControl$";
//char const* const SharedMemoryPlugin::MM_PLUGIN_CONTROL_FILE_NAME = "$rFactor2SMMP_PluginControl$";
//
//char const* const SharedMemoryPlugin::INTERNALS_TELEMETRY_FILENAME = R"(UserData\Log\RF2SMMP_InternalsTelemetryOutput.txt)";
//char const* const SharedMemoryPlugin::INTERNALS_SCORING_FILENAME = R"(UserData\Log\RF2SMMP_InternalsScoringOutput.txt)";
char const* const SharedMemoryPlugin::DEBUG_OUTPUT_FILENAME = R"(UserData\Log\LMU_SMMP_DebugOutput.txt)";

// plugin information
extern "C" __declspec(dllexport)
char const* __cdecl GetPluginName() { return PLUGIN_NAME_AND_VERSION; }

extern "C" __declspec(dllexport)
PluginObjectType __cdecl GetPluginType() { return(PO_INTERNALS); }

extern "C" __declspec(dllexport)
int __cdecl GetPluginVersion() { return(7); } // InternalsPluginV07 functionality (if you change this return value, you must derive from the appropriate class!)

extern "C" __declspec(dllexport)
PluginObject* __cdecl CreatePluginObject() { return((PluginObject*) new SharedMemoryPlugin); }

extern "C" __declspec(dllexport)
void __cdecl DestroyPluginObject(PluginObject* obj) { delete((SharedMemoryPlugin*) obj); }


//////////////////////////////////////
// SharedMemoryPlugin class
//////////////////////////////////////

SharedMemoryPlugin::SharedMemoryPlugin()
  :mExtended(SharedMemoryPlugin::MM_EXTENDED_FILE_NAME)
{
    memset(mParticipantTelemetryUpdated, 0, sizeof(mParticipantTelemetryUpdated));
}



void SharedMemoryPlugin::Startup(long version)
{
  // Print out configuration.
#ifdef VERSION_AVX2
#ifdef VERSION_MT
  DEBUG_MSG(DebugLevel::CriticalInfo, DebugSource::General, "Starting rFactor 2 Shared Memory Map Plugin 64bit Version: %s", SHARED_MEMORY_VERSION " AVX2+PGO+MT");
#else
  DEBUG_MSG(DebugLevel::CriticalInfo, DebugSource::General, "Starting rFactor 2 Shared Memory Map Plugin 64bit Version: %s", SHARED_MEMORY_VERSION " AVX2+PGO");
#endif
#else
  DEBUG_MSG(DebugLevel::CriticalInfo, DebugSource::General, "Starting LMU Shared Memory Map Plugin 64bit Version: %s", SHARED_MEMORY_VERSION);
#endif
 


  char charBuff[80] = {};
  sprintf(charBuff, "-STARTUP- (version %.3f)", (float)version / 1000.0f);
  //WriteToAllExampleOutputFiles("w", charBuff);
  DEBUG_MSG(DebugLevel::CriticalInfo, DebugSource::General, "-STARTUP- (version %.3f)", (float)version / 1000.0f);
  // Extended buffer is initialized last and is an indicator of initialization completed.
  RETURN_IF_FALSE(InitMappedBuffer(mExtended, "Extended", SubscribedBuffer::All));



  mExtStateTracker.mExtended.mUnsubscribedBuffersMask = SharedMemoryPlugin::msUnsubscribedBuffersMask;
  if (SharedMemoryPlugin::msDirectMemoryAccessRequested) {
    if (!mDMR.Initialize()) {
      DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "Failed to initialize DMA, disabling DMA.");

      // Disable DMA on failure.
      SharedMemoryPlugin::msDirectMemoryAccessRequested = false;
      mExtStateTracker.mExtended.mDirectMemoryAccessEnabled = false;
    }
    else {
      mExtStateTracker.mExtended.mDirectMemoryAccessEnabled = true;
    }
  }

  mIsMapped = true;
  DEBUG_MSG(DebugLevel::CriticalInfo, DebugSource::General, "Files mapped successfully.");

  ClearState();

  mExtended.BeginUpdate();
  memcpy(mExtended.mpWriteBuff, &(mExtStateTracker.mExtended), sizeof(LMU_Extended));
  mExtended.EndUpdate();
}


void SharedMemoryPlugin::Shutdown()
{
 // WriteToAllExampleOutputFiles("a", "-SHUTDOWN-");

  //if (mIsMapped)
    //TelemetryCompleteFrame();

  DEBUG_MSG(DebugLevel::CriticalInfo, DebugSource::General, "Shutting down");

  if (msDebugFile != nullptr) {
    fclose(msDebugFile);
    msDebugFile = nullptr;
  }

  if (msIsiTelemetryFile != nullptr) {
    fclose(msIsiTelemetryFile);
    msIsiTelemetryFile = nullptr;
  }

  if (msIsiScoringFile != nullptr) {
    fclose(msIsiScoringFile);
    msIsiScoringFile = nullptr;
  }

  mIsMapped = false;

  mExtended.ClearState(nullptr /*pInitialContents*/);
  mExtended.ReleaseResources();


}

void SharedMemoryPlugin::ClearTimingsAndCounters()
{
  //TelemetryCompleteFrame();

  mLastTelemetryUpdateMillis = 0.0;
  mLastTelemetryVehicleAddedMillis = 0.0;
  mLastScoringUpdateMillis = 0.0;
  mLastRulesUpdateMillis = 0.0;
  mLastMultiRulesUpdateMillis = 0.0;

  mLastTelemetryUpdateET = -1.0;
  mLastScoringUpdateET = -1.0;

  mCurrTelemetryVehicleIndex = 0;

  memset(mParticipantTelemetryUpdated, 0, sizeof(mParticipantTelemetryUpdated));

  mLastUpdateLSIWasVisible = false;

  mPitMenuLastCategoryIndex = -1L;
  mPitMenuLastChoiceIndex = -1L;
  mPitMenuLastNumChoices = -1L;

  mHWControlRequestReadCounter = 0;
  mHWControlRequestBoostCounter = 0;
  
  mHWControlInputRequestReceived = false;
  mWeatherControlInputRequestReceived = false;
  mRulesControlInputRequestReceived = false;
}


void SharedMemoryPlugin::ClearState()
{
  if (!mIsMapped)
    return;

  // Certain members of the extended state persist between restarts/sessions.
  // So, clear the state but pass persisting state as initial state.
  mExtended.ClearState(&(mExtStateTracker.mExtended));

  ClearTimingsAndCounters();
}


void SharedMemoryPlugin::StartSession()
{
  //WriteToAllExampleOutputFiles("a", "--STARTSESSION--");

  if (!mIsMapped)
    return;

  DEBUG_MSG(DebugLevel::Timing, DebugSource::General, "SESSION - Started.");

  mExtStateTracker.mExtended.mSessionStarted = true;
  mExtStateTracker.mExtended.mTicksSessionStarted = ::GetTickCount64();

  // Sometimes, game sends updates, including final qualification positions,
  // between Session Start/End.  We need to capture some of that info, because
  // it might be overwritten by the next session.
  // Current read buffer for Scoring info contains last Scoring Update.
  mDMR.ResetFounds();
  if (SharedMemoryPlugin::msDirectMemoryAccessRequested) {
    if (!mDMR.ReadOnNewSession(mExtStateTracker.mExtended)) {
      DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "DMA read failed, disabling.");

      // Disable DMA on failure.
      SharedMemoryPlugin::msDirectMemoryAccessRequested = false;
      mExtStateTracker.mExtended.mDirectMemoryAccessEnabled = false;
    }
  }

  // Clear state will do the flip for extended state.
  ClearState();
}


void SharedMemoryPlugin::EndSession()
{
  //WriteToAllExampleOutputFiles("a", "--ENDSESSION--");

  if (!mIsMapped)
    return;
  mDMR.ResetFounds();
  //TelemetryCompleteFrame();
  SharedMemoryPlugin::mGameModeInitialized = false;
  DEBUG_MSG(DebugLevel::Timing, DebugSource::General, "SESSION - Ended.");

  mExtStateTracker.mExtended.mSessionStarted = false;
  mExtStateTracker.mExtended.mTicksSessionEnded = ::GetTickCount64();

  mExtended.BeginUpdate();
  memcpy(mExtended.mpWriteBuff, &(mExtStateTracker.mExtended), sizeof(LMU_Extended));
  mExtended.EndUpdate();
}

void SharedMemoryPlugin::UpdateScoring(ScoringInfoV01 const& info)
{
  
  if (!mIsMapped)
    return;

  if (SharedMemoryPlugin::msDirectMemoryAccessRequested) {

    long mID = -1;
    for (int i = 0; i < info.mNumVehicles; ++i)
    {
      VehicleScoringInfoV01 vehicle = info.mVehicle[i];
      //DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "vechicle mID: %d mIsPlayer %d mPlace %d mControl: %d", vehicle.mID , (int)vehicle.mIsPlayer, vehicle.mPlace, vehicle.mControl);
        if (info.mVehicle[i].mIsPlayer)
        {
          mID = info.mVehicle[i].mID;
          break;
        }
    }

    if (!mDMR.Read(mExtStateTracker.mExtended, mID)) {  // Read on FCY or Formation lap.
      DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "DMA read failed, disabling.");

      // Disable DMA on failure.
      SharedMemoryPlugin::msDirectMemoryAccessRequested = false;
      mExtStateTracker.mExtended.mDirectMemoryAccessEnabled = false;
      // Extended flip will happen in ScoringUpdate.
    }
  }
  //mExtStateTracker.ProcessScoringUpdate(info);

  mExtended.BeginUpdate();
  memcpy(mExtended.mpWriteBuff, &(mExtStateTracker.mExtended), sizeof(LMU_Extended));
  mExtended.EndUpdate();
}

void SharedMemoryPlugin::UpdateInRealtimeFC(bool inRealTime)
{
  if (!mIsMapped)
    return;
  


  DEBUG_MSG(DebugLevel::Synchronization, DebugSource::General, inRealTime ? "Entering Realtime" : "Exiting Realtime");

  mExtStateTracker.mExtended.mInRealtimeFC = inRealTime;

  mExtended.BeginUpdate();
  memcpy(mExtended.mpWriteBuff, &(mExtStateTracker.mExtended), sizeof(LMU_Extended));
  mExtended.EndUpdate();
}


void SharedMemoryPlugin::EnterRealtime()
{
  if (!mIsMapped)
    return;

  // start up timer every time we enter realtime
  //WriteToAllExampleOutputFiles("a", "---ENTERREALTIME---");

  UpdateInRealtimeFC(true /*inRealtime*/);
}


void SharedMemoryPlugin::ExitRealtime()
{
  if (!mIsMapped)
    return;

  //TelemetryCompleteFrame();

  //WriteToAllExampleOutputFiles("a", "---EXITREALTIME---");

  UpdateInRealtimeFC(false /*inRealtime*/);
}


// Using GTC64 produces 7x larger average interpolation delta (roughly from 5cm to 35cm).
// The max offset stays close, so it might not matter much.
// So, let's keep QPC and see if it causes problems (FPS cost)?
#define USE_QPC
double TicksNow() {
#ifdef USE_QPC
  static double frequencyMicrosecond = 0.0;
  static bool once = false;
  if (!once) {
    LARGE_INTEGER qpcFrequency = {};
    QueryPerformanceFrequency(&qpcFrequency);
    frequencyMicrosecond = static_cast<double>(qpcFrequency.QuadPart) / MICROSECONDS_IN_SECOND;
    once = true;
  }

  LARGE_INTEGER now = {};
  QueryPerformanceCounter(&now);
  return static_cast<double>(now.QuadPart) / frequencyMicrosecond;
#else
  return GetTickCount64() * MICROSECONDS_IN_MILLISECOND;
#endif
}



/*
rF2 sends telemetry updates for each vehicle.  The problem is that we do not know when all vehicles received an update.
Below I am trying to complete buffer update per-frame, where "frame" means all vehicles received telemetry update.

I am detecting a new frame by checking time distance between mElapsedTime.  It appears that rF2 sends vehicle telemetry every 20ms
(every 10ms really, but most of the time contents are duplicated).  As a consquence, we flip every 20ms (50FPS).

Frame end is detected by either:
- checking if number of vehicles in telemetry frame matching number of vehicles reported via scoring
- we detect cycle (same mID updated twice)

Note that sometimes mElapsedTime for player vehicle is slightly ahead of the rest of vehicles (but never more than 20ms, most often being 2.5ms off).
This mostly happens during first few seconds of going green.

There's an alternative approach that can be taken: it appears that game sends vehicle telemetry ordered by mID (ascending order).
So we could detect new frame by checking mIDs and cut when mIDPrev >= mIDCurrent.
*/

template<typename BuffT>
bool SharedMemoryPlugin::InitMappedBuffer(BuffT& buffer, char const* const buffLogicalName, SubscribedBuffer sb)
{
  if (sb != SubscribedBuffer::All  // All indicates that buffer cannot be unsubscribed from.
    && Utils::IsFlagOn(SharedMemoryPlugin::msUnsubscribedBuffersMask, sb))
    DEBUG_MSG(DebugLevel::CriticalInfo, DebugSource::General, "Unsubscribed from the %s updates", buffLogicalName);

  if (!buffer.Initialize(SharedMemoryPlugin::msDedicatedServerMapGlobally)) {
    DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "Failed to initialize %s mapping", buffLogicalName);
    return false;
  }

  auto const size = static_cast<int>(sizeof(BuffT::BufferType) + sizeof(LMU_MappedBufferVersionBlock));
  DEBUG_MSG(DebugLevel::CriticalInfo, DebugSource::General, "Size of the %s buffer: %d bytes.", buffLogicalName, size);

  return true;
}


template<typename BuffT>
bool SharedMemoryPlugin::InitMappedInputBuffer(BuffT& buffer, char const* const buffLogicalName)
{
  if (!buffer.Initialize(SharedMemoryPlugin::msDedicatedServerMapGlobally)) {
    DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "Failed to initialize %s input mapping", buffLogicalName);
    return false;
  }

  auto const size = static_cast<int>(sizeof(BuffT::BufferType) + sizeof(rF2MappedBufferVersionBlock));
  DEBUG_MSG(DebugLevel::CriticalInfo, DebugSource::General, "Size of the %s buffer: %d bytes.  %s input buffer supported layout version: '%ld'", buffLogicalName, size, buffLogicalName, BuffT::BufferType::SUPPORTED_LAYOUT_VERSION);

  return true;
}


template <typename BuffT>
void SharedMemoryPlugin::TraceBeginUpdate(BuffT const& buffer, double& lastUpdateMillis, char const msgPrefix[]) const
{
  auto ticksNow = 0.0;
  if (Utils::IsFlagOn(SharedMemoryPlugin::msDebugOutputLevel, DebugLevel::Timing)) {
    ticksNow = TicksNow();
    auto const delta = ticksNow - lastUpdateMillis;

    DEBUG_MSG(DebugLevel::Timing, DebugSource::Telemetry | DebugSource::Scoring, "%s - Begin Update:  Delta since last update:%f  Version Begin:%ld  End:%ld", msgPrefix, delta / MICROSECONDS_IN_SECOND,
      buffer.mpWriteBuffVersionBlock->mVersionUpdateBegin, buffer.mpWriteBuffVersionBlock->mVersionUpdateEnd);

    lastUpdateMillis = ticksNow;
  }
}




//void SharedMemoryPlugin::ReadDMROnScoringUpdate(ScoringInfoV01 const& info)
//{
//  if (SharedMemoryPlugin::msDirectMemoryAccessRequested) {
//    auto const LSIVisible = info.mYellowFlagState != 0 || info.mGamePhase == static_cast<unsigned char>(rF2GamePhase::Formation);
//    if (!mDMR.Read(mExtStateTracker.mExtended)
//      || (LSIVisible && !mDMR.ReadOnLSIVisible(mExtStateTracker.mExtended))) {  // Read on FCY or Formation lap.
//      DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "DMA read failed, disabling.");
//
//      // Disable DMA on failure.
//      SharedMemoryPlugin::msDirectMemoryAccessRequested = false;
//      mExtStateTracker.mExtended.mDirectMemoryAccessEnabled = false;
//      // Extended flip will happen in ScoringUpdate.
//    }
//    else {  // Read succeeded.
//      if (mLastUpdateLSIWasVisible && !LSIVisible)
//        mDMR.ClearLSIValues(mExtStateTracker.mExtended);  // Clear LSI Values on LSI going away.
//
//      mLastUpdateLSIWasVisible = LSIVisible;
//    }
//  }
//}



// TODO: if STL is added eventually, use map SubscribedBuffer -> buffLogicalName
void SharedMemoryPlugin::DynamicallySubscribeToBuffer(SubscribedBuffer sb, long requestedBuffMask, char const* const buffLogicalName)
{
  if (Utils::IsFlagOn(SharedMemoryPlugin::msUnsubscribedBuffersMask, sb)
    && Utils::IsFlagOn(requestedBuffMask, sb)) {
    DEBUG_MSG(DebugLevel::CriticalInfo, DebugSource::General, "Subscribing to the %s updates based on the dynamic request.", buffLogicalName);

    SharedMemoryPlugin::msUnsubscribedBuffersMask ^= static_cast<long>(sb);
  }
}


void SharedMemoryPlugin::DynamicallyEnableInputBuffer(bool dependencyMissing, bool& controlInputRequested, bool& controlIputEnabled, char const* const buffLogicalName)
{
  if (!dependencyMissing) {
    DEBUG_MSG(DebugLevel::CriticalInfo, DebugSource::General, "Enabling %s input updates based on the dynamic request.", buffLogicalName);

    // Dynamic enable is allowed only once.
    controlInputRequested = controlIputEnabled = true;
  }
}



void SharedMemoryPlugin::UpdateThreadState(void)
{
  /*(type == 0 ? mExtStateTracker.mExtended.mMultimediaThreadStarted : mExtStateTracker.mExtended.mSimulationThreadStarted)
    = starting;*/

  mExtended.BeginUpdate();
  memcpy(mExtended.mpWriteBuff, &(mExtStateTracker.mExtended), sizeof(LMU_Extended));
  mExtended.EndUpdate();
}


void SharedMemoryPlugin::ThreadStarted(long type)
{
  if (!mIsMapped)
    return;

  DEBUG_MSG(DebugLevel::Synchronization, DebugSource::General, type == 0 ? "Multimedia thread started" : "Simulation thread started");
  UpdateThreadState();
}


void SharedMemoryPlugin::ThreadStopping(long type)
{
  if (!mIsMapped)
    return;

  DEBUG_MSG(DebugLevel::Synchronization, DebugSource::General, type == 0 ? "Multimedia thread stopped" : "Simulation thread stopped");
  UpdateThreadState();
}



bool SharedMemoryPlugin::GetCustomVariable(long i, CustomVariableV01& var)
{
  DEBUG_MSG(DebugLevel::Timing, DebugSource::General, "GetCustomVariable - Invoked:  mCaption - '%s'", var.mCaption);

  if (i == 0) {
    // rF2 will automatically create this variable and default it to 1 (true) unless we create it first, in which case we can choose the default.
    strcpy_s(var.mCaption, " Enabled");
    var.mNumSettings = 2;
    var.mCurrentSetting = 0;
    return true;
  }
  else if (i == 1) {
    strcpy_s(var.mCaption, "DebugOutputLevel");
    var.mNumSettings = 1;
    var.mCurrentSetting = 0;
    return true;
  }
  else if (i == 2) {
    strcpy_s(var.mCaption, "DebugISIInternals");
    var.mNumSettings = 2;
    var.mCurrentSetting = 0;
    return true;
  }
  else if (i == 3) {
    strcpy_s(var.mCaption, "DedicatedServerMapGlobally");
    var.mNumSettings = 2;
    var.mCurrentSetting = 0;
    return true;
  }
  else if (i == 4) {
    strcpy_s(var.mCaption, "EnableDirectMemoryAccess");
    var.mNumSettings = 2;
    var.mCurrentSetting = 0;
    return true;
  }
  else if (i == 5) {
    strcpy_s(var.mCaption, "UnsubscribedBuffersMask");
    var.mNumSettings = 1;

    // By default, unsubscribe from the Graphics and Weather buffer updates.
    // CC does not need some other buffers either, however it is going to be a headache
    // to explain SH users who rely on them how to configure plugin, so let it be.
    var.mCurrentSetting = 160;
    return true;
  }
  else if (i == 6) {
    strcpy_s(var.mCaption, "EnableHWControlInput");
    var.mNumSettings = 2;
    var.mCurrentSetting = 1;
    return true;
  }
  else if (i == 7) {
    strcpy_s(var.mCaption, "EnableWeatherControlInput");
    var.mNumSettings = 2;
    var.mCurrentSetting = 0;
    return true;
  }
  else if (i == 8) {
    strcpy_s(var.mCaption, "EnableRulesControlInput");
    var.mNumSettings = 2;
    var.mCurrentSetting = 0;
    return true;
  }
  else if (i == 9) {
    strcpy_s(var.mCaption, "DebugOutputSource");
    var.mNumSettings = 1;
    var.mCurrentSetting = 1;
    return true;
  }

  return false;
}


void SharedMemoryPlugin::AccessCustomVariable(CustomVariableV01& var)
{
  //DEBUG_MSG(DebugLevel::Timing, DebugSource::General, "AccessCustomVariable - Invoked.");

  if (_stricmp(var.mCaption, " Enabled") == 0)
    ; // Do nothing; this variable is just for rF2 to know whether to keep the plugin loaded.
  else if (_stricmp(var.mCaption, "DebugOutputLevel") == 0) {
    auto sanitized = min(max(var.mCurrentSetting, 0L), static_cast<long>(DebugLevel::All));
    SharedMemoryPlugin::msDebugOutputLevel = sanitized;

    // Remove previous debug output.
    if (SharedMemoryPlugin::msDebugOutputLevel != static_cast<long>(DebugLevel::Off))
      remove(SharedMemoryPlugin::DEBUG_OUTPUT_FILENAME);
  }
  else if (_stricmp(var.mCaption, "DebugISIInternals") == 0)
    SharedMemoryPlugin::msDebugISIInternals = var.mCurrentSetting != 0;
  else if (_stricmp(var.mCaption, "DedicatedServerMapGlobally") == 0)
    SharedMemoryPlugin::msDedicatedServerMapGlobally = var.mCurrentSetting != 0;
  else if (_stricmp(var.mCaption, "EnableDirectMemoryAccess") == 0)
    SharedMemoryPlugin::msDirectMemoryAccessRequested = var.mCurrentSetting != 0;
  else if (_stricmp(var.mCaption, "UnsubscribedBuffersMask") == 0) {
    auto sanitized = min(max(var.mCurrentSetting, 0L), static_cast<long>(SubscribedBuffer::All));
    SharedMemoryPlugin::msUnsubscribedBuffersMask = sanitized;
  }
  else if (_stricmp(var.mCaption, "EnableHWControlInput") == 0)
    SharedMemoryPlugin::msHWControlInputRequested = var.mCurrentSetting != 0;
  else if (_stricmp(var.mCaption, "EnableWeatherControlInput") == 0)
    SharedMemoryPlugin::msWeatherControlInputRequested = var.mCurrentSetting != 0;
  else if (_stricmp(var.mCaption, "EnableRulesControlInput") == 0)
    SharedMemoryPlugin::msRulesControlInputRequested = var.mCurrentSetting != 0;
  else if (_stricmp(var.mCaption, "DebugOutputSource") == 0) {
    auto sanitized = min(max(var.mCurrentSetting, 1L), static_cast<long>(DebugSource::All));
    SharedMemoryPlugin::msDebugOutputSource = sanitized;
  }
}


void SharedMemoryPlugin::GetCustomVariableSetting(CustomVariableV01& var, long i, CustomSettingV01& setting)
{
  DEBUG_MSG(DebugLevel::Timing, DebugSource::General, "GetCustomVariableSetting - Invoked.");

  if (_stricmp(var.mCaption, " Enabled") == 0) {
    if (i == 0)
      strcpy_s(setting.mName, "False");
    else
      strcpy_s(setting.mName, "True");
  }
  else if (_stricmp(var.mCaption, "DebugISIInternals") == 0) {
    if (i == 0)
      strcpy_s(setting.mName, "False");
    else
      strcpy_s(setting.mName, "True");
  }
  else if (_stricmp(var.mCaption, "DedicatedServerMapGlobally") == 0) {
    if (i == 0)
      strcpy_s(setting.mName, "False");
    else
      strcpy_s(setting.mName, "True");
  }
  else if (_stricmp(var.mCaption, "EnableDirectMemoryAccess") == 0) {
    if (i == 0)
      strcpy_s(setting.mName, "False");
    else
      strcpy_s(setting.mName, "True");
  }
  else if (_stricmp(var.mCaption, "EnableHWControlInput") == 0) {
    if (i == 0)
      strcpy_s(setting.mName, "False");
    else
      strcpy_s(setting.mName, "True");
  }
  else if (_stricmp(var.mCaption, "EnableWeatherControlInput") == 0) {
    if (i == 0)
      strcpy_s(setting.mName, "False");
    else
      strcpy_s(setting.mName, "True");
  }
  else if (_stricmp(var.mCaption, "EnableRulesControlInput") == 0) {
    if (i == 0)
      strcpy_s(setting.mName, "False");
    else
      strcpy_s(setting.mName, "True");
  }
}


////////////////////////////////////////////
// Debug output helpers.
////////////////////////////////////////////

//void SharedMemoryPlugin::WriteToAllExampleOutputFiles(char const* const openStr, char const* const msg)
//{
//  if (!SharedMemoryPlugin::msDebugISIInternals)
//    return;
//
//  //auto fo = fopen(SharedMemoryPlugin::INTERNALS_TELEMETRY_FILENAME, openStr);
//  //if (fo != nullptr) {
//  //  fprintf(fo, "%s\n", msg);
//  //  fclose(fo);
//  //}
//
//  //fo = fopen(SharedMemoryPlugin::INTERNALS_SCORING_FILENAME, openStr);
//  //if (fo != nullptr) {
//  //  fprintf(fo, "%s\n", msg);
//  //  fclose(fo);
//  //}
//}


void SharedMemoryPlugin::WriteDebugMsg(
  DebugLevel lvl,
  long src,
  char const* const functionName,
  int line,
  char const* const msg,
  ...)
{
  if (Utils::IsFlagOff(SharedMemoryPlugin::msDebugOutputLevel, lvl)
    || Utils::IsFlagOff(SharedMemoryPlugin::msDebugOutputSource, src))
    return;

  va_list argList;
  if (SharedMemoryPlugin::msDebugFile == nullptr) {
    SharedMemoryPlugin::msDebugFile = _fsopen(SharedMemoryPlugin::DEBUG_OUTPUT_FILENAME, "a", _SH_DENYNO);
    setvbuf(SharedMemoryPlugin::msDebugFile, nullptr, _IOFBF, SharedMemoryPlugin::BUFFER_IO_BYTES);
  }

  SYSTEMTIME st = {};
  ::GetLocalTime(&st);

  fprintf_s(SharedMemoryPlugin::msDebugFile, "%.2d:%.2d:%.2d.%.3d TID:0x%04lx  ", st.wHour, st.wMinute, st.wSecond , st.wMilliseconds, ::GetCurrentThreadId());
  fprintf_s(SharedMemoryPlugin::msDebugFile, "%s(%d) : ", functionName, line);

  if (lvl == DebugLevel::Errors)
    fprintf_s(SharedMemoryPlugin::msDebugFile, "ERROR: ");
  else if (lvl == DebugLevel::Warnings)
    fprintf_s(SharedMemoryPlugin::msDebugFile, "WARNING: ");

  if (SharedMemoryPlugin::msDebugFile != nullptr) {
    va_start(argList, msg);
    vfprintf_s(SharedMemoryPlugin::msDebugFile, msg, argList);
    va_end(argList);
  }

  fprintf_s(SharedMemoryPlugin::msDebugFile, "\n");

  // Flush periodically for low volume messages.
  static ULONGLONG lastFlushTicks = 0uLL;
  auto const ticksNow = GetTickCount64();
  if ((ticksNow - lastFlushTicks) / MILLISECONDS_IN_SECOND > DEBUG_IO_FLUSH_PERIOD_SECS) {
    fflush(SharedMemoryPlugin::msDebugFile);
    lastFlushTicks = ticksNow;
  }
}


void SharedMemoryPlugin::TraceLastWin32Error()
{
  if (Utils::IsFlagOn(SharedMemoryPlugin::msDebugOutputLevel, DebugLevel::Errors))
    return;

  auto const lastError = ::GetLastError();
  if (lastError == 0)
    return;

  LPSTR messageBuffer = nullptr;
  auto const retChars = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    nullptr  /*lpSource*/, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&messageBuffer),
    0  /*nSize*/, nullptr  /*argunments*/);

  DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "Win32 error code: %d", lastError);

  if (retChars > 0 && messageBuffer != nullptr)
    DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "Win32 error description: %s", messageBuffer);

  ::LocalFree(messageBuffer);
}
