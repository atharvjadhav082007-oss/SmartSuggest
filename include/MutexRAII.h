#ifndef MUTEXRAII_H
#define MUTEXRAII_H

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#else
    #include <pthread.h>
#endif

/**
 * @class Mutex
 * @brief Cross-platform Mutex primitive following strict RAII.
 * 
 * Works cleanly on all toolchains including win32-thread model MinGW without
 * requiring POSIX pthread emulation or external libraries.
 */
class Mutex {
private:
#ifdef _WIN32
    CRITICAL_SECTION cs;
#else
    pthread_mutex_t mtx;
#endif

    // Prevent copying (RAII principle)
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

public:
    Mutex() {
#ifdef _WIN32
        InitializeCriticalSection(&cs);
#else
        pthread_mutex_init(&mtx, nullptr);
#endif
    }

    ~Mutex() {
#ifdef _WIN32
        DeleteCriticalSection(&cs);
#else
        pthread_mutex_destroy(&mtx);
#endif
    }

    void lock() {
#ifdef _WIN32
        EnterCriticalSection(&cs);
#else
        pthread_mutex_lock(&mtx);
#endif
    }

    void unlock() {
#ifdef _WIN32
        LeaveCriticalSection(&cs);
#else
        pthread_mutex_unlock(&mtx);
#endif
    }
};

/**
 * @class LockGuard
 * @brief Scoped RAII lock guard for Mutex.
 */
class LockGuard {
private:
    Mutex& mutex;

    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;

public:
    explicit LockGuard(Mutex& m) : mutex(m) {
        mutex.lock();
    }

    ~LockGuard() {
        mutex.unlock();
    }
};

#endif // MUTEXRAII_H
