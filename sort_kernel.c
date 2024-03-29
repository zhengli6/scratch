
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static struct task_struct *thread_st;

#define MAX_THREADS 11
int input[12] = {2,5,3,1,6,8,7,9,53,23,3,4};/*List of integers contained in the file*/
int list[12];  /* List of up to 500 non-negative integers with values 0<=x<=1000*/
int result[12]; /* final sorted list*/
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
static int sort_fn(void *Ptr)
{
    parameters *data = Ptr;
    int i;
    printk(KERN_INFO "Thread#%d -> SORT\n", data->tid);
    printk(KERN_INFO "BEFORE SORT:");
    for(i=data->from_index; i<=data->to_index; i++)
    {
        printk(KERN_CONT "%d ", list[i]);
    }
    quickSort(list, data->from_index, data->to_index);
    printk(KERN_INFO "AFTER SORT:");
    for(i=data->from_index; i<=data->to_index; i++)
    {
        printk(KERN_CONT "%d ", list[i]);
    }
    printk(KERN_INFO "-------------------------------");
    do_exit(0);
    return 0;
}

static int merge_fn(void *Ptr)
{
    parameters *p = Ptr;
    printk(KERN_INFO "Thread#%d -> MERGE\n", p->tid);
    int i, j, num_samples, num_threads;
    num_threads = p->nt;
    num_samples = p->ns;

    for(i =0; i<num_threads; i++)
    {   
        data = (parameters*)vmalloc(sizeof(parameters));
        data->from_index = i*num_samples/num_threads;
        data->to_index = data->from_index + num_samples/num_threads - 1;
        /*Consider in the case of i==0, directly copy frist block to result*/
        if (i==0)
        {
            for (j=0; j<=data->to_index; j++)
            {
                result[j] = list[j];
            }
        }
        else
        {
            merger(data);
        } 
    };
    printk(KERN_INFO "-----------------------------");
    do_exit(0);
    return 0;
}
// Module Initialization
static int __init init_thread(void)
{

    /* ============================== READ SECTION ================================*/

    int num_samples = 12;
    int num_threads = 3;
    char *FileName = "Hard coded Array";
    int i;

    printk(KERN_INFO" ================================================\n");
    printk(KERN_INFO"|%48c|\n",' ');
    printk(KERN_INFO"| Array Size:           %-25d|\n", num_samples);
    printk(KERN_INFO"| Input File:           %-25s|\n", FileName);
    printk(KERN_INFO"| Number of Threads:    %-25d|\n", num_threads);
    printk(KERN_INFO"|%48c|\n",' ');
    printk(KERN_INFO" ================================================\n\n");


    for(i=0; i<num_samples; i++)
    {
        list[i] = input[i];
    }
    /* ============================== SORT SECTION ================================*/


    for(i =0; i<=num_threads-1; i++)
    {   
        data = (parameters*)vmalloc(sizeof(parameters));
        data->from_index = i*num_samples/num_threads;
        data->to_index = data->from_index + num_samples/num_threads - 1;
        data->tid = i;
        data->nt = num_threads;
        thread_st = kthread_run(sort_fn, data, "thread#1");
        if (thread_st)
            printk(KERN_INFO "Thread#%d Created successfully\n", i);
        else
            printk(KERN_ERR "Thread#%d creation failed\n", i);
        ssleep(1);
    };

    /* ============================== MERGE SECTION ================================*/
    data = (parameters*)vmalloc(sizeof(parameters));
    data->from_index = 0;
    data->to_index = 0;
    data->tid = i;
    data->nt = num_threads;
    data->ns = num_samples;

    thread_st = kthread_run(merge_fn, data, "thread#1");
    if (thread_st)
        printk(KERN_INFO "Thread#%d Created successfully\n", i);
    else
        printk(KERN_ERR "Thread#%d creation failed\n", i);
    ssleep(2);
    /* ============================== OUTPUTS DISPLAY ================================*/
    printk(KERN_INFO "\nINPUT ARRAY:\n");
    for (i=0; i<num_samples; i++)
    {
    printk(KERN_CONT "%d ", input[i]);
    }
    printk(KERN_INFO "\nFINAL RESULT:\n");
    for (i=0; i<num_samples; i++)
    {
    printk(KERN_CONT "%d ", result[i]);
    }
    printk(KERN_INFO "\n");
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
