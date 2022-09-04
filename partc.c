#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// Let us create a global variable to change it in threads
int average,min,max;
#define INF 100000000
#define  MAX_LIMIT 100
// The function to be executed by all threads

int mynums[MAX_LIMIT];
//this contains the elemets
int l;
//this is the size of the array mynums

void *ave(void *vargp)
{

	int *myid = (int *)vargp;
  
	int ctr=0;
	while(ctr<l){
	average=average+mynums[ctr];
	ctr++;
	}
	average=average/l;
	
	printf("The average value is %d (rounded down)and the thread id is %d\n",average,*myid);

	return NULL;
}

void *maximum(void *vargp)
{
	int ctr=0;
	int *myid = (int *)vargp;
  
	max=-INF;
	while(ctr<l){
	int temp=mynums[ctr];
	if (max<temp) max=temp;
	ctr++;}
	printf("The maximum value is %d and the thread id is %d\n",max,    *myid);

	return NULL;
}

void *minimum(void *vargp)
{
	int ctr=0;
	int *myid = (int *)vargp;
  
	min=INF;
	while(ctr<l){
	int temp =mynums[ctr];
	if (min>temp) min=temp;
	ctr++;}
	 printf("The minimum value is %d and the thread id is %d\n",min,    *myid);

	return NULL;

}


int main()
{
	int i;
	pthread_t t1id,t2id,t3id;
	printf("Enter the elements:\n");
	int count=0;

	do{
	scanf("%d", &mynums[count++]);


	}while(getchar() != '\n' && count < MAX_LIMIT-2);
	mynums[count];//resize the array to count
	//array is made
	l=count;//update the length of the array	
	// Let us create three threads

	pthread_create(&t1id, NULL, ave, (void *)&t1id );
	pthread_join(t1id, NULL);
	pthread_create(&t2id, NULL, maximum, (void *)&t2id);
	pthread_join(t2id, NULL);
	pthread_create(&t3id, NULL, minimum, (void *)&t3id);
	pthread_join(t3id, NULL);

	//the three global variables average, max, min are set by the threads
	printf("AVG:%d MIN:%d MAX:%d and the process id is :%d\n",average,min ,max ,getpid());


	
	pthread_exit(NULL);
}

