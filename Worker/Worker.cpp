#include "common.h"
#include <iostream>
#include <cstdio>

int main(int argc, char* argv[]) {
  if (argc < 3) return 1;
  int mode = atoi(argv[1]);
  int operations = atoi(argv[2]);
  DWORD pid = GetCurrentProcessId();

  HANDLE hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEM_NAME);
  if (!hMap) { printf("Worker: OpenFileMapping failed\n"); return 1; }

  SharedData* shared = (SharedData*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData));

  // 2. Основной цикл (БЕЗ СИНХРОНИЗАЦИИ)
  for (int i = 0; i < operations; i++) {
    // Искусственно расширяем окно гонки (Race Window), 
    // чтобы продемонстрировать потерю данных.
    // В реальной жизни такое случается из-за прерываний ОС.
    LONG temp = shared->counter;
    if (i % 10 == 0) {
      SwitchToThread(); // Уступаем процессор другому процессу
    }
    
    else if (mode == MODE_NONE) {
      // Старый грязный код (для сравнения)
      LONG temp = shared->counter;

      // Искусственная задержка внутри критической операции!
      // Это гарантированно заставит другие процессы прочитать старое значение.
      if (i % 100 == 0) {
        Sleep(1);
      }
      else {
        SwitchToThread();
      }

      shared->counter = temp + 1;
    }
  }
  InterlockedIncrement(&shared->workersDone);

  UnmapViewOfFile(shared);
  CloseHandle(hMap);

  return 0;
}