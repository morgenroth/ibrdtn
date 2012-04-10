#include "ibrcommon/config.h"
#include "ibrcommon/thread/Semaphore.h"
#include <iostream>

using namespace std;

namespace ibrcommon
{
	Semaphore::Semaphore(unsigned int value)
	{
		sem_init(&count_sem, 0, value);
	}

	Semaphore::~Semaphore()
	{
		sem_destroy(&count_sem);
	}

	void Semaphore::wait()
	{
		sem_wait(&count_sem);
	}

	void Semaphore::post()
	{
		sem_post(&count_sem);
	}
}
