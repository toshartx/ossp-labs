#include "common.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstdio>

int main(int argc, char* argv[]) {
  setlocale(LC_ALL, "Russian");

  int numWorkers = 3;
  int operations = 50000;
  int mode = MODE_NONE;

  if (argc >= 2) numWorkers = atoi(argv[1]);
  if (argc >= 3) operations = atoi(argv[2]);
  if (argc >= 4) mode = atoi(argv[3]);

  int STUDENT_ID = 99;
  int finalOperations = operations + STUDENT_ID * 5;

  const char* modeName = (mode == MODE_NONE) ? "NONE (Race)" :
    (mode == MODE_MUTEX) ? "MUTEX" : "INTERLOCKED";
  printf("=== Manager ===\n");
  printf("Воркеров: %d, Операций: %d, Режим: %s\n", numWorkers, finalOperations, modeName);

  HANDLE hMap = CreateFileMappingW(
    INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
    0, sizeof(SharedData), SHARED_MEM_NAME
  );
  if (!hMap) { printf("CreateFileMapping failed: %lu\n", GetLastError()); return 1; }

  SharedData* shared = (SharedData*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData));

  shared->counter = 0;
  shared->workersDone = 0;
  shared->startTime = GetTickCount();
  shared->expectedValue = numWorkers * finalOperations;

  HANDLE hMutex = NULL;
  if (mode == MODE_MUTEX) {
    hMutex = CreateMutexW(NULL, FALSE, MUTEX_NAME);
    if (!hMutex) { printf("CreateMutex failed: %lu\n", GetLastError()); return 1; }
  }

  std::vector<PROCESS_INFORMATION> procs(numWorkers);
  LARGE_INTEGER freq, start, end;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&start);

  for (int i = 0; i < numWorkers; i++) {
    wchar_t cmdLine[256];
    swprintf(cmdLine, 256, L"Worker.exe %d %d W%d", mode, finalOperations, i + 1);

    STARTUPINFOW si = { sizeof(si) };
    if (!CreateProcessW(NULL, cmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &procs[i])) {
      printf("CreateProcess failed: %lu\n", GetLastError());
    }
  }

  std::vector<HANDLE> handles;
  for (int i = 0; i < numWorkers; i++) handles.push_back(procs[i].hProcess);
  WaitForMultipleObjects(numWorkers, handles.data(), TRUE, INFINITE);

  QueryPerformanceCounter(&end);
  double time_ms = (double)(end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;

  printf("\n=== Результаты ===\n");
  printf("Время: %.2f ms\n", time_ms);
  printf("Ожидалось: %lu, Получено: %ld\n", shared->expectedValue, shared->counter);

  if (shared->counter == shared->expectedValue) {
    printf("[OK] Инвариант соблюден.\n");
  }
  else {
    printf("[FAIL] НАРУШЕНИЕ ИНВАРИАНТА! Потеряно: %ld\n", shared->expectedValue - shared->counter);
  }

  for (int i = 0; i < numWorkers; i++) {
    CloseHandle(procs[i].hProcess);
    CloseHandle(procs[i].hThread);
  }
  UnmapViewOfFile(shared);
  if (hMutex) CloseHandle(hMutex);
  CloseHandle(hMap);

  return 0;
}