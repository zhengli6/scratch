
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

void *sort_routine(void* Ptr)
{   
    parameters *data = Ptr;
    /*
    int i;
    printf("--------------------------------\n");
    printf("Thread id is: %d SORT: %d->%d elements\n", data->tid, data->from_index, data->to_index);
    printf("BEFORE SORT:");

    for(i=data->from_index; i<=data->to_index; i++)
    {
        printf("%d ", list[i]);
    }
    printf("\n");
    */
    quickSort(list, data->from_index, data->to_index);
    /*
    printf("AFTER SORT:");
    for(i=data->from_index; i<=data->to_index; i++)
    {
        printf("%d ", list[i]);
    }
    printf("\n");
    free(Ptr);
    pthread_exit(NULL);
    */
}

void *merge_routine(void* Ptr)
{
    parameters *p = Ptr;
    /*
    printf("--------------------------------\n");
    printf("Thread id is: %d MERGE ROUTINE \n", p->tid);
    */
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
    /*
    free(Ptr);
    pthread_exit(NULL);
    */
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
    int num_samples = 12;
    int num_threads = 2;
    char *FileName = "Hard coded Array";
    int temp;
    int i;

    printk(KERN_INFO" ================================================\n");
    printk(KERN_INFO"|%47c|\n",' ');
    printk(KERN_INFO"| Array Size:\t\t%-24d|\n", num_samples);
    printk(KERN_INFO"| Input File:\t\t%-24s|\n", FileName);
    printk(KERN_INFO"| Number of Threads:\t%-24d|\n", num_threads);
    printk(KERN_INFO"|%47c|\n",' ');
    printk(KERN_INFO" ================================================\n\n");


    for(i=0; i<num_samples; i++)
    {
        list[i] = input[i];
    }

    for(i =0; i<=num_threads-1; i++)
    {   
        data = (parameters*)malloc(sizeof(parameters));
        data->from_index = i*num_samples/num_threads;
        data->to_index = data->from_index + num_samples/num_threads - 1;
        data->tid = i;
        data->nt = num_threads;
        thread_st = kthread_run(sort_routine, data, "thread#1");
        if (thread_st)
        printk(KERN_INFO "Thread#%d Created successfully\n", i);
        else
            printk(KERN_ERR "Thread#%d creation failed\n", i);
        ssleep(1);
    };

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