## General info
ASCII turn based game written upon client-server architecture. Main goal of making it was to learn about C IPC mechanisms through practice.

Game's objective is to collect coins scattered throughout the map while avoiding obstacles e.g. other players or monsters.
Each turn takes about 1 second, before it's resolved players declare actions they want to perform.
Most of the communication between server and clients revolves reading and writing to multiple shared memory files, some of them contain semaphores for synchronization between processes. 

Written as a part of Operating Systems course at my university.
