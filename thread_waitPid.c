/*
 * thread_waitPid.c
 *	Поток для принятия завершающих сигналов от потомков-серверов tcp
 *  Created on: 14 февр. 2018 г.
 *      Author: jake
 */

#include <stdlib.h>
#include <pthread.h>

void* thread_waitPid(void*ppid)
{
	pid_t pid = *((pid_t*)ppid);
	waitpid(pid,0,0);
	pthread_exit(0);
}
