#include "common.h"
#include <iostream>
#include <cstdio>

void LogEvent(const char* workerId, const char* state) {
  printf("[%05lu ms] W%s PID=%lu STATE=%s\n",
    GetTickCount(), workerId, GetCurrentProcessId(), state);
}

int main(int argc, char* argv[]) {
  if (argc < 4) return 1;
  int mode = atoi(argv[1]);
  int operations = atoi(argv[2]);
  const char* workerId = argv[3];

  HANDLE hMap = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEM_NAME);
  if (!hMap) { printf("Worker: OpenFileMapping failed. Error: %lu\n", GetLastError()); return 1; }

  SharedData* shared = (SharedData*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData));
  if (!shared) { printf("Worker: MapViewOfFile failed\n"); return 1; }

  HANDLE hMutex = NULL;
  if (mode == MODE_MUTEX) {
    hMutex = OpenMutexW(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME);
    if (!hMutex) {
      printf("Worker %s: OpenMutex failed. Error: %lu\n", workerId, GetLastError());
      return 1;
    }
  }

  LogEvent(workerId, "START");

  for (int i = 0; i < operations; i++) {
    if (mode == MODE_MUTEX) {
      DWORD waitResult = WaitForSingleObject(hMutex, 1000); 
      if (waitResult == WAIT_OBJECT_0) {
        shared->counter++;
        ReleaseMutex(hMutex);
      }
      else if (waitResult == WAIT_TIMEOUT) {
        printf("Worker %s: TIMEOUT waiting for mutex!\n", workerId);
      }
    }
    else if (mode == MODE_INTERLOCKED) {
      InterlockedIncrement(&shared->counter);
    }
    else if (mode == MODE_NONE) {
      LONG temp = shared->counter;
      if (i % 100 == 0) Sleep(1); else SwitchToThread();
      shared->counter = temp + 1;
    }
  }

  LogEvent(workerId, "FINISHED");
  InterlockedIncrement(&shared->workersDone);

  if (hMutex) CloseHandle(hMutex);
  UnmapViewOfFile(shared);
  CloseHandle(hMap);

  return 0;
}