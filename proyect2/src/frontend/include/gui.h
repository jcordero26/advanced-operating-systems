

#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <string.h>
#include <time.h>

// typedef struct
// {
// 	int threadId;
// 	int progress;
// 	float piApprox;
// 	GtkWidget *threadIdLabel;
// 	GtkWidget *progressLabel;
// 	GtkWidget *progressBar;
// 	GtkWidget *piApproxLabel;
	
// } threadStruct; ///< Struct to handle threads information
#ifndef GUI_FILE
#define GUI_FILE


#define MAX_PIDS_GUI 50

typedef struct
{
	GtkWidget *indexLabel;
	GtkWidget *producerIdLabel;
	GtkWidget *dateLabel;
	GtkWidget *keyLabel;
	
} bufferElementStruct; ///< Struct to handle threads information


bufferElementStruct bufferGUI[MAX_PIDS_GUI];
int _bufferSizeGUI;
int _bufferId;

GtkWidget *producerCounterLabel;
GtkWidget *consumerCounterLabel;
GtkWidget *logContentLabel;
GtkWidget *logViewText;

//void updateGUI(int threadId, int progress, float piApprox, int totalProgress, float totalPiApprox);

void runGUI(int argc, char **argv, int bufferId);

gboolean time_handler(GtkWidget *widget);

void exit_app();


//Pointers to update the GUI
// int _pGuiThreadId;
// int _pGuiThreadProgress;
// int _pGuiTotalProgress;
// float _pGuiPiApprox;

void (*_ptrUpdateGUI)();

#endif
