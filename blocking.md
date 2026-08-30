# Why codexion blocks

The program hangs due to a real deadlock. Confirmed with gdb: one coder stuck in
`take_dongles`, another in `cooldown_dongles`, both blocked on mutexes the other
holds.

## Root causes (in order of severity)

1. **Lock-order inversion (ABBA deadlock)**
   - `take_dongles` locks `dongles[i+1]` then `dongles[i]` (scheduler.c:199-200).
   - `cooldown_dongles` / `free_dongle` lock `dongles[i]` then `dongles[i+1]`
     (scheduler.c:181-182, 250-251).
   - Coder *i* and coder *i+1* share a dongle, so two threads can each hold one
     lock and wait for the other's.
   - `release_dongles` spawns a detached `free_dongle` thread on every release
     (scheduler.c:273) and never joins them, so multiple free threads for the
     same/adjacent dongles pile up and race with the coder's next `take_dongles`,
     making the deadlock practically certain.

2. **Busy-wait scheduler starves everything**
   - `start_scheduler` is `while (1)` with no `usleep`/yield (scheduler.c:281-287).
   - Each `fifo_scheduler` pass calls `is_coder_turn` -> `get_coder_dongles`,
     which malloc's every iteration (scheduler.c:10).
   - Observed flooding `signaling cond for 1` thousands of times per second,
     hogging the CPU so coder threads barely run.

3. **Recursive mutex + `pthread_cond_timedwait` is undefined behavior**
   - Mutexes are created `PTHREAD_MUTEX_RECURSIVE` (init_codex.c:57) then used as
     the cond mutex (scheduler.c:223). POSIX forbids this; on glibc the wait may
     never properly release the mutex, so waiters can hold it forever.

4. **Data races**
   - Queue scanned without `queue_lock` (scheduler.c:105).
   - Dongle `state` read unlocked in `is_dongles_free` (scheduler.c:67).
   - `rc` used uninitialized in `wait_for_dongles` (scheduler.c:210).

## Fix priorities

- Use one consistent dongle lock order everywhere (e.g. always lowest index first).
- Use normal (non-recursive) mutexes for the condition variable.
- Add `usleep` / `sched_yield` in the scheduler loop.
