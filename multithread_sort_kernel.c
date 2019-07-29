#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
/*partition function*/
int partition(int arr[], int low, int high)
{
    int pivot = arr[high];
    int i = (low-1);
    for (int j = low; j<high; j++)
    {
        if (arr[j]<= pivot)
        {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i+1], &arr[high]);
    return (i+1);
}

/*Sorting thread (QuickSort)*/
void quickSort(int arr[], int low, int high)
{
    int i;
    if (low<high)
    {
        int pivot = partition(arr, low, high);
        /*
        printf("Pivot: %d\n", pivot);
        for (i=0; i<4; i++)
        {
            if (i==pivot)
            {
                printf("*%d* ",list[i]);
            }else
            {
                printf("%d ",list[i]);
            }
            
        }
        printf("\n");*/
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
    int position = 0;
    while (i < n1 && j < n2) 
    {   
        if (arr1[i] <= arr2[j]) 
        {
            result[position] = arr1[i]; 
            position++; i++;

        }
        else
        {
            result[position] = arr2[j]; 
            position++; j++;
        } 
    }

    /* copy the remainder */
    while (i < n1) 
    {
         result[position] = arr1[i]; 
         position++; i++;
    }      
    
    while (j < n2) 
    {
        result[position] = arr2[j];
        position++; j++;
    } 

}
void *sort_routine(void* Ptr)
{   
    int i;
    parameters *data = Ptr;
    printf("--------------------------------\n");
    printf("Thread id is: %d SORT: %d->%d elements\n", data->tid, data->from_index, data->to_index);
    printf("BEFORE SORT:");
    for(i=data->from_index; i<=data->to_index; i++)
    {
        printf("%d ", list[i]);
    }
    printf("\n");
    quickSort(list, data->from_index, data->to_index);
    printf("AFTER SORT:");
    for(i=data->from_index; i<=data->to_index; i++)
    {
        printf("%d ", list[i]);
    }
    printf("\n");
    free(Ptr);
    pthread_exit(NULL);
    

}

void *merge_routine(void* Ptr)
{
    parameters *p = Ptr;
    printf("--------------------------------\n");
    printf("Thread id is: %d MERGE ROUTINE \n", p->tid);
    int i, j, num_samples, num_threads, from_index, to_index;
    num_threads = p->nt;
    num_samples = p->ns;

    for(i =0; i<num_threads; i++)
    {   
        data = (parameters*)malloc(sizeof(parameters));
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

    free(Ptr);
    pthread_exit(NULL);
}


/* This function is called when the module is removed. */ 
void main_exit(void)
{
    printk(KERN_INFO "Removing SORTING Module\n"); 
}

int main_init(int argc, char * argv[])
{

    printk(KERN_INFO "Loading SORTING Module\n"); 
    
    /*
    if(argc != 4) 
    {
        printf("Please Specify [Array Size][Input FileName][Number of Threads] In The Command Line\n");
        exit(1); 
    }
    
    int num_samples = atoi(argv[1]);
    int num_threads = atoi(argv[3]);
    char *FileName = argv[2];
    */

    int num_samples = 30;
    int num_threads = 5;
    char *FileName = "Hard coded Array";
    int temp;

    pthread_t workers[num_threads + 1];
    int i;

    if(num_threads > MAX_THREADS) num_threads = MAX_THREADS;
    printf(" ================================================\n");
    printf("|%47c|\n",' ');
    printf("| Array Size:\t\t%-24d|\n", num_samples);
    printf("| Input File:\t\t%-24s|\n", FileName);
    printf("| Number of Threads:\t%-24d|\n", num_threads);
    printf("|%47c|\n",' ');
    printf(" ================================================\n\n");
    if (num_samples % num_threads!=0)
    {
        fprintf(stderr, "* Integers(%d) cannot be evenly allocated across threads(%d).\n  Please specify different number of threads!\n", num_samples, num_threads);
        return 1;
    }
    
    /*
    FILE *myFile = fopen(FileName, "r");
    if (myFile== NULL)
    {
        fprintf(stderr,"Error: Fail to open file: %s\n", FileName);
        return 1;
    }
    for (i = 0; i< num_samples; i++)
    {   
        if (fscanf(myFile, "%d,", &temp) == 1)
        {
            input[i] = temp;
        }
        else
        {
            break;
        }
    }
    fclose(myFile);
    */
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
        pthread_create(&workers[i], 0, sort_routine, data);
        sleep(1);
    };
    for(i=0; i<=num_threads-1; i++)
    {
        pthread_join(workers[i], NULL);
    }

    data = (parameters*)malloc(sizeof(parameters));
    data->from_index = 0;
    data->to_index = 0;
    data->tid = i;
    data->nt = num_threads;
    data->ns = num_samples;
    pthread_create(&workers[num_threads + 1], 0, merge_routine, data);

    pthread_join(workers[num_threads + 1], NULL);

    printf("\nINPUT ARRAY:\n");
    for(i=0; i<num_samples; i++)
    {
        printf("%d ", input[i]);
    }
    printf("\n");
    printf("\nFINAL RESULT:\n");
    for(i=0; i<num_samples; i++)
    {
        printf("%d ", result[i]);
    }
    printf("\n");
    
    return 0;
}

/* Macros for registering module entry and exit points. */ 
module_init(main_init);
module_exit(main_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Sorting Module"); 
MODULE_AUTHOR("ZhengLi@SMU");
/*
ksmod > output lsmod.txt
dmesg > output dmesg.txt
*/

