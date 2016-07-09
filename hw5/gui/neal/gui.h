#ifndef gui_h
#define gui_h

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

void printToView(gchar* text);

void printToViewInBlue(char* text);

void printToViewInPink(char* text);

void printToViewInRed(char* text);

void clickedSend(GtkWidget* entry);

#endif