#include "nogvl.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#define N_THREAD 3
#define N_LOOP 1000000000

pid_t gettid(void)
{
    return syscall(SYS_gettid);
}

void *thread_main(void *p)
{
    pid_t current_tid = gettid();
    printf("tid: %d\n", current_tid);
    for (size_t i = 0; i < N_LOOP; i++);
    pthread_exit(NULL);
    return 0;
}

int invoke_threads()
{
    pthread_t thread[N_THREAD];
    int ret;
    for (int i = 0; i < N_THREAD; i++) {
        ret = pthread_create(&thread[i], NULL, (void *)thread_main, NULL);
        if (ret < 0) return -1; // failure
    }
    for (int i = 0; i < N_THREAD; i++) {
        ret = pthread_join(thread[i], NULL);
        if (ret != 0) return -1;  // failure
    }
    return 0;
}

// rb_thread_call_without_gvl
struct nogvl_run_args {
    int val;
};

// rb_thread_call_without_gvl
static void* nogvl_run(void *ptr)
{
    struct nogvl_run_args *args = ptr;
    printf("args->val %d\n", args->val);
    invoke_threads();
    return (void*)Qnil;
}

static VALUE run()
{
    struct nogvl_run_args args;
    args.val = 1;
    return (VALUE)rb_thread_call_without_gvl(nogvl_run, &args, RUBY_UBF_IO, NULL);
}

void
Init_nogvl(void)
{
    VALUE mNogvl = rb_define_module("Nogvl");
    rb_define_singleton_method(mNogvl, "run", run, 0);
}
