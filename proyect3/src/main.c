#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>

#include "backend/include/beamer.h"
#include "backend/include/common.h"
#include "backend/include/rateMonothonicScheduler.h"
#include "backend/include/earliestDeadlineFirstScheduler.h"
#include "backend/include/leastLaxityFirstScheduler.h"
#include "frontend/include/gui.h"

int *_executionTime;
int *_remainTime;
int *_timeLine;
int _numProcesses;

int _rmFlag = 0;
int _edfFlag = 0;
int _llfFlag = 0;


rateMonothonic *_pRateMonothonic;
earliestDeadlineFirst *_pEarliestDeadlineFirst;
leastLaxityFirst *_pLeastLaxityFirst;

// sincronizes GUI elements
void getDataFromGUI()
{
	_rmFlag = _selectedRM;
	_edfFlag = _selectedEDF;
	_llfFlag = _selectedLLF;
	_mixedSlidesFlag = _mixedSlides? 1: 0;
	_numProcesses = _numTasks;
	_executionTime = (int*)malloc(_numProcesses * sizeof(int));
	_remainTime = (int*)malloc(_numProcesses * sizeof(int));
	_timeLine = (int*)malloc(_numProcesses * sizeof(int));
	for (int i = 0; i < _numProcesses; i++)
	{
		_executionTime[i] = tasksGUI[i].executionTime;
		_remainTime[i] = _executionTime[i];
		_timeLine[i] = tasksGUI[i].periodTime;
	}

	exit_app();
/*
	// Initializes time for random number generator: dummy for testing
	time_t t;
	srand((unsigned)time(&t));

	// dummy by the moment
	_numProcesses = MAX_PROCESS;
	int tmp;
	for (int i = 0; i < _numProcesses; i++)
	{
		_executionTime[i] = (rand() % MAX_TIME_UNITS) + 1;
		_remainTime[i] = _executionTime[i];
		tmp = (rand() % MAX_TIME_UNITS) + _executionTime[i];
		_timeLine[i] = (tmp <= MAX_TIME_UNITS) ? tmp : MAX_TIME_UNITS;
	}
*/
}


void runRateMonothonicScheduler()
{
	if (populateRMProcessInfo(_pRateMonothonic, _numProcesses, _executionTime, _timeLine, _remainTime) == ERROR)
	{
		printf("Error: while running populateRMProcessInfo().\n");
		_pRateMonothonic = NULL;
	}

	if (!_pRateMonothonic)
	{
		printf("Error: _pRateMonothonic is NULL.\n");
		exit(1);
	}

	int observationTimeRM = getObservationTime(_pRateMonothonic->period, _pRateMonothonic->numProcesses);
	_cycles = observationTimeRM;
	if (observationTimeRM == ERROR)
	{
		printf("Error: while running getObservationTime().\n");
		exit(1);
	}

	if (rateMonothonicScheduler(_pRateMonothonic, observationTimeRM) == ERROR)
	{
		printf("Error: while running RateMonothonic algorithm.\n");
		exit(1);
	}
}


void runEarliestDeadlineFirstScheduler()
{
	if (populateEDFProcessInfo(_pEarliestDeadlineFirst, _numProcesses, _executionTime, _timeLine, _remainTime) == ERROR)
	{
		printf("Error: while running populateEDFProcessInfo().\n");
		_pEarliestDeadlineFirst = NULL;
	}

	if (!_pEarliestDeadlineFirst)
	{
		printf("Error: _pEarliestDeadlineFirst is NULL.\n");
		exit(1);
	}

	int observationTimeEDF = getObservationTime(_pEarliestDeadlineFirst->deadline, _pEarliestDeadlineFirst->numProcesses);
	_cycles = observationTimeEDF;
	if (observationTimeEDF == ERROR)
	{
		printf("Error: while running getObservationTime().\n");
		exit(1);
	}

	if (earliestDeadlineFirstScheduler(_pEarliestDeadlineFirst, observationTimeEDF, _timeLine, _executionTime) == ERROR)
	{
		printf("Error: while running EarliestDeadlineFirst algorithm.\n");
		exit(1);
	}
}


void runLeastLaxityFirstScheduler()
{
	if (populateLLFProcessInfo(_pLeastLaxityFirst, _numProcesses, _executionTime, _timeLine, _remainTime) == ERROR)
	{
		printf("Error: while running populateLLFProcessInfo().\n");
		_pLeastLaxityFirst = NULL;
	}

	if (!_pLeastLaxityFirst)
	{
		printf("Error: _pLeastLaxityFirst is NULL.\n");
		exit(1);
	}

	int observationTimeLLF = getObservationTime(_pLeastLaxityFirst->deadline, _pLeastLaxityFirst->numProcesses);
	_cycles = observationTimeLLF;
	if (observationTimeLLF == ERROR)
	{
		printf("Error: while running getObservationTime().\n");
		exit(1);
	}

	if (leastLaxityFirstScheduler(_pLeastLaxityFirst, observationTimeLLF, _timeLine, _executionTime) == ERROR)
	{
		printf("Error: while running leastLaxityFirstScheduler algorithm.\n");
		exit(1);
	}
}


int main(int argc, char *argv[])
{
	// sanity check
	if (!argv)
	{
		printf("Error: argv is NULL.\n");
		exit(1);
	}

	_pRateMonothonic = (rateMonothonic*)malloc(sizeof(rateMonothonic));
	_pEarliestDeadlineFirst = (earliestDeadlineFirst*)malloc(sizeof(earliestDeadlineFirst));
	_pLeastLaxityFirst = (leastLaxityFirst*)malloc(sizeof(leastLaxityFirst));
	RMData.isDone = 1;
	EDFData.isDone = 1;
	LLFData.isDone = 1;

	void (*ptr)() = &getDataFromGUI;
	_ptrGetFromGUI = ptr;
	runGUI(argc, argv);

	getDataFromGUI();

	createPresentation();
	if (_edfFlag && !_mixedSlidesFlag)
	{	
		definitionEDF();
	}

	if (_rmFlag && !_mixedSlidesFlag)
	{	
		definitionRM();
	}
	
	if (_llfFlag && !_mixedSlidesFlag)
	{
		definitionLLF();
	}

	if(!_mixedSlidesFlag)
	{
		SchedulabilityTest(_executionTime, _timeLine, _numProcesses);
	}

	if (_edfFlag)
	{	
		EDFData.isValid = 1;
		EDFData.isDone = 0;
		runEarliestDeadlineFirstScheduler();
	}

	if (_rmFlag)
	{	
		RMData.isValid = 1;
		RMData.isDone = 0;
		runRateMonothonicScheduler();
	}
	
	if (_llfFlag)
	{
		LLFData.isValid = 1;
		LLFData.isDone = 0;
		runLeastLaxityFirstScheduler();
	}

	if (!_edfFlag && !_rmFlag && !_llfFlag)
	{
		printf("No algorithm selected.\n");
	}

	if(_mixedSlidesFlag == 1)
	{
		simulationAllAlgorithm(_cycles);
	}
		
	finistPresentation();


        system("pdflatex -output-directory presentation presentation/outputBeamer.tex > /dev/null 2>&1");

	system ("evince presentation/outputBeamer.pdf &");

	// clean up section
	if (_pEarliestDeadlineFirst != NULL)
	{
		free(_pEarliestDeadlineFirst);
	}

	if (_pRateMonothonic != NULL)
	{
		free(_pRateMonothonic);
	}

	if (_pLeastLaxityFirst != NULL)
	{
		free(_pLeastLaxityFirst);
	}

	if (_executionTime != NULL)
	{
		free(_executionTime);
	}

	if (_remainTime != NULL)
	{
		free(_remainTime);
	}

	if (_timeLine != NULL)
	{
		free(_timeLine);
	}

	return 0;
}
