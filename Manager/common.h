#pragma once
#include <Windows.h>

#define SHARED_MEM_NAME L"Local\\Variant23_SharedMem"

#define MUTEX_NAME L"Local\\Variant23_Mutex"

const int MAX_WORKERS = 5;

#pragma pack(push, 1)
struct SharedData {
  volatile LONG counter;      
  DWORD expectedValue;        
  DWORD startTime;          
  volatile LONG workersDone;  
};
#pragma pack(pop)

enum SyncMode {
  MODE_MUTEX = 1,
  MODE_INTERLOCKED = 2,
  MODE_NONE = 0
};