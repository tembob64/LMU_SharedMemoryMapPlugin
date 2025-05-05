#include <windows.h>
#include <psapi.h>
#include "Utils.h"
#include "LMU_SharedMemoryMap.hpp"

namespace Utils
{


  //uintptr_t* FindPatternForPointerInMemory()
  //{
  //  HANDLE processHandle = ::GetCurrentProcess();
  //  if (processHandle == INVALID_HANDLE_VALUE || processHandle == NULL)
  //  {
  //    DEBUG_MSG(DebugLevel::Errors, DebugSource::All, "Error get handle");
  //    return nullptr;
  //  }

  //  DWORD pointsAddress = (DWORD)0x1763878FA2CuLL;
  //  ReadProcessMemory(processHandle, (LPVOID)pointsAddress, &pointsAddress, sizeof(pointsAddress), NULL);
  //}



  uintptr_t* GetValueFromMemory(HMODULE module, unsigned long long OffSet)
  {
    MODULEINFO info = {};
    ::GetModuleInformation(::GetCurrentProcess(), module, &info, sizeof(MODULEINFO));

    auto addressAbsoluteRIP = reinterpret_cast<uintptr_t>(module);
    if (addressAbsoluteRIP == 0uLL)
      return nullptr;
    
    DEBUG_MSG(DebugLevel::DevInfo, DebugSource::All, "StartPointerAddress %p" , addressAbsoluteRIP);
    addressAbsoluteRIP += OffSet;
    auto const valueSizeoffset = LODWORD(*reinterpret_cast<uintptr_t*>(addressAbsoluteRIP));
    return reinterpret_cast<uintptr_t*>(addressAbsoluteRIP + valueSizeoffset);
  }

uintptr_t* FindPatternForPointerInMemory(HMODULE module, unsigned char const* pattern, char const* mask, size_t bytedIntoPatternToFindOffset)
{

  MODULEINFO info = {};
  ::GetModuleInformation(::GetCurrentProcess(), module, &info, sizeof(MODULEINFO));

  //auto startAdress = reinterpret_cast<uintptr_t>(module);
  
  //size_t endAddress = info.SizeOfImage;
  //if (TCCarData) endAddress = info.SizeOfImage + 0x9000000000uLL;
  DEBUG_MSG(DebugLevel::DevInfo, DebugSource::All, "Start: %llx End: %llx", reinterpret_cast<uintptr_t>(module), info.SizeOfImage);
  auto addressAbsoluteRIP = FindPattern(reinterpret_cast<uintptr_t>(module), info.SizeOfImage , pattern, mask);
  if (addressAbsoluteRIP == 0uLL)
    return nullptr;


  addressAbsoluteRIP += bytedIntoPatternToFindOffset;
  //auto const offsetFromRIP = LODWORD(*reinterpret_cast<uintptr_t*>(addressAbsoluteRIP) + 1uLL );
  return reinterpret_cast<uintptr_t*>(addressAbsoluteRIP);

}

uintptr_t FindPattern(uintptr_t start, size_t length, unsigned char const* pattern, char const* mask)
{
  size_t maskPos = 0u;
  auto const maskLength = strlen(reinterpret_cast<char const*>(mask)) - 1;

  auto startAdress = start;
  for (auto currAddress = startAdress; currAddress < startAdress + length; ++currAddress) {
    if (*reinterpret_cast<unsigned char*>(currAddress) == pattern[maskPos] || mask[maskPos] == '?') {
      if (mask[maskPos + 1u] == '\0')
         return currAddress - maskLength;

      ++maskPos;
    } else
      maskPos = 0;
  }

  return 0uLL;
}


char* GetFileContents(char const* const filePath)
{
  FILE* fileHandle = nullptr;

  auto onExit = MakeScopeGuard(
    [&]() {
    if (fileHandle != nullptr) {
      auto ret = fclose(fileHandle);
      if (ret != 0)
        DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "fclose() failed with: %d", ret);
    }
  });

  char* fileContents = nullptr;
  auto ret = fopen_s(&fileHandle, filePath, "rb");
  if (ret != 0) {
    DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "fopen_s() failed with: %d", ret);
    return nullptr;
  }

  ret = fseek(fileHandle, 0, SEEK_END);
  if (ret != 0) {
    DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "fseek() failed with: %d", ret);
    return nullptr;
  }

  auto const fileBytes = static_cast<size_t>(ftell(fileHandle));
  rewind(fileHandle);

  fileContents = new char[fileBytes + 1];
  auto elemsRead = fread(fileContents, fileBytes, 1 /*items*/, fileHandle);
  if (elemsRead != 1 /*items*/) {
    delete[] fileContents;
    fileContents = nullptr;
    DEBUG_MSG(DebugLevel::Errors, DebugSource::General, "fread() failed.");
    return nullptr;
  }

  fileContents[fileBytes] = 0;

  return fileContents;
}

void  WriteMemoryToFile(HMODULE module)
{

  MODULEINFO info = {};
  ::GetModuleInformation(::GetCurrentProcess(), module, &info, sizeof(MODULEINFO));

  FILE* fo;

  auto startAdress = reinterpret_cast<uintptr_t>(module);
  fo = fopen(R"(UserData\Log\LMU_InternalsRAMOutput.txt)", "a");
  //unsigned char* msg = nullptr;

  if (fo != NULL)
  {
    int i = 0;
    for (auto currAddress = startAdress; currAddress < startAdress + 1152; ++currAddress)
    {
      if (i < 48)
      {
        fprintf(fo, "%02X", *reinterpret_cast<unsigned char*>(currAddress));
      }
      else
      {
        i = 0;
        fprintf(fo, "\n%02X", *reinterpret_cast<unsigned char*>(currAddress));
      }
      i++;
    }
      fclose(fo);

  }
}
}