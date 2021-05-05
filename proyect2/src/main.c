#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "backend/include/consumer.h"
#include "backend/include/common.h"
#include "backend/include/factory.h"
#include "backend/include/finisher.h"
#include "backend/include/producer.h"
#if defined(CREATOR_APP)
#include "frontend/include/gui.h"
#endif

#if defined(CREATOR_APP)

int main(int argc, char  *argv[])
{
	printf("CREATOR_APP.\n");

	// simple test, just for review functionality
	int bufferSize = 3;
	int bufferId = 0;
	if (!createSharedBuffer(bufferSize, bufferId))
	{
		printf("Error, creating shared buffer.\n");
		return 1;
	}

	while (1);

    // ends

	return 0;
}

#elif defined(PRODUCER_APP)

int main(int argc, char  *argv[])
{
	// sanity check
	if (!argv)
	{
		printf("Error: argv is NULL.\n");
		return 1;
	}

	int PRODUCER_ARGS_NUM = 3;

	if (argc != PRODUCER_ARGS_NUM)
	{
		printf("Usage: $ %s Buffer_ID(int) Average_Time_To_Send(int)\n", argv[0]);
		return 1;
	}

	int bufferId = atoi(argv[1]);
	double averageTime = atoi(argv[2]);

	double lambda = 1/averageTime;

	// Intializes time for random number generator
	time_t t;
	srand((unsigned)time(&t));

	int waitTime = 0;


	// simple test, just for review functionality
	printf("PRODUCER_APP.\n");
	dataMessage data;
	data.producerId = 80;
	data.key = 3;

	time_t rawTime;
	struct tm *infoTime;
	time(&rawTime);
	infoTime = localtime(&rawTime);
	strftime(data.date, sizeof(data.date), "%x - %I:%M%p", infoTime);

	char sharedBufferName[50];
	strcpy(sharedBufferName, getBufferName(SHARED_BUFFER_NAME, bufferId));

    while (1)
    {
		waitTime = ceil(randomExponentialDistribution(lambda));
		sleep(waitTime);

		infoTime = localtime(&rawTime);
		strftime(data.date, sizeof(data.date), "%x - %I:%M%p", infoTime);
		tryWrite(data, sharedBufferName);
    }


	return 0;
}

#elif defined(CONSUMER_APP)

int main(int argc, char  *argv[])
{
	// simple test, just for review functionality
	printf("CONSUMER_APP.\n");
    // ends

	return 0;
}

#elif defined(TERMINATOR_APP)

int main(int argc, char  *argv[])
{
	// simple test, just for review functionality
	printf("TERMINATOR_APP.\n");

	int bufferId = 0;
	char sharedBufferName[50];
	strcpy(sharedBufferName, getBufferName(SHARED_BUFFER_NAME, bufferId));
	if (!removeBuffer(sharedBufferName))
	{
		printf("Error, removing buffer.\n");
		return 1;
	}
    // ends

	return 0;
}

#endif

