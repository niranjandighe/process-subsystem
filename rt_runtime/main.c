#include<stdio.h>
#include<pthread.h>
#include<assert.h>
#include<stdlib.h>
#include<sched.h>

#ifndef COUNT
#define COUNT (0x1fffffff)
#endif
pthread_mutex_t index_mutex = PTHREAD_MUTEX_INITIALIZER;

static int data[2][8];
static int index[2];
static char *names[2][8];

struct thread_desc {
	unsigned int sched_policy;
	unsigned int cpu;
	void * (*thread_func)(int, int);
	int nice;
	int prio;
	pthread_t thread;
	char const *name;
	int idx;
};

void * startup(void *d)
{
	struct thread_desc *desc = (struct thread_desc *)d;
	cpu_set_t cpuset;
	int idx, i;
	struct sched_param param;
	unsigned long long sum;
	double percentage;

	CPU_ZERO(&cpuset);
	CPU_SET(desc->cpu, &cpuset);


	pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);


	if (desc->sched_policy == SCHED_OTHER)
		nice(desc->nice);
	else {
		param.sched_priority = desc->prio;
		pthread_setschedparam(pthread_self(), desc->sched_policy, &param);
	}

	desc->thread_func(desc->cpu, desc->idx);

	if (desc->sched_policy == SCHED_OTHER)
		return NULL;

	printf("Data -\n------\n");

	for(i = 0; (i < 8) && names[desc->cpu][i]; i++)
		printf("%s\t", names[desc->cpu][i]);
	printf("\n");

	for(i = 0; i < 8; i++)
		sum += data[desc->cpu][i];

	for(i = 0; i < 8; i++)
		printf("%lf%\t", ((double)data[desc->cpu][i]/(double)sum) * 100);
	printf("\n\n");	
}

void spawn_thread(struct thread_desc *desc)
{
	assert(desc->thread_func != NULL);
	assert((desc->cpu == 0) || (desc->cpu == 1));
	assert((desc->nice >= -20) && (desc->nice < 20));
	assert(desc->prio < 140);

	switch(desc->sched_policy)
	{
		case SCHED_FIFO:
		case SCHED_RR:
		case SCHED_OTHER:
			//pthread_attr_setschedpolicy(&attr, desc->sched_policy);
			break;
		default:
			//pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
			desc->sched_policy = SCHED_OTHER;
	}
	desc->idx = index[desc->cpu]++;
	names[desc->cpu][desc->idx] = desc->name;

	pthread_create(&desc->thread, NULL, startup, (void *)desc);
}

void perform(int cpu, int idx)
{
	int i = COUNT;

	while(i--)
		data[cpu][idx]++;
}

int main()
{
	struct thread_desc threads[] = {
		{
			.sched_policy = SCHED_OTHER,
			.cpu = 0,
			.thread_func = perform,
			.nice = -20,
			.name = "SCHED_OTHER-0"
		},
		{
			.sched_policy = SCHED_FIFO,
			.cpu = 0,
			.thread_func = perform,
			.name = "SCHED_FIFO-0",
			.prio = 1
		},
		{
			.sched_policy = SCHED_RR,
			.cpu = 0,
			.thread_func = perform,
			.name = "SCHED_RR-0",
			.prio = 1
		},
		{
			.sched_policy = SCHED_RR,
			.cpu = 1,
			.thread_func = perform,
			.name = "SCHED_RR-1",
			.prio = 1
		},
		{
			.sched_policy = SCHED_OTHER,
			.cpu = 0,
			.thread_func = perform,
			.nice = -20,
			.name = "SCHED_OTHER-0"
		}
	};
	int i;
	int *ret;

	for(i = 0; i < sizeof(threads)/sizeof(threads[0]); i++)
	{
		spawn_thread(&threads[i]);
	}

	for(i = 0; i < sizeof(threads)/sizeof(threads[0]); i++)
	{
		pthread_join(threads[i].thread, &ret);
	}

	return 0;
}
