# sched
### Fast, lightweight, simple, cooperative multitasking

## Getting Started:
### On Arduino:
* include "sched.h"
* Create a scheduler and one or more tasks
* Start the scheduler
* Call the scheduler's loop() function as the only content of your main loop function
### On other platforms:
* (optional) typedef NOW_T - a unitless measure of time. defaults to long long
* You must define two functions: 
**  void SLEEP(NOW_T time) -- takes the number of time units to sleep
**  NOW_T NOW() -- returns a NOW_T representing the current time

Sched is time-unit-agnostic, as long as your SLEEP and NOW functions share a 
timebase. The maximum interval is limited by the number of bits in your NOW_T
and time-per-bit of your NOW() function.

### Implemented Features:
1. Periodic execution of tasks
2. Run each task once, a set number of times, or forever
3. Runtime metrics per scheduler and per task
5. Dynamically add/delete tasks
6. Dynamically deactivate/reactivate tasks
7. Reference tasks by pointer, taskID, or pointer to callback function
8. Anonymous callback functions

### Future Features:
1. Multiple instances of each task using context structs
2. Dynamically modify tasks including intervals, remaining iterations, and callbacks


