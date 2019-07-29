
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>

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
void swap_pair(int* a, int* b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

/*partition function*/
int partition(int arr[], int low, int high)
{
    int pivot = arr[high];
    int i = (low-1);
    int j = low;
    for (j = low; j<high; j++)
    {
        if (arr[j]<= pivot)
        {
            i++;
            swap_pair(&arr[i], &arr[j]);
        }
    }
    swap_pair(&arr[i+1], &arr[high]);
    return (i+1);
}

/*Sorting thread (QuickSort)*/
void quickSort(int arr[], int low, int high)
{
    if (low<high)
    {
        int pivot = partition(arr, low, high);

        quickSort(arr, low, pivot-1);
        quickSort(arr, pivot+1, high);
    }
}

void merger(void *params)
{
    parameters* p = (parameters *)params; int i, j, k;
    
    int arr1[p->to_index - p->from_index + 1];
    int arr2[p->from_index];
    int n1 = sizeof(arr1) / sizeof(int);
    int n2 = sizeof(arr2) / sizeof(int);
    k=0;
    /*Setup two arrays to be merged*/
    for (i=p->from_index; i<=p->to_index; i++)
    {
        arr1[k] = list[i]; k++;
    }
    k=0;
    for (i=0; i<=p->from_index-1; i++)
    {   
        arr2[k] = result[i]; k++;
    }
    i=0; j=0;/* position being inserted into result list */
    int pposition = 0;
    while (i < n1 && j < n2) 
    {   
        if (arr1[i] <= arr2[j]) 
        {
            result[pposition] = arr1[i]; 
            pposition++; i++;

        }
        else
        {
            result[pposition] = arr2[j]; 
            pposition++; j++;
        } 
    }

    /* copy the remainder */
    while (i < n1) 
    {
         result[pposition] = arr1[i]; 
         pposition++; i++;
    }      
    
    while (j < n2) 
    {
        result[pposition] = arr2[j];
        pposition++; j++;
    } 
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