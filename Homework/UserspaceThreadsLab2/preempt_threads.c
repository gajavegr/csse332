/*
Basic Threads - a rudimentary userspace threads library

Author: Buffalo (hewner@rose-hulman.edu) and you!

Contrary to C convention (but for your convenience) we've documented
these functions here in the .c file rather than the header.

 */
#include <malloc.h>
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "preempt_threads.h"
#include <signal.h>

// 64kB stack
#define THREAD_STACK_SIZE 1024*64


/*
max number of threads

note if we had an expandable structure like an arraylist in C it
would be pretty easy to make this arbitrary, but let's not
introduce extra C libraries.

also note that the max we've picked is insanely small.  These threads
are very lightweight so we could probably have thousands.  But that
would make writing test code that exploits the thread max more
annoying.  So please leave this value as it is and use MAX_THREADS
(not the hardcorded value 5) in your code.
*/
#define MAX_THREADS 5

// storage for your thread data
ucontext_t threads[MAX_THREADS];
bool bools[MAX_THREADS];


// add additional constants and globals here as you need

ucontext_t child, parent;
bool child_done;
int currentThread;
int timesCalled;
bool canWrite[MAX_THREADS];
bool freedom[MAX_THREADS];


/*
initialize_basic_threads

A function that resets any globals to a brand new clean state - put
whatever you want in here.

You can rely on the fact that this function is called before any calls
to create_new_thread or schedule_threads.

Note that this function run at the beginning of each test case, so be
sure to use it to ensure your test cases don't leave data that can
effect each other.

BUT as you're starting out there's no need to agonize over what to put
in here.  As you have global data you intend to be initialized before
each test, add the code here.  As you begin, you can just leave this
blank.

 */

void catch_alarm(int blockOrDrop){
    sigset_t mask;
    sigemptyset (&mask);
    sigaddset (&mask, SIGALRM);
    if(sigprocmask(blockOrDrop, &mask, NULL) < 0) {
        perror ("sigprocmask");
    }
}


void initialize_basic_threads() {
    catch_alarm(SIG_BLOCK);
    child_done = false;
    for (int i = 0; i < MAX_THREADS; i++){
        bools[i] = false;
        canWrite[i] = true;
        freedom[i] = false;
    }
    currentThread = 0;
    timesCalled = 0;
    catch_alarm(SIG_UNBLOCK);
}

/*
create_new_thread

Gets a new thread ready to run, but does not start it.  It will be
started within schedule_threads() when it is this thread's turn (see
below).

This function takes a function pointer to the function the thread
should run when it starts.  The function provided should take no
parameters and return nothing (at least in our first iteration).

To create a new thread, memory must be allocated to store the thread's
stack.  This function should malloc that memory.

The function could fail either because the number of threads is at max
or enough memory cannot be malloc'ed.  Either way, it's fine if this
function prints and exits the program (use "exit(errorCode);" to exit
with an error code - note that 0 means no error).

Example usage:

void thread_function()
{
    // Some code
}

// elsewhere

create_new_thread(thread_function());

 */
void create_new_thread(void (*fun_ptr)()) {
    // catch_alarm(SIG_BLOCK);
    create_new_parameterized_thread(fun_ptr,NULL);
    // catch_alarm(SIG_UNBLOCK);
}
    


/*
create_new_parameterized_thread

Don't fill out this function till you get to Test 4.

This function works exactly like create_new_thread, except it expects
a function that takes a void pointer as a paramter, plus a value for
that parameter.

Example Usage:

void takesAnInt(void* val) {
    int* int_ptr = (int*) val;
    //more code
}

//elsewhere

initialize_basic_threads();
int val = 7;
create_new_parameterized_thread(takesAnInt, &val);
schedule_threads();




 */

void noFinishHelper (void (*funptr)(),void* parameter){
    catch_alarm(SIG_UNBLOCK);
    funptr(parameter);
    finish_thread();
    // catch_alarm(SIG_UNBLOCK);
}


void create_new_parameterized_thread(void (*fun_ptr)(void*), void* parameter) {
    catch_alarm(SIG_BLOCK);
    //printf("Calling create new thread on method %d\n",timesCalled);
    // signal(SIGALRM, catch_alarm);
    timesCalled++;
    // for (int j = 0; j < MAX_THREADS; j++){
    //         //printf("bools[%d] rn %d\n",j,bools[j]);
    //     }
    for (int i = 0; i < MAX_THREADS; i++){
        // catch_alarm(SIG_BLOCK);
        if (!bools[i]){
            //printf("canWrite[%d] rn %d\n",i,canWrite[i]);
            if(canWrite[i]){
                // catch_alarm();
                getcontext(&threads[i]);
                threads[i].uc_link = 0;
                threads[i].uc_stack.ss_sp = malloc( THREAD_STACK_SIZE );
                threads[i].uc_stack.ss_size = THREAD_STACK_SIZE;
                threads[i].uc_stack.ss_flags = 0;
                if ( threads[i].uc_stack.ss_sp == 0) {
                    //add condition for exceeding max threads
                    perror( "malloc: Could not allocate stack" );
                    exit( 1 );
                }
                //assign ptr to function to child thread
                void(*cast_ptr)() = (void(*)()) fun_ptr;
                makecontext(&threads[i], (void*) (noFinishHelper),2,fun_ptr,parameter);
                bools[i] = true;
                canWrite[i] = false;
                freedom[i] = true;
                // printf("set bools[%d] = %d\n",i,bools[i]);
                // printf("set canWrite[%d] = %d\n",i,canWrite[i]);
                break;
            }
        }
    }
    catch_alarm(SIG_UNBLOCK);
}


/*
schedule_threads

This function should be called once all the initial threads have been
created.  The code that calls it becomes the "master" scheduler
thread.

It should switch to one of the newly created threads.  When that
thread yields (see the next required function) control should return
to the scheduler thread.  Then the scheduler should switch to another
available thread.  The scheduler should continue to switch between
threads until every thread has run completely (i.e. not just yielded
once, but actually returned).  Once every other thread has finished,
schedule_threads() should return.

Threads in this assignment should be scheduled in the simplest
scheduling possible - "round robin".  That is, if we have 3 threads we
run 1 2 3 1 2 3 1 2 3 etc. - giving each thread an equal turn.  We'll
talk about why you might want more fancy scheduling systems later in
the course.

Example usage:

create_new_thread(thread_function1());
create_new_thread(thread_function2());
create_new_thread(thread_function3());

printf("Starting threads...");
schedule_threads()
printf("All threads finished");
*/
    //printf("scheduling!\n");


void threadYield() {
    // printf("yielding!\n");
    // swapcontext(&threads[currentThread], &parent);
    //printf("Now yielding: I am storing context into child at address %p I am swapping to context from parent at address %p\n",&child, &parent);
    // catch_alarm(SIG_BLOCK);
    swapcontext(&threads[currentThread], &parent);
    // catch_alarm(SIG_UNBLOCK);
}

void schedule_threads_with_preempt(int usecs) {
    catch_alarm(SIG_BLOCK);
    // printf("%d usecs\n",usecs);
    int buffaloCount = 0;
    signal(SIGALRM, threadYield);
    for (; currentThread < MAX_THREADS; currentThread++){
        // buffaloCount++;
        // printf("%d\n",buffaloCount);
        // printf("Current th1read: %d\n",currentThread);
        // printf("bools @ Current thread: %d\n",bools[currentThread]);
        if (bools[currentThread]){
            ualarm(usecs, 0);
            // signal(SIGALRM, catch_alarm);
            swapcontext (&parent, &threads[currentThread]);
            // signal(SIGALRM, drop_alarm);
            ualarm(usecs, 0);
            //if(buffaloCount > 3) exit(0);
            // catch_alarm(SIG_BLOCK));
        }
        if (currentThread == MAX_THREADS-1){
            currentThread = -1;
        }
        if (freedom[currentThread] && !bools[currentThread]){
            free(threads[currentThread].uc_stack.ss_sp);
            freedom[currentThread] = false;
        }
        for (int j = 0; j < MAX_THREADS; j++){
            // if (bools[j] != 0)
            // printf("bools[%d]\n",j);
        }
        if (((bools[0]||bools[1]||bools[2]||bools[3]||bools[4]) == 0)){
            catch_alarm(SIG_UNBLOCK);
            return;
        }

    }
    catch_alarm(SIG_UNBLOCK);
}
/*
void schedule_threads() {
    // printf("scheduling!\n");
    while(!child_done){
        swapcontext(&parent, &child);
    }
    //swapcontext(&child,&parent);
    //finish_thread();
    //for loop going through all threads
    printf("all done!\n");
    return;
}
*/

/*
yield

This function is called within a thread to indicate that it is ready
to allow things to switch and other threads to run (for a time).  The
threading we will write for this assignment will be non-preemptive:
threads will have to manually give control back to the scheduler by
calling yield.

Yield should use swapcontext to put the scheduler back in control and
save the current state in an appropriate u_context variable.  Later,
when the scheduler opts to run the yielding threat again swapcontext
will appear to have returned normally, the yield function itself can
return, and execution continues normally.

Note: The fact that this threading system is non-preemptive means that
when a thread is within a long-running calculation the programmer must
remember to periodically call yield or the system will appear to lock
up.  We'll handle adding preemption in a future assignment, but it
will be complicated.

Example usage:

void thread_function()
{
    for(int i = 0; i < 200; i++) {
        printf( "working\n" );
        
        // allow other threads to do some work too
        yield();
        // ok, switched back, better do some more work
    }
    printf( "done\n" );
    
    // like yield but never switches back
    finish_thread();
}

*/

//userYield!

void yield() {
    // printf("yielding!\n");
    // swapcontext(&threads[currentThread], &parent);
    //printf("Now yielding: I am storing context into child at address %p I am swapping to context from parent at address %p\n",&child, &parent);
    catch_alarm(SIG_BLOCK);
    swapcontext(&threads[currentThread], &parent);
    catch_alarm(SIG_UNBLOCK);
}



/*
finish_thread

This function works like yield but also marks things so that the
thread is marked as finished and won't be scheduled again.

Eventually, we'll figure out a way to have this function called
implicitly when the thread function returns but for simplicity in our
earily examples we just call it directly.

Note: This is not a good place to call the free corresponding to the
malloc in create_new_thread.  For the first couple tests it's OK to
let that memory leak and then we'll discuss the issue in detail.

Example usage:

void thread_function()
{
    printf("thread running\n");
    finish_thread();
    printf("If this lines prints, finish thread is broken\n");
}

*/
void finish_thread() {
    catch_alarm(SIG_BLOCK);
    bools[currentThread] = false;
    canWrite[currentThread] = true;
    //child_done = true;
    //free(child.uc_stack.ss_sp);
    // catch_alarm(SIG_BLOCK);
    // printf("finish Threads! %d \n",currentThread);
    swapcontext(&threads[currentThread], &parent);
    catch_alarm(SIG_UNBLOCK);
}
