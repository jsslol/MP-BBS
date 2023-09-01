# MP-BBS
Multi-Process Bulletin Board System 

Specifications
(a) The shared message should be implemented as a character string (up to 255 characters) in a
shared memory.

(b) The content of the posted message should be initialized as “no one has posted a message” at
the beginning of your program (before child processes are created).

(c) The first process (which is called “the parent process”) becomes the first reader (R1) and
should create other five processes (R2, R3, R4, W1, and W2) using fork system call. Processes
that are created by the parent process are called “child processes”.

(d) Each new process should wait until all five child processes are created by the parent process
before they start doing their job (i.e., accessing the posted message).

(e) Four readers (R1, R2, R3, and R4) and two writers (W1 and W2) should be concurrently
running (multi-tasked).

(f) Up to three (not four) readers (any three in R1, R2, R3, and R4) can read the posted message
at the same time, while only one writer can post a new message at a time. When a writer is
posting (updating) the posted message, no other process can access the posted message.

(g) When a process (a reader or a writer) tries to access the posted message (i.e., when a process
tries to enter the critical section), each process should display: “R# (or W#) likes to read (or
post) the message” (see Figure 1 and 2 for an example).

(h) Right after each process enters the critical section (for accessing the posted message), each
process outputs (displays) the following message:
 “R# starts reading the message: the contents of the message”, if it is a reader and # = 1, 2,
3, or 4.

 “W# starts posting a new message: the contents of the new message”, if it is a writer and
 is 1 or 2.

(i) Right before a reader leaves its critical section, each reader outputs (displays) the following
message:

 “R# finishes reading the message”

(j) Right before a writer leaves its critical section, the writer outputs (displays) the following
message:

 “W# finishes posting a new message”

(k) Writers continue to repeat until each of them finishes repeating NUM_REPEAT times. Each
writer process terminates after that.

(l) The four readers stop reading the posted message after both writers terminate. After that,
the three child readers (R2, R3, and R4) terminate themselves.

(m)The parent process should wait until all the five child processes to terminate. After that, the
parent process delete semaphore(s) and the shared memory before it terminates itself.

(n) To control the activity timing of the readers and the writers, the following five labels should
be declared at the top of your source code. Figure 1 shows the required structure of the critical section for each reader. Figure
2 shows the required structure of the critical section for each writer.

 NUM_REPEAT: the number of times each writer updates a new message.

 READER_TIME_01: the time each reader sleeps before it reads the posted message (before
a reader starts trying to enter the critical section).

 READER_TIME_02: the time each reader sleeps after the reader finishes reading and after
displaying the posted message (but before a reader leaves the critical section).

 WRITER_TIME_01: the time each writer sleeps right before it updates a new message
(before a writer starts trying to enter the critical section).

 WRITER_TIME_02: the time each writer sleeps after it finishes updating the posted
message (but before a writer leaves the critical section).

(o) To prevent starvation against writers, no more than two readers can enter the critical section
after a writer starts trying to enter its critical section (any number of readers can read the
posted message as many times as they like as long as no writer tries to post a message).

This program is designed for the following objectives

(1) To understand the concept of process

(2) To understand the concept of inter process communication (IPC)

(3) To understand the concept of process synchronization

(4) To develop system programming skills to manage race condition and critical section

(5) To develop programming skills to avoid process starvation and deadlock

(6) To have UNIX system programming experience

(7) To be familiar with UNIX-based operating systems


Requirements
(1) No starvation should occur.

(2) No violation of mutual exclusion should occur for any process.

(3) No deadlock should occur (all the processes should be always completed)

(4) Multiple readers should be able to read the posted message (at least several times in a
program execution).

(5) No spin wait to wait for a condition to be satisfied should be used.

