```markdown
*This project has been created as part of the 42 curriculum by <azgor>*

# Codexion

## Description
Codexion is a multi-threaded C program designed to simulate a circular co-working hub where multiple coders share a central Quantum Compiler. Coders alternate between three distinct states: **compiling**, **debugging**, and **refactoring**. 

To compile quantum code, a coder must simultaneously hold two USB dongles—the one to their left and the one to their right. Because the total number of USB dongles equals the number of coders, adjacent coders compete for the same resources. Coders operate independently without inter-process communication or visibility into each other's state, and must compile regularly to avoid burning out. The simulation terminates immediately if any coder suffers a burnout due to lack of compilation.

## Instructions

### Prerequisites
* GCC or Clang compiler
* Standard POSIX threads library (`pthread`)
* GNU Make

### Compilation
To compile Codexion, run:
```bash
make

```

This produces the compiled binary `codexion` in the root directory.

Additional Makefile rules:

* `make clean`: Removes object files.
* `make fclean`: Removes object files and the executable.
* `make re`: Recompiles the entire project.

### Execution

Run the executable with the required arguments representing timing and simulation parameters:

```bash
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <number_of_compiles_required> <dongle_cooldown> <scheduler>
```

*Example:*

```bash
./codexion 5 800 200 200 200 4 100 fifo

```

## Blocking Cases Handled

* **Deadlock Prevention & Coffman Conditions:**
* *Resource Hierarchy (Lock Ordering):* To eliminate the circular wait condition, coders acquire dongles based on an absolute index order (e.g., taking `min(left, right)` first, then `max(left, right)`). This prevents the classic dining philosophers deadlock where every coder grabs their left dongle and waits indefinitely for the right.


* **Starvation Prevention:** A fair resource allocation strategy prevents greedy coders from repeatedly acquiring adjacent dongles, ensuring every thread gets equal access to compile before approaching its burnout limit.
* **Cooldown Handling:** Coders strictly adhere to configured state durations (`time_to_compile`, `time_to_debug`, `time_to_refactor`), releasing resources immediately after compilation and yielding execution time so adjacent coders can acquire the dongles.
* **Precise Burnout Detection:** A dedicated monitoring loop continually checks the timestamp of each coder's last compilation against `time_to_burnout` using high-precision time tracking (`gettimeofday`). If a coder exceeds their limit, the monitor logs the burnout and triggers a global shutdown instantly.
* **Log Serialization:** All state changes (`has taken a dongle`, `is compiling`, `is debugging`, `is refactoring`, `burned out`) are protected by a centralized logging mutex. Output lines are written atomically with accurate timestamps, preventing interleaved text or post-death logging.

## Thread Synchronization Mechanisms

### Threading Primitives

* **`pthread_mutex_t`:**
* *Dongle Mutexes:* Each USB dongle on the table is represented by a dedicated mutex to ensure exclusive access.
* *Logging Mutex:* Serializes standard output so concurrent thread logs do not interleave.
* *State Mutexes:* Guard individual coder status flags, such as the timestamp of the last meal/compilation and simulation end flags.

* **`pthread_cond_t`:** Used in combination with state mutexes to signal state changes and facilitate efficient sleeping without high CPU usage or busy-waiting.
* **Custom Event Implementation:** A synchronized state-checking mechanism where the monitor thread safely polls coder states or listens for state broadcast signals to detect simulation termination criteria (all coders compiled $N$ times or a burnout occurred).

### Resource Coordination & Race Condition Prevention

* **Dongle Allocation Strategy:** Before changing state to compiling, a coder must lock both adjacent dongle mutexes in a pre-determined order. If both cannot be secured safely, the thread sleeps until signaled, avoiding data races over dongle states.
* **Monitor State Inspection:** The monitor thread reads shared Coder metadata (e.g., `last_compile_time`) through thread-safe getter functions wrapped in mutex locks to prevent read/write races during concurrent updates by worker threads.
* **Thread-Safe Communication:** Inter-thread signals (such as stopping the simulation) are managed via atomic state updates protected by a central control mutex, guaranteeing that all coder threads notice termination flags promptly and exit cleanly.

## Resources

* **POSIX Threads (pthreads) Documentation:** [man7.org - pthreads(7)](https://man7.org/linux/man-pages/man7/pthreads.7.html)
* **The Dining Philosophers Problem:** Dijkstra, E. W., *Hierarchical ordering of sequential processes* (Foundational resource for resource allocation and deadlock avoidance).
* **Coffman Conditions:** Operating Systems literature covering conditions required for system deadlocks (Mutual Exclusion, Hold and Wait, No Preemption, Circular Wait).

### Use of AI

* **Task Identification & Scaffolding:** AI tools were used to outline edge-case scenarios involving resource contention and lock ordering strategies for circular seating layouts.
* **Documentation & Formatting:** AI was used to help draft and structure this `README.md` file in compliance with the required specification.
* **Code Review & Debugging Assistance:** AI was consulted to analyze log traces, verify deadlock-free properties of asymmetric lock acquisition routines, and validate timing precision for high-concurrency loops.

```

```
