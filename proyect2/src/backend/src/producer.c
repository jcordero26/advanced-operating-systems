#include "include/producer.h"

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/mman.h>
#include <semaphore.h>


int writeData(bufferElement *pBuffElement, dataMessage message, int bufferIndex, char *bufferName)
{
	int semValue;
    if ((pBuffElement->indexAvailable != 1) || (sem_getvalue(&(pBuffElement->mutex), &semValue) < 0) || (semValue == 0))
    {
    	printf("Warning, not possible to write in the shared buffer.\n");
    	return 0;
    }

    sem_wait(&(pBuffElement->mutex));

    //critical section
    pBuffElement->producerId = message.producerId;
    strcpy(pBuffElement->date, message.date);
    pBuffElement->key = message.key;

    // no longer available
    pBuffElement->indexAvailable = 0;

    printf("==================================\n");
	printf("Message written at index %d of the buffer %s.\n", bufferIndex, bufferName);
	printf("Producer ID who wrote the message %i.\n", pBuffElement->producerId);
	printf("Key of the message %i.\n", pBuffElement->key);
	printf("Date of the message %s.\n", pBuffElement->date);
	printf("==================================\n");

    //signal
    sem_post(&(pBuffElement->mutex));

    return 1;
}


void tryWrite(dataMessage message, char *bufferName)
{
	int fileDescriptor = open(bufferName, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fileDescriptor == -1)
	{
		printf("Error, open() failed, error code %d.\n", errno);
		return;
	}

	// map shared memory to process address space
	sharedBuffer *pSharedBuffer = (sharedBuffer*)mmap(NULL, sizeof(sharedBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
	if (pSharedBuffer == MAP_FAILED)
	{
		printf("Error, mmap() failed, error code %d.\n", errno);
		return;
	}

	fileDescriptor = open(pSharedBuffer->childBufferName, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fileDescriptor == -1)
	{
		printf("Error, open() failed, error code %d.\n", errno);
		return;
	}

	// map shared memory to process address space
	bufferElement *pChildBuffer = (bufferElement*)mmap(NULL, sizeof(bufferElement), PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
	if (pChildBuffer == MAP_FAILED)
	{
		printf("Error, mmap() failed, error code %d.\n", errno);
		return;
	}

	for (int i = 0; i < pSharedBuffer->size; i++)
	{
		if (!writeData(pChildBuffer, message, i, bufferName))
		{
			continue;
		}
		else
		{
			return;
		}
	}
}