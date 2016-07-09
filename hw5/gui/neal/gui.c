#include "gui.h"

GtkWidget* textView;
void printToView(const gchar* text){
	GtkTextBuffer* viewbuff = gtk_text_view_get_buffer( (GtkTextView*) textView);
	GtkTextIter insertion;
	gtk_text_buffer_get_end_iter (viewbuff, &insertion);
	gtk_text_buffer_insert (viewbuff, &insertion, text, strlen(text));
}

void printToViewInBlue(char* text){

}

void printToViewInPink(char* text){

}

void printToViewInRed(char* text){

}

int t = 0;
void clickedSend(GtkWidget* entry){
	printToView(gtk_entry_get_text ( (GtkEntry*) entry));
	gtk_entry_set_text ( (GtkEntry*) entry, "" );
}

void on_window_destroy (GtkWidget *object, gpointer user_data){
        gtk_main_quit();
}


//gcc gui.c -o gui -rdynamic `pkg-config --cflags --libs gtk+-3.0`
int main(int argc, char *argv[]){

	GtkWidget* window;
	
	GtkBuilder* builder;

	gtk_init(&argc, &argv);

	builder = gtk_builder_new ();
	gtk_builder_add_from_file(builder, "pineapple.glade", NULL);

	window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	textView = GTK_WIDGET(gtk_builder_get_object(builder, "mainView"));

	gtk_builder_connect_signals(builder, NULL);
	g_object_unref(G_OBJECT(builder));

	gtk_widget_show(window);
	gtk_main();

	return 0;
}

