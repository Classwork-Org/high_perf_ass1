/* C implementation QuickSort */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h> 

#define NTHREAD 1
#define TEST_ARRAY_SIZE 100000000
#define MAX_VAL 10000

typedef struct
{
	int *arr;
	int low;
	int high;
} SortArgs;

typedef struct
{
	int *arr;
	int l;
	int m;
	int r;
} MergeArgs;

unsigned int logBase2( unsigned int x )
{
  unsigned int ans = 0 ;
  while( x>>=1 ) ans++;
  return ans ;
}

// A utility function to swap two elements 
void swap(int* a, int* b) 
{ 
	int t = *a; 
	*a = *b; 
	*b = t; 
} 
/* Function to print an array */
void printArray(int arr[], int size) 
{ 
	int i; 
	for (i=0; i < size; i++) 
	{		
		printf("arr[%d]: %d\n", i, arr[i]); 
	}	
	// printf("\n"); 
} 
/* This function takes last element as pivot, places 
the pivot element at its correct position in sorted 
array, and places all smaller (smaller than pivot) 
to left of pivot and all greater elements to right 
of pivot */
int partition (int arr[], int low, int high) 
{ 
	int pivot = arr[high]; // pivot 
	int i = (low - 1); // Index of smaller element 
	int j;

	for (j = low; j <= high- 1; j++) 
	{ 
		// If current element is smaller than the pivot 
		if (arr[j] < pivot) 
		{ 
			i++; // increment index of smaller element 
			swap(&arr[i], &arr[j]); 
		} 
	} 
	swap(&arr[i + 1], &arr[high]); 
	return (i + 1); 
} 

/* The main function that implements QuickSort 
arr[] --> Array to be sorted, 
low --> Starting index, 
high --> Ending index */
void *quickSort(void *args) 
{ 
	SortArgs range = *((SortArgs*)args);
	int low = range.low;
	int high = range.high;
	int *arr = range.arr;
	// printArray(arr, TEST_ARRAY_SIZE);
	if (low < high) 
	{ 
		/* pi is partitioning index, arr[p] is now 
		at right place */
		int pi = partition(arr, low, high); 

		// Separately sort elements before 
		// partition and after partition 
		range.low = low;
		range.high = pi-1;
		quickSort(&range); 

		range.low = pi+1; 
		range.high = high;
		quickSort(&range); 
	} 
} 

int* generateRandomArray(int size, int maxNum)
{
	int *arr = (int*)malloc(size*sizeof(int));
	int i;
	for(i = 0; i<size; i++)
	{
		(arr)[i] = rand()%maxNum;
	}
	return arr;
}

// Merges two subarrays of arr[]. 
// First subarray is arr[l..m] 
// Second subarray is arr[m+1..r] 
void *merge(void* args) 
{ 
	MergeArgs range = *((MergeArgs*)args);

	int* arr = range.arr;
	int l = range.l;
	int m = range.m;
	int r = range.r;

    int i, j, k; 
    int n1 = m - l + 1; 
    int n2 =  r - m; 

    /* create temp arrays */
    int *L = (int*)malloc(n1*sizeof(int));
    int *R = (int*)malloc(n2*sizeof(int));

    if(L == NULL || R == NULL)
    {
    	printf("FAILED TO ALLOCATE MEMORY FOR L AND R ARRAYS\n");
    	exit(-1);
    }
  
    /* Copy data to temp arrays L[] and R[] */
    for (i = 0; i < n1; i++) 
        L[i] = arr[l + i]; 
    for (j = 0; j < n2; j++) 
        R[j] = arr[m + 1+ j]; 


    /* Merge the temp arrays back into arr[l..r]*/
    i = 0; // Initial index of first subarray 
    j = 0; // Initial index of second subarray 
    k = l; // Initial index of merged subarray 
    while (i < n1 && j < n2) 
    { 
        if (L[i] <= R[j]) 
        { 
            arr[k] = L[i]; 
            i++; 
        } 
        else
        { 
            arr[k] = R[j]; 
            j++; 
        } 
        k++; 
    } 
  
    /* Copy the remaining elements of L[], if there 
       are any */
    while (i < n1) 
    { 
        arr[k] = L[i]; 
        i++; 
        k++; 
    } 
  
    /* Copy the remaining elements of R[], if there 
       are any */
    while (j < n2) 
    { 
        arr[k] = R[j]; 
        j++; 
        k++; 
    } 

    // printf("merge has merged array\n");
    // printArray(arr, r-l+1);

    free(L);
    free(R);
} 

int partitionQuickSort(int *arr, unsigned int size)
{
	pthread_t thrd[NTHREAD];
	SortArgs range[NTHREAD];

	int sortSectionSize = size/NTHREAD;

	int i;
	int low = 0;
	int high = sortSectionSize;
	for (i = 0; i < NTHREAD; ++i)
	{
		range[i].low = low;
		range[i].high = high-1;
		range[i].arr = arr;
		low+=sortSectionSize;
		high+=sortSectionSize;
	}

	for(i = 0; i<NTHREAD; i++)
	{
		printf("Thread [%d] sorting ranges %d to %d\n", i, range[i].low, range[i].high);
		if(pthread_create(&thrd[i], NULL, &quickSort, &(range[i])))
		{
			printf("FAILED TO CREATE THREAD\n");
			return -1;
		}
	}

	for(i = 0; i<NTHREAD; i++)
	{
		pthread_join(thrd[i], NULL);
	}
	return 0;
}

int parallelMerge(int *arr, unsigned int size)
{
	int i,j,k;
	int sortSectionSize = size/NTHREAD;
	pthread_t thrd[NTHREAD/2];


	MergeArgs mergeRange[NTHREAD/2];
	unsigned int nMergeThreads = NTHREAD/2;
	unsigned int mergeSectionSize = sortSectionSize*2;
	unsigned int log2_NTHREAD = logBase2(NTHREAD);
	for(i = 0; i<log2_NTHREAD; i++)
	{
		mergeRange[0].arr = arr;
		mergeRange[0].l = 0;
		mergeRange[0].m = mergeSectionSize/2-1;
		mergeRange[0].r = mergeSectionSize-1;

		for(j = 1; j<nMergeThreads; j++)
		{
			mergeRange[j].arr = arr;
			mergeRange[j].l = mergeRange[j-1].l + mergeSectionSize;
			mergeRange[j].m = mergeRange[j-1].m + mergeSectionSize;
			mergeRange[j].r = mergeRange[j-1].r + mergeSectionSize;
		}

		for(j = 0; j<nMergeThreads; j++)
		{
			printf("Thread[%d] Merging Range low: %d, mid: %d, high: %d\n", \
				j, mergeRange[j].l, mergeRange[j].m, mergeRange[j].r);
			if(pthread_create(&thrd[j], NULL, &merge, &(mergeRange[j])))
			{
				printf("FAILED TO CREATE THREAD\n");
				return -1;
			}
		}

		for(j = 0; j<nMergeThreads; j++)
		{
			pthread_join(thrd[j], NULL);
		}


		nMergeThreads /= 2;
		mergeSectionSize *= 2;
	}
}

int verifyArrayAscendingOrder(int *arr, unsigned int size)
{
	int k;	
	for (k = 0; k < TEST_ARRAY_SIZE-1; ++k)
	{
		if(arr[k]>arr[k+1])
		{
			// printArray(arr, TEST_ARRAY_SIZE);
			printf("ERROR AT arr[%d]:%d, arr[%d]:%d!\n", k, arr[k], k+1, arr[k+1]);
			// assert(1 == 2);
			return -1;
		}
	}
	return 0;

}

void seedRandomNumberGenerator()
{
	srand(time(0));
}

// Driver program to test above functions 
int main() 
{ 

	seedRandomNumberGenerator();

	printf("Creating array of size: %d with random values between 0 and %d\n", \
		TEST_ARRAY_SIZE, MAX_VAL);

	int *arr = generateRandomArray(TEST_ARRAY_SIZE, MAX_VAL);

	if(arr == NULL)
	{
		printf("FAILED TO ALLOCATE RANDOM ARRAY\n");
		return -1;
	}

	printf("Begin Sorting array\n");

	if(partitionQuickSort(arr, TEST_ARRAY_SIZE))
	{
		printf("ERROR: Partition quick sort failed!\n");
		return -1;
	}
	if(parallelMerge(arr, TEST_ARRAY_SIZE))
	{
		printf("ERROR: parallel Merge failed!\n");
		return -1;
	}

	printf("%s\n", "Running sanity check... ");		
	if(verifyArrayAscendingOrder(arr, TEST_ARRAY_SIZE))
	{
		printf("ERROR: verification failed\n");
		return -1;
	}
	printf("%s\n", "DONE!");

	free(arr);

	return 0; 
} 




