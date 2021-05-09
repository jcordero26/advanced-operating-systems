#include "include/finisher.h"
#include "include/common.h"
#include "include/producerConsumerManager.h"
#include "include/scheduler.h"
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/mman.h>
#include <time.h>


int writeDataTerminateMessage(bufferElement *pBuffElement, dataMessage message, int bufferIndex, char *bufferName)
{
	int semValue;
    if ((pBuffElement->indexAvailable != 1) || (sem_getvalue(&(pBuffElement->mutex), &semValue) < 0) || (semValue == 0))
    {
    	printf("Warning, not possible to write in the shared buffer.\n");
    	return 0;
    }

    sem_wait(&(pBuffElement->mutex));

    //critical section
    pBuffElement->data.producerId = message.producerId;
    strcpy(pBuffElement->data.date, message.date);
    pBuffElement->data.key = message.key;
	
	pBuffElement->data.killFlag = message.killFlag;

    // no longer available
    pBuffElement->indexAvailable = 0;

    printf("==================================\n");
	printf("Message WRITE at index %d of the buffer %s.\n", bufferIndex + 1, bufferName);
	//printf("Producer ID who wrote the message %i.\n", pBuffElement->data.producerId);
	//printf("Key of the message %i.\n", pBuffElement->data.key);
	printf("Date of the message %s.\n", pBuffElement->data.date);
	printf("Message for consumer with PID %i.\n", pBuffElement->data.killFlag);
	printf("==================================\n");

    //signal
    sem_post(&(pBuffElement->mutex));

    return 1;
}

int tryWriteTerminateMessage(dataMessage message, char *bufferName, int index, int terminatorPid)
{
	sharedBuffer *pSharedBuffer = getSharedBuffer(bufferName);
	if (pSharedBuffer == NULL)
	{
		printf("Error, getSharedBuffer() failed.\n");
		return -1;
	}

	int bufferSize = pSharedBuffer->size;
	for (index; index % bufferSize < bufferSize; index++)
	{	
		index = index % bufferSize;
		if (!writeDataTerminateMessage(&(pSharedBuffer->bufferElements[index]), message, index, bufferName))
		{
			if (isBufferFull(pSharedBuffer))
			{
				// wake up consumers to they read the message written by terminator
				wakeup(bufferName, CONSUMER_ROLE);
				//doProcess(terminatorPid, STOP);
				//printf("Terminator with PID %i, just woke up.\n", terminatorPid);
			}

			continue;
		}
		else
		{
			// perform logging of the process
			printf("Terminator with PID %i, left a message for consumer with PID %i.\n", terminatorPid, message.killFlag);

			// wake up consumers to they read the message written by terminator
			wakeup(bufferName, CONSUMER_ROLE);

			index++;
			return 1;
		}
	}

	return 0;
}

// Removes a buffer
int removeBuffer(char *bufferName)
{
	// map shared memory to process address space
	sharedBuffer *pSharedBuffer = getSharedBuffer(bufferName);
	if (pSharedBuffer == NULL)
	{
		printf("Error, getSharedBuffer() failed.\n");
		return 0;
	}

    if (munmap(pSharedBuffer, STORAGE_SIZE) != 0)
    {
		printf("Error, munmap() failed, error code %d.\n", errno);
		return 0;
    }

    if (remove(bufferName) == -1)
	{
		printf("Error, remove() failed, error code %d.\n", errno);
		return 0;
	}

    return 1;
}


int killProducer(char *bufferName)
{
	sharedBuffer *pSharedBuffer = getSharedBuffer(bufferName);
	if (pSharedBuffer == NULL)
	{
		printf("Error, getSharedBuffer() failed.\n");
		return 0;
	}

	pSharedBuffer->killFlag = TERMINATE_SYSTEM; //SYSTEM WILL END
	pSharedBuffer->killerPID = getpid();
	// LOG  of this !!! 

	// wake up poducers to they read the message written by terminator
	wakeup(bufferName, PRODUCER_ROLE);

	return 1;
}

int killConsumer(char *bufferName)
{
	sharedBuffer *pSharedBuffer = getSharedBuffer(bufferName);
	if (pSharedBuffer == NULL)
	{
		printf("Error, getSharedBuffer() failed.\n");
		return 0;
	}
	
	//for(int i= 0; i < MAX_PIDS; i++)
	//{
	//	printf("[%i][%i]\n", pSharedBuffer ->PIDs.consumersPIDs[i][0], pSharedBuffer ->PIDs.consumersPIDs[i][1]);
	//}

	int index = 0;
	int terminatorPid = getpid();
	int consumerPid = -1;
	int loopCondition = 1;
	int writeResult = 0;

	dataMessage data;
	data.producerId = 0;
	data.key = 0;
	data.killerPID = terminatorPid;
	time_t rawTime;
	struct tm *infoTime;
	time(&rawTime);
	for (int i = 0; i < MAX_PIDS; i++)
	{
		consumerPid = pSharedBuffer->PIDs.consumersPIDs[i][0];
		if (NO_PID != consumerPid)
		{
			data.killFlag = consumerPid;
			loopCondition = 1;
			while (loopCondition)
			{
				infoTime = localtime(&rawTime);
				strftime(data.date, sizeof(data.date), "%x - %I:%M%p", infoTime);
				writeResult = tryWriteTerminateMessage(data, bufferName, &index, terminatorPid);
				if ((writeResult == -1) || (writeResult == 1))
				{
					loopCondition = 0;
				}
			}
		}
	}

	return 1;
}

