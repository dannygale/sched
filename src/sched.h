#ifndef __SCHED_H__
#define __SCHED_H__

// === BASIC DEFINITIONS ===

#ifndef NULL
#define NULL 0
#endif

#define SCHED_FOREVER 0
#define SCHED_ONCE 1

// === TYPES ===
#ifndef NOW_T
typedef unsigned long long NOW_T;
#endif

#ifndef NULL
#define NULL 0
#endif

// sleep functions
#ifndef SLEEP
// #define SLEEP LowPower.deepSleep
//#define SLEEP LowPower.sleep
#include <unistd.h>
#define SLEEP usleep
// #define SLEEP LowPower.idle
#endif

#ifndef NOW_T 
typedef unsigned long long NOW_T;
#endif

#ifndef NOW
//#define NOW Millis

#define USE_STDLIB
#ifdef USE_STDLIB
using namespace std;
#include <iostream>
#define DEBUG(...) { cout << __VA_ARGS__ <<  flush }
#else
#define DEBUG(...)
#endif
using namespace std;
NOW_T NOW() {
    return std::chrono::duration_cast<std::chrono::microseconds>
              (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}
#endif

typedef unsigned TaskID;

class Task;
class Sched;

// === CLASS DEFINITIONS ===

class Task {
    friend class Sched;
    public:
        void (*func)();
        unsigned long moreTimes;
        unsigned long nextRun;
        unsigned long period;
        unsigned id;
        bool enabled;

        Task(Sched *sched, unsigned long times, NOW_T period, void (*func)());

        void run();

        NOW_T t_total() { return _total_runtime; }
        unsigned t_invocations() { return _invocations; }
        NOW_T t_avgRun() { return _total_runtime / (NOW_T)_invocations; }
    private:
        Task * next;
        
        NOW_T _total_runtime;
        unsigned _invocations;
};

class Sched {
    friend class Task;

    public:
        Task* nextTask;

        Sched();

        void loop();

        void start();
        void stop();
        void restart();
        void reset();
        TaskID addTask(Task* t);
        TaskID addTask(int times, int period, void (*func)());

        bool taskEnable(Task *t);
        bool taskEnable(TaskID id);
        bool taskEnable(void (*func)());

        bool taskDisable(Task *t);
        bool taskDisable(TaskID id);
        bool taskDisable(void (*func)());

        bool taskDelete(Task *t);
        bool taskDelete(TaskID id);
        bool taskDelete(void (*func)());

        NOW_T t_total() { return _t_run + _t_sleep + elapsed(); }
        NOW_T t_run() { return _t_run; }
        NOW_T t_sleep() { return _t_sleep; }
        NOW_T t_overhead() { return elapsed() - _t_sleep - _t_run; }

    private:
        bool running;
        unsigned _taskCount;
        unsigned _activeTaskCount;
        TaskID nextTaskID;

        NOW_T _t_run;
        NOW_T _t_sleep;
        NOW_T _t_start;
        NOW_T elapsed() { 
            NOW_T now = NOW();
            return now <= _t_start ? now - _t_start : ((NOW_T)(-1) - _t_start) + now;
        }

        void scheduleNextRun(Task *t);
        void printTaskList();
};

#endif // __SCHED_H__
