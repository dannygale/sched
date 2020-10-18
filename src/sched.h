#ifndef __SCHED_H__
#define __SCHED_H__

// === BASIC DEFINITIONS ===

#ifndef NULL
#define NULL 0
#endif

// sleep functions
#ifndef SCHED_SLEEP
// #define SCHED_SLEEP LowPower.deepSleep
//#define SCHED_SLEEP LowPower.sleep
#include <unistd.h>
#define SCHED_SLEEP usleep
// #define SCHED_SLEEP LowPower.idle
#endif

#ifndef NOW_T 
#define NOW_T long long
#endif

#ifndef NOW
//#define NOW Millis
using namespace std;
NOW_T NOW() {
    return std::chrono::duration_cast<std::chrono::microseconds>
              (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

// ONLY TO GET YCM TO STOP COMPLAINING
//NOW_T Millis() {return 0;};
#endif

#define SCHED_FOREVER 0
#define SCHED_ONCE 1

// === TYPES ===

typedef unsigned TaskID;


class Task;
class Sched;

// === BEHAVIOR CONFIGURATION ===
//#define SCHED_METRICS
//#define SCHED_TASK_DATA


#ifdef SCHED_METRICS
NOW_T _start, _end, _elapsed;
#define SCHED_NOW_START() { NOW_T _start = NOW(); }
#define SCHED_NOW_END { NOW_T _end = NOW(); NOW_T _elapsed = _end - _start; }
#endif

// === CLASS DEFINITIONS ===

class Task {
    friend class Sched;
    public:
        void (*func)();
        unsigned long moreTimes;
        unsigned long nextRun;
        unsigned long period;
        unsigned id;

        Task(Sched *sched, unsigned long times, unsigned long period, void (*func)());

        void run();
    private:
        Task * next;
        NOW_T _t_run;
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

        NOW_T t_total();
        NOW_T t_run();
        NOW_T t_sleep();
        NOW_T t_overhead();
    private:
        bool running;
        unsigned taskCount;
        TaskID nextTaskID;

        NOW_T _t_run;
        NOW_T _t_sleep;
        NOW_T _t_start;

        void scheduleNextRun(Task *t);
        void printTaskList();
};

#endif // __SCHED_H__
