# Threadpool

A header only library implementing a templated thread-pool.

## Objective

I made this as a facility for Audaspace's convolution system, but I figured it'd be useful to have a standalone version.
This class is fairly straightforward to use, just enqueue tasks like you'd do with a standard C++ std::thread and they'll be assigned to the pooled threads following a FIFO ordering.
It also allows stopping, restarting, and resizing of the pool, with one caveat: Running tasks must finish before doing this. Users should evaluate if they want to implement a stop-request mechanism in their tasks in order to improve response times with these operations.
