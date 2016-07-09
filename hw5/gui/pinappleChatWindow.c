
#include <gtk/gtk.h>
//#include <glade/glade.h>

static void run_gui(GtkApplication *app, gpointer user_data){
  GtkWidget* box;
  GtkWidget* button;
  GtkWidget* text_view;
  GtkWidget* text_field;
  GtkWidget* window;
  //GtkWidget* button_box;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window),"Halakahiki");
  gtk_window_set_default_size(GTK_WINDOW(window),800,600);

  button = gtk_button_new_with_label("Enter");
  text_view = gtk_text_view_new();
  //gtk_text_view_set_top_margin (GTK_Object(text_view), 400);
  gtk_text_view_set_pixels_below_lines (GTK_Object(text_view),400);
  text_field = gtk_entry_new();
  //gtk_entry_set_max_length (text_field, 1024);

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL,5);
  gtk_container_add(GTK_CONTAINER (window), box);
  gtk_container_add(GTK_CONTAINER(box), text_view);
  gtk_container_add(GTK_CONTAINER(box), text_field);
  gtk_container_add(GTK_CONTAINER(box), button);

  gtk_widget_show_all (window);
}


int main (int argc, char **argv){
  GtkApplication *app;
    int status;

    app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (run_gui), NULL);
    status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);

    return status;

}
