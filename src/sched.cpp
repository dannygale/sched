#include <string>
#include <ctime>

#include "sched.h"


//
// Constructor 
//
Sched::Sched() : 
    nextTaskID(1), 
    _t_run(0), 
    _t_sleep(0), 
    _t_start(0)
{  }

//
// === Public Methods 
//

void Sched::loop() {
    Task *cursor, *previous, *temp, *t;

    //std::cout << "loop\n" << //std::flush;
    if (running && nextTask && nextTask->enabled) {
        //std::cout << "have a task and running\n" << //std::flush;
        NOW_T now = NOW();

        if (nextTask->nextRun > now) {
            //printTaskList();
            NOW_T sleep = nextTask->nextRun - now;
            //std::cout << "next: task" << nextTask->id << " in " << sleep << "us at " << nextTask->nextRun << ". now: " << now << "\n" << //std::flush;
            SLEEP(sleep);
            //std::cout << "end sleep\n" << //std::flush;
            _t_sleep += sleep;
        }

        //std::cout << "running task " << nextTask->id << "\n" << //std::flush;

        // run it
        t = nextTask;

        NOW_T run_start = NOW();

        t->run();

        NOW_T run_end = NOW();
        NOW_T elapsed = run_end >= run_start ? 
            run_end - run_start : 
            ((NOW_T)(-1) - run_end) + run_start;

        _t_run += elapsed;
        t->_total_runtime += elapsed;

        // advance nextTask
        nextTask = nextTask->next;

        // schedule the next run if necessary
        if (t->moreTimes == 1) {
            // this was the last run
            //std::cout << "deleting task " << nextTask->id << "\n" << //std::flush;
            temp = t->next;
            delete t;
            nextTask = temp;
        } else {
            // if this is a not-forever task, decrement moreTimes
            if (t->moreTimes) t->moreTimes--;

            // calculate and schedule next run time
            t->nextRun += t->period;
            // figure out when this task needs to run again
            scheduleNextRun(t);
        }
    } else {
        running = false;
    }
}

void Sched::start() {
    Task *cursor = nextTask, *previous = NULL, *next = NULL;
    
    NOW_T now = NOW();
    _t_start = now;
    running = true;

    // set nextRun for each task = now+period
    // re-order accordingly
    while (cursor) {
        cursor->nextRun = now + cursor->period;
        next = cursor->next;

        // if cursor doesn't need to run until after cursor-> next, swap their positions
        // repeat until the task at cursor is in the right place
        while (cursor->next && cursor->period > cursor->next->period) {
            previous->next = cursor->next;
            cursor->next = cursor->next->next;
            previous->next = cursor;
        }

        previous = cursor;
        cursor = next;
    }
}

void Sched::stop() {
    running = false;
}

void Sched::restart() {
    stop();
    start();
}

void Sched::reset() {
    Task *cursor = nextTask, *next;
    running = false;

    while(cursor) {
        next = cursor->next;
        delete cursor;
        cursor = next;
    }
}

TaskID Sched::addTask(Task *t) {
    if (!t->id) {
        t->id = this->nextTaskID++;
    }
    //std::cout << "taskID: " << t->id << "\n" << //std::flush;

    t->nextRun = NOW();

    scheduleNextRun(t);
    return t->id;
}

TaskID Sched::addTask(int times, int period, void (*func)()) {
    //std::cout << "creating task\n" << //std::flush;
    Task *t = new Task(this, times, period, func);
    return t->id;
}

bool Sched::taskEnable(Task* t) {
    t->enabled = true;
    scheduleNextRun(t);
    return true;
}

bool Sched::taskEnable(TaskID id) {
    Task* t = nextTask;
    while(t) {
        if (t->id == id) {
            scheduleNextRun(t);
            return true;
        }
    }
    return false;
}

bool Sched::taskEnable(void (*func)()) {
    bool atLeastOne = false;
    Task* t = nextTask;
    while(t) {
        if (t->func == func) {
            atLeastOne = true;
            scheduleNextRun(t);
        }
    }
    return atLeastOne;
}

bool Sched::taskDisable(Task* t) {
    t->enabled = false;
    t->nextRun = -1ul;
    scheduleNextRun(t);
    return true;
}

bool Sched::taskDisable(TaskID id) {
    Task *t = nextTask;
    while(t) {
        if (t->id == id) {
            return taskDisable(t);
        }
    }
    return false;
}

bool Sched::taskDisable(void (*func)()) {
    bool atLeastOne = false;
    Task *t = nextTask;
    while(t) {
        if (t->func == func) {
            atLeastOne = true;
            taskDisable(t);
        }
    }
    return atLeastOne;
}


//
// Private methods
//

void Sched::scheduleNextRun(Task* t) {
    //std::cout << "scheduling run\n" << //std::flush;
    Task *cursor = nextTask, *previous = NULL, *temp = NULL, *result = NULL;
    
    if (!nextTask || t->nextRun < nextTask->nextRun) {
        // special handling if t needs to run at the head of the list
        t->next = nextTask;
        nextTask = t;
    } else {
        // start t at the beginning and bubble it through the list to find the 
        // right spot for the next run
        cursor = nextTask;
        while (cursor->next && (cursor->next->nextRun < t->nextRun)) { cursor = cursor->next; }
        // now cursor is the node after which to insert t
        t->next = cursor->next;
        cursor->next = t;
    }
}

void Sched::printTaskList() {
    Task *t = this->nextTask;
    NOW_T now = NOW();

    NOW_T total_elapsed = now - _t_start;
    //std::cout << "Total time: " << total_elapsed << "\n" << flush;
    //std::cout << "Sleep time: " << _t_sleep << "\n" << flush;
    //std::cout << "Run   time: " << _t_run << "\n" << flush;
    //std::cout << "Sched time: " << total_elapsed - _t_run - _t_sleep;

    //std::cout << "\nTask list:\n" << //std::flush;
    long long in = 0;
    while (t) {
        in = t->nextRun - now;
        //std::cout << "\ttask" << t->id << " in " << in << "us at " << t->nextRun << ". now: " << now <<  ", next: " << t->next << "\n" << //std::flush;
        t = t->next;
    }
}

Task::Task(Sched *sched, unsigned long moreTimes, NOW_T period, void (*func)()) :
    func(func), moreTimes(moreTimes), period(period), id(0), next(NULL), _total_runtime(0), enabled(true)
{
    sched->addTask(this);
}

void Task::run() {
    this->func();
}
