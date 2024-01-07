You and a chosen partner will implement a zero-copy ring buffer based on the "magic buffer" idea where the buffer is mapped into a process's address space twice.

This assignment has a user mode component and a kernel mode component.

This assignment document is intended to be a loose specification. Some things will not be specified tightly, in which case of course you are free to make your own decisions. You can also change the specification somewhat, but you must abide by these hard requirements:

ring buffer read and write operations are zero copy (they must not copy into any intermediate buffer; a process should write bytes directly to the buffer, and another process should directly read those bytes);
the kernel is never involved except to setup and tear down ring buffers;
the ring buffer is "magic"; that is, it is mapped twice into the user address space to avoid awkward fragmentation problems when the area the user is reading from or writing to wraps around the end of the ring buffer. As a result, your code should always be able to copy in/copy out data from the buffer in one memmove call.
