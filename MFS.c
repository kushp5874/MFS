// Name: Kushal Patel 			ID: V00733023
// CSC 360 Assignment 2

#include <stdio.h>
#include <err.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <bsd/stdlib.h>

#define MAX 100
typedef struct{
	pthread_t thread_id;
	int id, arr, run, prio; // id= flow #, arr= arrival time, run= transmission time, prio= priority
}Flow;

void init(char *str, Flow *flow);
int flowComp(const void *A, const void *B);
int comp(const Flow *flowA, const Flow *flowB);
static void start(Flow *self);
void finish();
int flowComp(const void *A, const void *B);
void control(Flow *flow);
int main(int argc, char *argv[]);

pthread_mutex_t mu1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mu2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t idle = PTHREAD_COND_INITIALIZER;
int status = 1;//0 if running, 1 if idle
int currID;
Flow *waiting[MAX];//array of waiting flows
int numWait = 0; //number of flow waiting

void init(char *str, Flow *flow){
	flow->id = strtonum(strtok(str,":"), 1, INT_MAX, NULL);
	flow->arr = strtonum(strtok(NULL,","), 1, INT_MAX, NULL);
	flow->run = strtonum(strtok(NULL,","), 1, INT_MAX, NULL);
	flow->prio = strtonum(str, 1, INT_MAX, NULL);
	
	int x = atoi(strtok(str,":"));
	printf("%d \n", x);
	/*flow->id = atoi(strtok(str,":"));
	flow->arr = atoi(strtok(NULL,","));
	flow->run = atoi(strtok(NULL,","));
	flow->prio = atoi(str);  */
}

#define larger(val, NEXT_COND)\
	do{\
		if(flowA->val < flowB->val){\
			return -1;\
		}\
		else if(flowA->val > flowB->val){\
			return 1;\
		}\
		else if(flowA->val == flowB->val){\
			NEXT_COND;\
		}\
	}while(0)
#define smaller(val, NEXT_COND)\
	do{\
		if(flowA->val > flowB->val){\
			return -1;\
		}\
		else if(flowA->val < flowB->val){\
			return 1;\
		}\
		else if(flowA->val == flowB->val){\
			NEXT_COND;\
		}\
	}while(0)

int flowComp(const void *A, const void *B){
	return comp((Flow *) A, (Flow *) B);
}

int comp(const Flow *flowA, const Flow *flowB){
	larger(prio, smaller(arr, smaller(run, smaller(id, return 0))));
	return 0;
}

static void start(Flow *self){//start flow, wait if a flow is running
	pthread_mutex_lock(&mu1);
	if(status == 1 && numWait == 0){
		status = 0;
		pthread_mutex_unlock(&mu1);
		return;
	}
	pthread_mutex_lock(&mu2);
	waiting[numWait] = self;
	numWait++;
	qsort(waiting, numWait, sizeof(Flow), flowComp);
	pthread_mutex_unlock(&mu2);

	while(status == 0 || comp(waiting[0], self) != 0){
		printf("Flow %2d waits for the finish of flow %2d. \n", self->id, currID);
		pthread_cond_wait(&status, &mu1);
	}
	waiting[0] = waiting[numWait-1];
	qsort(waiting, numWait, sizeof(Flow), flowComp);
	status = 0;
	numWait--;
	pthread_mutex_unlock(&mu1);
}

void finish(){
	pthread_mutex_lock(&mu1);
	status = 1;
	pthread_mutex_unlock(&mu1);
	pthread_cond_broadcast(&status);
}

void * flowCTRL(void *flow){
	control((Flow *) flow);
	return 0;
}

void control(Flow *flow){
	struct timespec arrival, service, removal;
	time_t t;
	arrival.tv_nsec = flow->arr *1000 *1000;//convert to seconds
	service.tv_nsec = flow->run *1000 *1000;//convert to seconds
	nanosleep(&arrival, &removal);
	printf("Flow %2d arrives: arrival time (%.2f), transmission time (%.1f), priority (%2d). \n", flow->id, flow->arr, flow->run, flow->prio);
	start(flow);
	currID = flow->id;
	time(&t);
	printf("Flow %2d starts its transmission at time %.2f. \n", flow->id, ctime(&t));
	nanosleep(&service, &removal);
	time(&t);
	printf("Flow %2d finishes its transmission at time %d. \n", flow->id, ctime(&t));
	finish();
}

int main(int argc, char *argv[]){
	FILE *f;//file
	char *l;//line
	size_t len = 0;//string length
	Flow *flow;

	if(argc < 2){
		errx(1, "Error");
	}
	f = fopen(argv[1], "r");
	getline(&l, &len, f);
	printf("File opened");

	while(getline(&l, &len, &f) != -1){
		flow = (Flow *)malloc(sizeof(flow));
		init(l, flow);
		pthread_create(&flow->thread_id, NULL, flowCTRL, flow);
	}
	free(l);
	fclose(f);
	pthread_exit(0);
	return 0;
}