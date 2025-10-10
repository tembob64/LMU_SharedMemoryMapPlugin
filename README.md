# LMU Internals Shared Memory Plugin
Used as template https://github.com/TheIronWolfModding/rF2SharedMemoryMapPlugin

Instalation:
1. Extract archive content to GAME_Install_dir\Plugins folder
2. if new instalation, you need modify Game Plugins Settings file. GAME_Install_dir\UserData\player\CustomPluginVariables.JSON
3. add or modify LMU_SharedMemoryMapPlugin64.dll section. set "Enabled" : 1 and "EnableDirectMemoryAccess": 1
<div>
CustomPluginVariables.JSON example
</div>

<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <title>Отображение JSON</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            padding: 20px;
        }
        pre {
            background-color: #f4f4f4;
            padding: 15px;
            border: 1px solid #ccc;
            overflow-x: auto;
        }
    </style>
</head>
<body>
    <h1>JSON Данные</h1>
    <pre id="json-output"></pre>

    <script>
        // Пример JSON-объекта
        const jsonData = {
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
        // Преобразуем JSON в строку с отступами и вставляем в HTML
        document.getElementById("json-output").textContent = JSON.stringify(jsonData, null, 4);
    </script>
</body>
</html>
