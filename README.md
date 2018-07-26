# Node.cpp

Node.cpp is intended to facilitate (Re)Actor-based development in a way similar to Node.js, just A LOT better. 
* We tried to support as much of Node.js semantics and syntax as it was possible. In our opinion, Node.cpp does enable 
programming style which is pretty close to Node.js

Advantages of Node.cpp compared to Node.js include:
* It is C++ with static typing, templates, etc. etc. Whether to consider it a pro or con is up to you :-)
  * Optionally, it can be made _memory_safe_ C++ (a combination of local static and lightweight dynamic checks, for more details see memory-safe-cpp subproject)
* It will be made deterministic (more strictly - exhibiting at least "same-executable determinism"). 
Which in turn enables quite a few goodies:
  * We plan to support optional replay of last N seconds of program's life, just before it crashes/asserts/...
  (most of the time recording/replay should be usable in production!)
  * Determinism enables further goodies such as low-latency fault tolerance, low-latency state migration, etc. 
* Node.cpp relies on await from the very beginning, which simplifies programming A LOT compared to dreaded "lambda pyramids" (sure, Node.js also enables using await, but for Node.js await is a late addition, which has its implications)
  * You still can code "lambda pyramid" style if desired (but we hope you will use it only for migration from Node.js)
* We plan to add a system for inter-Node communications (with standardized marshalling, Node addressing etc.)
  * Load balancing is not in scope now, but it can be built on top of these inter-Node communications
* We did spend quite a bit of time on different goodies such as additional event dispatch models, ranging from Node.js-style lambdas, via OO-style virtual-function-based listeners, to template-based high-performance static dispatch.
* We intend Node.cpp to be as high-performance as possible, and a bit more. 
  * Just as one example - we already wrote our own allocator, which beats ptmalloc and tcmalloc by 1.5-2x. 

## Current Status
We have just started, so there isn't much to try. Stay tuned! 
