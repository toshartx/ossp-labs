# Архитектура приложения (Вариант 23)

## 1. Участники системы
Система состоит из одного главного процесса **Manager** и \(N\) дочерних процессов **Worker**.
*   **Manager**: Инициализирует ресурсы, запускает воркеров, ожидает их завершения, проверяет инварианты.
*   **Worker**: Подключается к общим ресурсам, выполняет цикл инкрементов, логирует состояния.

## 2. Общие ресурсы (Shared Resources)
Между процессами используются следующие системные объекты Windows:
1.  **Разделяемая память (Shared Memory)**:
    *   Объект: `File Mapping` с именем `Global\Variant23_SharedMem`.
    *   Содержимое: Структура `SharedData` (счетчик, ожидаемое значение, время старта).
    *   Доступ: `MapViewOfFile` в обоих процессах.
2.  **Мьютекс (Mutex)**:
    *   Объект: Именованный мьютекс `Global\Variant23_Mutex`.
    *   Назначение: Защита критической секции при `counter++` в режиме синхронизации Mutex.
3.  **Журнал (Log File)**:
    *   Объект: Файл `log.txt`.
    *   Назначение: Запись событий (START, FINISHED) от всех процессов.

## 3. Схема взаимодействия

В mermaid: 
```
sequenceDiagram
    participant M as Manager
    participant SM as Shared Memory
    participant Mutex as Mutex
    participant W1 as Worker 1
    participant W2 as Worker 2
    participant W3 as Worker 3

    M->>SM: CreateFileMapping / MapViewOfFile
    M->>SM: Инициализация counter = 0
    M->>Mutex: CreateMutex

    M->>W1: CreateProcess
    M->>W2: CreateProcess
    M->>W3: CreateProcess

    W1->>SM: OpenFileMapping / MapView
    W2->>SM: OpenFileMapping / MapView
    W3->>SM: OpenFileMapping / MapView

    par Параллельная работа
        loop 50 000 раз
            W1->>Mutex: WaitForSingleObject
            W1->>SM: counter++
            W1->>Mutex: ReleaseMutex
        end
    and
        loop 50 000 раз
            W2->>Mutex: WaitForSingleObject
            W2->>SM: counter++
            W2->>Mutex: ReleaseMutex
        end
    and
        loop 50 000 раз
            W3->>Mutex: WaitForSingleObject
            W3->>SM: counter++
            W3->>Mutex: ReleaseMutex
        end
    end

    W1-->>M: Завершение процесса
    W2-->>M: Завершение процесса
    W3-->>M: Завершение процесса

    M->>SM: Чтение counter (проверка инварианта)
    M->>SM: UnmapViewOfFile
    M->>Mutex: CloseHandle
```

## 4. Состояния участников (для журнала)
В соответствии с п. 4.3 требований, воркеры должны выводить свои состояния:

- START — воркер подключился к памяти и начал работу.
- WORKING — выполняется цикл инкрементов (опционально, чтобы не перегружать лог).
- WAITING — воркер ожидает освобождения мьютекса (в режиме Mutex).
- FINISHED — воркер завершил цикл и отключился.