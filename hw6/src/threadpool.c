#include "threadpool.h"

pthread_mutex_t poolLock = PTHREAD_MUTEX_INITIALIZER;
pool_t *poolsQ = NULL;

uint16_t* readConfigFile(char* filename){
    
    // make some space for an array of 16 bit ints cus that makes sense
    uint16_t* configSettings = calloc(4, sizeof(uint16_t));
    
    // Do we have a valid file?
    bool fileValid = isFileValid(filename);
    if(!fileValid){
        // No, then bye bye!
        printf("File Not Found");
        exit(EXIT_FAILURE);
    }
 
    char* line = NULL;
    size_t len = 0;
    FILE* config = fopen(filename, "r");
    
    int count = 0;
    errno = 0;
    while ((getline(&line, &len, config)) != -1) {
        // read lines and do error checking each time
        configSettings[count] = strtoul(line, NULL, 10);
        if(errno != 0){
            perror("Bad file format");
            exit(EXIT_FAILURE);
        }
        count++;
        if(count == 4)break;
    }
    
    // Less than 4 values
    if(count != 4){
        fprintf(stderr, "Less than 4 values");
        exit(EXIT_FAILURE);
    }
    
    // min is not greater than or equal to 1
    if( !(configSettings[0] >= 1) ){
        fprintf(stderr, "Minimum Thread Pool Wrong");
        exit(EXIT_FAILURE);
    }
    
    // max is not greater than min
    if( !(configSettings[1] >= configSettings[0]) ){
        fprintf(stderr, "Maximum Thread Pool Wrong");
        exit(EXIT_FAILURE);
    }
    
    // linger is not greater than 0
    if( !(configSettings[3] >= 0) ){
        fprintf(stderr, "Linger Thread Pool Wrong");
        exit(EXIT_FAILURE);
    }
    
    // close the file
    fclose(config);
    
    // give em the array of [min, max, queueSize, linger]
    return configSettings;
}

pool_t* pool_create(uint16_t min, uint16_t max,
                    uint16_t linger, pthread_attr_t* attr){
    
    pool_t* pool = calloc(1, sizeof(pool_t));
    // BRAND NEW POOL. LET'S GO SWIMMING
    
    // More initializing 
    Mutex_init(&pool->mutex); // went to the smithy got a new lock  
    Cond_init(&pool->busy); // the busy condition, so fresh!
    Cond_init(&pool->work); // the work condition, ripe for the picking
    Cond_init(&pool->wait); // the wait condition, NEWNEWNEW
    
    // Set the stuff from the config file
    pool->pool_minimum = min; 
    pool->pool_maximum = max; 
    pool->pool_linger = linger;
    
    // We can't rely on this function's stack to be here forever
    // copt the attr argument
    copyThreadAttr(&pool->pool_attr, attr);
    
    // LOCK IT UP
    Lock(&poolLock);
    
    // Insertion party
    if (poolsQ == NULL) {
        // if nothing is in here
        // we set forward and reverse to point to the single item
        // it's nice to be an only child, so much attention
        // aka so many pointers point at you
        pool->forward = pool;
        pool->backward = pool;
        poolsQ = pool;
    } else {
        // If you have siblings then we insert you in the queue
        // you're at the end to begin with tho
        poolsQ->backward->forward = pool;
        pool->forward = poolsQ;
        pool->backward = poolsQ->backward;
        poolsQ->backward = pool;
    }
    // if processes get spawned as children then threads must be siblings 
    // just thinking out loud
    // Anyways, be free my data!
    Unlock(&poolLock);
    
    // here is your pool, pls take it
    return pool;
}

void copyThreadAttr(pthread_attr_t *copy, pthread_attr_t *orig) {
    struct sched_param param;
    void *addr;
    size_t size;
    int value;

    pthread_attr_init(copy);

    if (orig != NULL) {
        // Just copy every aspect of the thread attr
        // No pick and choose
        
        pthread_attr_getstack(orig, &addr, &size);
        // don't allow a non-NULL thread stack address 
        pthread_attr_setstack(copy, NULL, size);

        pthread_attr_getscope(orig, &value);
        pthread_attr_setscope(copy, value);

        pthread_attr_getinheritsched(orig, &value);
        pthread_attr_setinheritsched(copy, value);

        pthread_attr_getschedpolicy(orig, &value);
        pthread_attr_setschedpolicy(copy, value);

        pthread_attr_getschedparam(orig, &param);
        pthread_attr_setschedparam(copy, &param);

        pthread_attr_getguardsize(orig, &size);
        pthread_attr_setguardsize(copy, size);
    }

    // make all pool threads be detached threads 
    // why?? becuase we want our resources back someday 
    // but we are too lazy to do it ourselves 
    // we'd need some garbage collector thread 
    // go use java if that's what you want
    pthread_attr_setdetachstate(copy, PTHREAD_CREATE_DETACHED);
}

int pool_queue(pool_t* pool, void* (*func)(void *), void* arg){
    job_t *job;
    // new job pointer.
    
    // Let's error check calloc because who knows could fail
    // and it doesn't do its own error checking I should've known that earlier 
    // but here its important.
    if ((job = calloc(1, sizeof (*job))) == NULL) {
        errno = ENOMEM;
        return -1;
        // Everything failed
    }
    
    job->job_next = NULL;
    job->job_func = func;
    job->job_arg = arg;

    // LOCK IT DOWN MAN!
    // THINGS ARE TURBULANT-CONCURRENT  
    Lock(&pool->mutex);// we use the pool's mutex to keep data safe

    
    if (pool->pool_head == NULL){
        // If no head then we set one!
        pool->pool_head = job;
        
    }else{
        // otherwise stick it on the end
        pool->pool_tail->job_next = job;
    }
    // tail updated to point to the last insertion 
    pool->pool_tail = job;

    
    if (pool->pool_idle > 0){
        // Oh there's some lazy idle threads hanging around
        // You want a hot body? You want a Bugatti?
        // You want a Maserati? You betta work b*tch
        pthread_cond_signal(&pool->work);
    }else if (pool->pool_nthreads < pool->pool_maximum && createThread(pool)){
        // If there is less threads than max and thread was created  
        // add one to the total thread number
        pool->pool_nthreads++;
    }
    
    // STORM HAS PASSED I FREE YOU MY LITTLE DATA BABIES 
    Unlock(&pool->mutex);
    return 0;
}

void *thread(void *arg) {
    pool_t *pool = (pool_t *) arg;
    bool timeout;
    job_t *job;
    void *(*func)(void *);
    active_t active;
    struct timespec ts;
    sigset_t fillset;

    sigfillset(&fillset);    
    
    /*
     * This is the worker's main loop.  It will only be left
     * if a timeout occurs or if the pool is being destroyed.
     */
    Lock(&pool->mutex);
    pthread_cleanup_push((void*)threadCleanup,(void*) pool);
    active.active_tid = pthread_self();
    
    bool everythingIsFine = true;
    while(everythingIsFine) {
        
        /*
         * Reset all values for the given thread 
         * 
         * We need to make it reusable 
         */
        
        pthread_sigmask(SIG_SETMASK, &fillset, NULL); // Set mask for thread
        
        // Set cancel type and state.
        pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL); 
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

        // Timeout initialize
        timeout = false;
        
        // We now have a new idle thread 
        pool->pool_idle++; 
        
        // If we have some flags to handle like waiting or destruction
        if (pool->pool_flags & POOL_WAIT){
            update(pool); // Set proper flags
        }
        // So while we have no initial job and we are not destroying the pool
        while (pool->pool_head == NULL && !(pool->pool_flags & POOL_DESTROY)) {
            
            // If there's less or equal minimum threads 
            if (pool->pool_nthreads <= pool->pool_minimum) {
                
                // Set the work condition to wait
                pthread_cond_wait(&pool->work, &pool->mutex);
            
            } else { // If there is more than the minimum threads
                
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_sec += pool->pool_linger; // add the current time to the linger
                
                if (pool->pool_linger == 0 || pthread_cond_timedwait(&pool->work, &pool->mutex, &ts) == ETIMEDOUT) {
                    // If the linger is 0 or the wait condition returns an error then signal timeout
                    timeout = true;
                    break;
                    
                }
                
            }
        }
        
        // Thread is no longer idle (we are starting it up now)
        pool->pool_idle--;
        
        if (pool->pool_flags & POOL_DESTROY) // If the flag is set to destroy
            break; // everything is not fine
        if ((job = pool->pool_head) != NULL) {
            
            timeout = false;
            // let's get the function to call and its args from the current job
            func = job->job_func;
            arg = job->job_arg;
            // move pointer to next job
            pool->pool_head = job->job_next;
            
            // make sure the tail is null so our loop will end and no segfault
            if (job == pool->pool_tail){
                // Paul's Xeno Name will never be!!
                pool->pool_tail = NULL;
            }
            
            // Linked list of active threads (it's just their ids)
            active.active_next = pool->active;
            
            // points to current active thread id
            pool->active = &active;
            
            // We are done changing the pool data some other thread can have at it
            Unlock(&pool->mutex);
            
            // using a nice api thread stack can keep track of our clean up
            // the idea here is we can push a threadCleanup to the bottom
            // and while we run jobs we push and (spoiler) pop cleanup for each job
            // when we've finished all our jobs we can now expect there to be a thread clean up 
            // waiting for us at the bottom of the stack, that is so nice. 
            pthread_cleanup_push((void*)jobCleanup, (void*)pool);
            
            // Waht??? free something?? ME??? Noooooo neverrrrrr
            free(job);
            
            /*
             * Call the function queued 
             */
            
            // This is where the magic happens 
            func(arg);//like this all we are trying to do
            // nothing else matters
            
            /*
             * If the queued function calls Thread_exit(), the thread
             * calls jobCleanup(pool) and threadCleanup(pool);
             * the integrity of the pool is thereby maintained.
             */
            
            // sorry I spoiled it earlier but yeah clean function ready to go
            pthread_cleanup_pop(1); /* jobCleanup(pool) */
            
        }
        if (timeout && pool->pool_nthreads > pool->pool_minimum) {
            /*
             * TIMEOUT!
             * 
             * number of threads exceed the minimum pool size
             * so lets downsize to save some resources
             */
            break;
        }
    }
    
    // The last pop, all is well thread is cleaned up
    pthread_cleanup_pop(1); /* threadCleanup(pool) */
    
    return NULL;
}

bool createThread(pool_t *pool) {
    sigset_t saveSet;
    int ret;
    sigset_t fillset;
    //pthread_t id;
    
    
    sigfillset(&fillset);
    
    pthread_sigmask(SIG_SETMASK, &fillset, &saveSet);//////////////
    // Set the sig for the thread wee are operating in
    // New thread will inherit the mask 
    
    ret = Thread_create(NULL, &pool->pool_attr, &thread, pool);
       
    // undo everything for our current thread
    pthread_sigmask(SIG_SETMASK, &saveSet, NULL);//////////////////
    
    // Return whether or not this worked
    // hopefully thread was created no problem
    return ret?false:true;
}

void threadCleanup(pool_t *pool) {
   
    --pool->pool_nthreads;
    // We now have one less thread
    
    // Are we destroying the pool?
    if (pool->pool_flags & POOL_DESTROY) {
        // Ye.
        
        if (pool->pool_nthreads == 0){
            // If any threads are blocked on busy then we free them from their 
            // mortal coil, jk just make em not hang no more
            pthread_cond_broadcast(&pool->busy);
            
        }
        
    } else if (pool->pool_head != NULL && pool->pool_nthreads < pool->pool_maximum && createThread(pool) == 0) {
        
        // if we aren't destroying the thread pool 
        // and there isn't more threads than the max
        // lets make a new one
        pool->pool_nthreads++;
        
    }
    
    // Unlock the mutex just incase it hasn't been already
    Unlock(&pool->mutex);
    
}

void pool_wait(pool_t *pool){
    
    // No messing with my data 
    Lock(&pool->mutex);
    
    // push to the cleanup stack an unlock call
    pthread_cleanup_push((void*)Unlock, (void*) &pool->mutex);
    
    // while head is not null or active threads exist
    while (pool->pool_head != NULL || pool->active != NULL) {
        
        // use or to insert the flag 0001 
        pool->pool_flags |= POOL_WAIT;
        
        // make threads wait, using mutex to keep them so
        pthread_cond_wait(&pool->wait, &pool->mutex);
        
    }
    
    pthread_cleanup_pop(1); /* Unlock(&pool->mutex); */
    
}

void pool_destroy(pool_t *pool){
    // Total obliteration time
    active_t *activep;
    job_t *job;

    // Lock that shtuff up, its my data and I want it now
    Lock(&pool->mutex);
    
    // push an Unlock to the cleanup stack
    pthread_cleanup_push((void*)Unlock, (void*)&pool->mutex);

    // mark the pool as being destroyed; wakeup lazy threads */
    pool->pool_flags |= POOL_DESTROY;
    
    // I AIN'T NEVER GET NO SLEEP CUS OF Y'ALL 
    // Y'ALL AIN'T NEVER NO SLEEP CUS OF ME
    // http://www.oxygen.com/bad-girls-club/season-14/blogs/the-best-of-tanisha-on-beat
    
    pthread_cond_broadcast(&pool->work);
    // Ya betta work!
    
    // Jk yall are gunna die
    for (activep = pool->active;            // start at the top of the active list
            activep != NULL;                // while we haven't reached the end
            activep = activep->active_next){// iterate to the next and the next...
     
        // Cancel the thread, bye bye.
        Thread_cancel(activep->active_tid);
    
    }

    // Take a seat, grab some coffee really, its not a problem, its not like we
    // have a program to run or anything 
    // no, no, I'll wait
    while (pool->active != NULL) {
        
        pool->pool_flags |= POOL_WAIT;
        pthread_cond_wait(&pool->wait, &pool->mutex);
        
    }

    // Oh? all of you are done? 
    // Great, now we can move on.
    while (pool->pool_nthreads != 0){
        // we now need to chill on busy threads
        pthread_cond_wait(&pool->busy, &pool->mutex);
    
    }
    
    // Unlock me databits
    pthread_cleanup_pop(1); /* Unlock(&pool->mutex); */

    // Global lock for safetey purposes
    Lock(&poolLock);
    
    if (poolsQ == pool){
        
        // Move forward in the pool Q
        poolsQ = pool->forward;
        
    }
    
    // If there was only one item in the Q then we won't 
    // have done anything in the if statement above
    if (poolsQ == pool){
     
        poolsQ = NULL;
    
    }else {
        
        // nice queue management
        pool->backward->forward = pool->forward;
        // if there are items in the queue we remove properly
        pool->forward->backward = pool->backward;
        
    }
    
    // Unlock the global lock 
    Unlock(&poolLock);

    // We should be 100% at this point 
    
    // but why not be sure
    for (job = pool->pool_head; job != NULL; job = pool->pool_head) {
        
        // iterate thorugh the jobs
        pool->pool_head = job->job_next;
    
        // I hope you're proud of me @Paul
        free(job);
        // You get memory! You get memory! You ALL get memoryyyyy!!
    }
    
    // Kaboooom, no more atributes 
    Thread_attr_destroy(&pool->pool_attr);
    
    // freedom! 
    free(pool);
    
}

void update(pool_t *pool) {
    
    if (pool->pool_head == NULL && pool->active == NULL) {
        // Remove the wait flag
        pool->pool_flags &= ~POOL_WAIT;
        // HEY YO EVERYBODY, CHECK THE WAIT COND PLZ KTHNXBAI
        pthread_cond_broadcast(&pool->wait);
        
    }
}

void jobCleanup(pool_t *pool) {
    // get myself
    pthread_t my_tid = pthread_self();
    
    // pointer to active thread
    active_t *activep;
    // pointer to pointer of active thread
    active_t **activepp;

    // Lock the mutex for the pool data 
    Lock(&pool->mutex);
    
    // 
    for (activepp = &pool->active; (activep = *activepp) != NULL; activepp = &activep->active_next) {
        // find me!
        if (activep->active_tid == my_tid) {
            // Look its me! who's next after me?
            *activepp = activep->active_next;
            break;
        }
    }
    // If the pool is waiting
    if (pool->pool_flags & POOL_WAIT){
        // Then update removing the wait flag and broadcasting
        update(pool);
    }
        
}