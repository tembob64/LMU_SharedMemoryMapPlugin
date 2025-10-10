# Used as template https://github.com/TheIronWolfModding/rF2SharedMemoryMapPlugin

## Instalation:
### 1. Extract LMU_SharedMemoryMapPlugin64.dll  to GAME_Install_dir\Plugins folder
### 2. if new instalation, you need modify Game Plugins Settings file. GAME_Install_dir\UserData\player\CustomPluginVariables.JSON
### 3. add or modify LMU_SharedMemoryMapPlugin64.dll section. set "Enabled" : 1 and "EnableDirectMemoryAccess": 1


## CustomPluginVariables.JSON example

 <pre>
 {
    "ChatTransceiver.dll": {
    " Enabled": 1
  },
  "LMU_SharedMemoryMapPlugin64.dll": {
    " Enabled": 1,
    "DebugISIInternals": 0,
    "DebugOutputLevel": 0,
    "DebugOutputSource": 0,
    "DedicatedServerMapGlobally": 0,
    "EnableDirectMemoryAccess": 1,
    "EnableHWControlInput": 0,
    "EnableRulesControlInput": 0,
    "EnableWeatherControlInput": 0,
    "UnsubscribedBuffersMask": 160
  },
  "TrackIR_LMU_Plugin.dll": {
    " Enabled": 1
  },
  "rFactor2SharedMemoryMapPlugin64.dll": {
    " Enabled": 1,
    "DebugISIInternals": 1,
    "DebugOutputLevel": 0,
    "DebugOutputSource": 0,
    "DedicatedServerMapGlobally": 0,
    "EnableDirectMemoryAccess": 0,
    "EnableHWControlInput": 0,
    "EnableRulesControlInput": 0,
    "EnableWeatherControlInput": 0,
    "UnsubscribedBuffersMask": 160
  }
};
</pre>
</body>
</html>
