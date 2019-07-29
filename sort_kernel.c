
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <std.io>

static struct task_struct *thread_st;

#define MAX_THREADS 11
int input[500] = {2,5,3,1,6,8,7,9,53,23,3,4,7,1,71,66,22,34,51,16,22,11,13,46,24,88,192,222,431,94,29,32,18,72,17,19,122,43,15,33,36,31,30,42,57,61,39,74,12,18,37,7,14,9,29,19,86,69,23,57};/*List of integers contained in the file*/
int list[500];  /* List of up to 500 non-negative integers with values 0<=x<=1000*/
int result[500]; /* final sorted list*/
typedef struct
{
    int from_index;
    int to_index;
    int tid;
    int nt;
    int ns;
}parameters;

parameters *data;

/* swap function*/
void swap(int* a, int* b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

// Function executed by kernel thread
static int thread_fn(void *unused)
{
    
    printk(KERN_INFO "Thread Running\n");
    ssleep(2);
    
    printk(KERN_INFO "Thread Stopping\n");
    do_exit(0);
    return 0;
}
// Module Initialization
static int __init init_thread(void)
{
    printk(KERN_INFO "Creating Thread\n");
    //Create the kernel thread with name 'mythread'
    thread_st = kthread_run(thread_fn, NULL, "mythread");
    if (thread_st)
        printk(KERN_INFO "Thread Created successfully\n");
    else
        printk(KERN_ERR "Thread creation failed\n");
    return 0;
}
// Module Exit
static void __exit cleanup_thread(void)
{
   printk(KERN_INFO "Cleaning Up\n");
   /*
   if (thread_st)
   {
       kthread_stop(thread_st);
       printk(KERN_INFO "Thread stopped");
   }
   */
}
MODULE_LICENSE("GPL");
module_init(init_thread);
module_exit(cleanup_thread);