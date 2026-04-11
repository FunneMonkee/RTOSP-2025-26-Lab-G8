Completions are a synchronization mechanism
- A completion is a wait for completion barrier
- If you have one or more threads that must wait for some kernel activity to have reached a point or a specific state, completions can provide a race-free solution to this problem
- Completions are built on top of the waitqueue and wakeup infrastructure of the Linux scheduler.

for b atomic inc and dec are not needed since completions are used, so we enfore a proper order of operations
