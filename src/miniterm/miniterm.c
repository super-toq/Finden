/* miniterm is part of the application "finden";
 * toq 2025  LICENSE: BSD 2-Clause "Simplified"
 *
 *
 *
 * gcc -g $(pkg-config --cflags gtk4 libadwaita-1) -o miniterm miniterm.c $(pkg-config --libs gtk4 libadwaita-1)
 *
 * Please note:
 * The Use of this code and execution of the applications is at your own risk, I accept no liability!
 *
 * Version 0.6.1 2025.11.09
 */

#include <gtk/gtk.h>
#include <adwaita.h>
#include <gdk/gdkkeysyms.h>

#define CMD_MAX_LEN 256

/* ---------- Globale Tags für Theme‑Handling ---------- */
static GtkApplication *global_app = NULL;
static GtkTextTag *light_theme_tag = NULL;
static GtkTextTag *dark_theme_tag  = NULL;

/* Funktionsprototypen … */
static void   on_entry_activate          (GtkEntry *entry,
                                          gpointer  user_data);
/*static gboolean on_key_pressed            (GtkEventControllerKey *controller,
                                          guint keyval,
                                          guint keycode,
                                          GdkModifierType state,
                                          gpointer user_data); */
static void   on_color_scheme_changed    (AdwStyleManager *manager,
                                          GParamSpec *pspec,
                                          gpointer user_data);
static void   execute_command_async      (const gchar *command,
                                          GtkTextView *view);
static void   read_line_cb               (GObject      *source_object,
                                          GAsyncResult *res,
                                          gpointer      user_data);   /* neue Zeile */

/* ----- Ursprünglich verwendeter Themenwechsel, abgespeckt ----- */
static void create_theme_tags (GtkTextBuffer *buf)
{
    light_theme_tag = gtk_text_buffer_create_tag (buf,
        "light_theme",
//        "foreground", "#000000",
//        "background", "#ffffff",
        "font", "monospace 10",
        NULL);

    dark_theme_tag = gtk_text_buffer_create_tag (buf,
        "dark_theme",
//        "foreground", "#ffffff",
//        "background", "#000000",
        "font", "monospace 10",
        NULL);
}

/* ---------- Hilfsfunktion: Text an TextView anhängen ---------- */
static void
append_text (GtkTextView *text_view, const char *text)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (text_view);
    GtkTextIter    end;
    gtk_text_buffer_get_end_iter (buffer, &end);

    AdwStyleManager *style_manager = adw_style_manager_get_default ();
    GtkTextTag *current_tag = adw_style_manager_get_dark (style_manager)
                              ? dark_theme_tag
                              : light_theme_tag;
    if (!current_tag) current_tag = light_theme_tag;   /* safety */

    gtk_text_buffer_insert_with_tags (buffer, &end, text, -1,
                                      current_tag, NULL);
    gtk_text_buffer_insert (buffer, &end, "\n", -1);
}
/* ---------- Callback Theme‑Änderungen ---------- */
static void on_color_scheme_changed(AdwStyleManager *manager,
                                     GParamSpec *pspec,
                                     gpointer user_data) {
    // Standard-GNOME-Farben:
    GtkTextView *text_view = GTK_TEXT_VIEW(user_data);
}

/* ---------- Asynchronen Befehl starten ---------- */
static void
execute_command_async (const gchar *command, GtkTextView *view)
{
    gchar **argv = NULL;
    GError *error = NULL;

    /* Shell‑Parsing */
    if (!g_shell_parse_argv (command, NULL, &argv, &error)) {
        gchar *msg = g_strdup_printf ("Parse‑Fehler: %s", error->message);
        append_text (view, msg);
        g_free (msg);
        g_error_free (error);
        return;
    }

    GSubprocess *proc = g_subprocess_newv (
        (const gchar * const *)argv,
        G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_MERGE,
        &error);
    g_strfreev (argv);

    if (!proc) {
        gchar *msg = g_strdup_printf ("Start‑Fehler: %s",
                                      error ? error->message : "Unbekannter Fehler");

        append_text (view, msg);
        g_free (msg);
        if (error) g_error_free (error);
        return;
    }
    /* Prozess Ablauf: */
    append_text(view, "Prozess läuft …");
    /* Ausgabestream holen und erste Zeile asynchron lesen */
    GInputStream      *out = g_subprocess_get_stdout_pipe (proc);
    GDataInputStream  *din = g_data_input_stream_new (out);
    g_object_unref (out);               /* nur den DataInputStream behalten */

    g_data_input_stream_read_line_async (din,
                                          G_PRIORITY_DEFAULT,
                                          NULL,
                                          read_line_cb,
                                          view);
}

/* ---------- Callback read-line, jede gelesene Zeile verarbeiten ---------- */
static void
read_line_cb (GObject *source_object,
              GAsyncResult *res,
              gpointer user_data)
{
    GDataInputStream *din  = G_DATA_INPUT_STREAM (source_object);
    GtkTextView      *view = GTK_TEXT_VIEW (user_data);
    GError           *error = NULL;
    gchar            *line = g_data_input_stream_read_line_finish (din,
                                                                   res,
                                                                   NULL,
                                                                   &error);

    if (line) {
        append_text (view, line);
        g_free (line);

        /* Nächste Zeile lesen */
        g_data_input_stream_read_line_async (din,
                                              G_PRIORITY_DEFAULT,
                                              NULL,
                                              read_line_cb,
                                              view);
    } else {
        if (error) {
            g_printerr ("Fehler beim Lesen: %s\n", error->message);
            g_error_free (error);
        }
        g_object_unref (din);   /* Stream freigeben */
    }
}

/* ---------- Callback nach Eingabe der Enter‑Taste: ---------- */
static void
on_entry_activate (GtkEntry *entry, gpointer user_data)
{
    const gchar *cmd = gtk_editable_get_text (GTK_EDITABLE (entry));
    GtkTextView *tv  = GTK_TEXT_VIEW (user_data);

    /* ---- "exit"-Eingabe beendet das Programm ---- */
    if (g_strcmp0 (cmd, "exit") == 0) {
        if (global_app)
            g_application_quit (G_APPLICATION (global_app));
        return;
    }

    /* vorherigen Inhalt leeren */
    gtk_text_buffer_set_text (gtk_text_view_get_buffer (tv), "", -1);

    execute_command_async (cmd, tv);
    gtk_editable_set_text (GTK_EDITABLE (entry), "");
}
/* ---------- Tastatur‑Callback ---------- */
//static gboolean
//on_key_pressed (GtkEventControllerKey *controller,
//                guint keyval,
//                guint keycode,
//                GdkModifierType state,
//                gpointer user_data)
//{
//    GtkTextView *view = GTK_TEXT_VIEW (user_data);

//    if (keyval == GDK_KEY_Return) {
//        append_text (view, "Enter gedrückt");
//        return TRUE;   /* Event verarbeitet */
//    }
//    return FALSE;      /* Weiterreichen */
//}

/* ----- Funktion zum Verarbeiten von Kommandozeilenargumenten ----- */
static int
on_command_line (GtkApplication *app,
                 GApplicationCommandLine *cmdline,
                 gpointer user_data)
{
    char **argv;
    int   argc;

    argv = g_application_command_line_get_arguments (cmdline, &argc);

    if (argc > 1) {
        GString *command = g_string_new (NULL);

        for (int i = 1; i < argc; ++i) {
            if (g_strcmp0 (argv[i], "--") == 0)
                continue;               /* ignorieren */

            /* Quote, falls nötig */
            gboolean need_quotes = FALSE;
            const char *p = argv[i];
            while (*p) {
                if (g_ascii_isspace (*p) || strchr ("\"'\\$`", *p))
                    need_quotes = TRUE;
                ++p;
            }

            if (need_quotes) {
                gchar *escaped = g_shell_quote (argv[i]);
                g_string_append (command, escaped);
                g_free (escaped);
            } else {
                g_string_append (command, argv[i]);
            }

            if (i < argc - 1)
                g_string_append_c (command, ' ');
        }

        char *cmd_str = g_string_free (command, FALSE);
        g_object_set_data_full (G_OBJECT (app),
                                "initial-command",
                                cmd_str,
                                g_free);
    }

    g_strfreev (argv);
    g_application_activate (G_APPLICATION (app));
    return 0;
}
/* ---------- Hilfsfunktion setze Fokus auf Eingabefeld (Mausklick) ---- */
static gboolean
focus_entry_idle (gpointer user_data)
{
    GtkWidget *entry = GTK_WIDGET (user_data);
    gtk_widget_grab_focus (entry);
    return G_SOURCE_REMOVE;   /* Callback nur einmal ausführen */
}
/* ---------- Callback für Mausklick auf Ausgabefenster ------ */
static void
on_text_view_clicked (GtkGestureClick *gesture,
                      gint             n_press,
                      gdouble          x,
                      gdouble          y,
                      gpointer         user_data)
{
    GtkWidget *entry = GTK_WIDGET (user_data);
//  g_print ("Klick auf Ausgabefeld erkannt!\n"); /* testen */
//    gtk_widget_grab_focus (entry); funktioniert nicht, daher: */
    g_idle_add (focus_entry_idle, entry); /* Fokus wird im nächsten Idle‑Durchlauf setzen */
}

/* ---------- Aktivierungs‑Callback der GtkApplication ---------- */
static void
on_app_activate (GtkApplication *app, gpointer user_data)
{
    /* ----- Fenster ----- */
    AdwApplicationWindow *win = ADW_APPLICATION_WINDOW (
        adw_application_window_new (app));
    gtk_window_set_default_size (GTK_WINDOW (win), 1000, 480);

    /* ----- ToolbarView (Root‑Widget) ----- */
    AdwToolbarView *toolbar_view = ADW_TOOLBAR_VIEW (adw_toolbar_view_new ());
    adw_application_window_set_content (win, GTK_WIDGET (toolbar_view));

    /* ----- HeaderBar ----- */
    AdwHeaderBar *header = ADW_HEADER_BAR (adw_header_bar_new ());
    GtkLabel *title_label = GTK_LABEL (gtk_label_new (NULL));
    gtk_label_set_markup (title_label, "<b>miniterm</b>");
    adw_header_bar_set_title_widget (header, GTK_WIDGET (title_label));
    adw_toolbar_view_add_top_bar (toolbar_view, GTK_WIDGET (header));

    /* ----- Haupt‑Layout (Box) ----- */
    GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    adw_toolbar_view_set_content (toolbar_view, box);

    /* ----- TextView + Tags ----- */
    GtkWidget *text_view = gtk_text_view_new ();
    gtk_text_view_set_editable (GTK_TEXT_VIEW (text_view), FALSE);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (text_view), FALSE);
    gtk_text_view_set_monospace (GTK_TEXT_VIEW (text_view), TRUE);
    GtkTextBuffer *buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
    create_theme_tags (buf);   /* erzeugt light_theme_tag / dark_theme_tag */

    /* ----- ScrolledWindow ----- */
    GtkWidget *scroller = gtk_scrolled_window_new ();
    gtk_widget_set_hexpand (scroller, TRUE);
    gtk_widget_set_vexpand (scroller, TRUE);
    gtk_widget_set_name (scroller, "output_scroller");
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroller), text_view);
    gtk_box_append(GTK_BOX(box), scroller); // TextView wird jetzt hier hinzugefügt

    /* ----- Entry deklarieren für Befehle (Eingabefeld) ----- */
    GtkWidget *entry = gtk_entry_new ();
    gtk_entry_set_placeholder_text (GTK_ENTRY (entry), "Befehl eingeben...");
    gtk_box_append (GTK_BOX (box), entry); // Entry nach dem TextView hinzufügen
    /* ----- Signal für Enter-Taste drücken ----- */
    g_signal_connect (entry, "activate",
                      G_CALLBACK (on_entry_activate), text_view);

    /* ----- Gesture‑Controller anlegen ----- */
    GtkGestureClick *click_gesture = GTK_GESTURE_CLICK (gtk_gesture_click_new ());
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (click_gesture),
                                   GDK_BUTTON_PRIMARY);   /* Linksklick */
    g_signal_connect (click_gesture, "pressed",
                      G_CALLBACK (on_text_view_clicked),
                      entry);                     /* entry wird als user_data übergeben */
    /* ----- Controller dem TextView hinzufügen ----- */
    gtk_widget_add_controller (text_view, GTK_EVENT_CONTROLLER (click_gesture));

    /* Fokus‑Widget für Theme‑Callback hinterlegen (optional) */
    AdwStyleManager *style_manager = adw_style_manager_get_default ();
    g_object_set_data (G_OBJECT (style_manager), "entry-widget", entry);

    /* ----- CSS (für Hintergrund) ----- */
    GtkCssProvider *css = gtk_css_provider_new ();
/* -- DEAKTIVIERT:
      gtk_css_provider_load_from_string (css,
        "scrolledwindow#output_scroller {"
        "  background-color: #ffffff;"
        "  border-radius: 6px;"
        "} "
        "scrolledwindow#output_scroller textview {"
        "  background-color: #ffffff;"
        "}");
    gtk_style_context_add_provider_for_display (
        gdk_display_get_default (),
        GTK_STYLE_PROVIDER (css),
        GTK_STYLE_PROVIDER_PRIORITY_USER); 
    g_object_unref (css); 
-- */

    /* ----- Theme‑Änderungen überwachen ----- */
    g_signal_connect (style_manager, "notify::color-scheme",
                      G_CALLBACK (on_color_scheme_changed), text_view);

    /* ----- Fenster sichtbar machen ----- */
    gtk_window_present (GTK_WINDOW (win));
    global_app = app; 

    /* ----- Eingabefeld fokussieren ----- */
    gtk_widget_grab_focus (entry);

    /* ----- Initial Befehl ausführen, falls vorhanden ----- */
    const char *initial_command = g_object_get_data (G_OBJECT (app), "initial-command");
    if (initial_command && *initial_command) {
        /* ---- "exit" → sofort beenden ---- */
        if (g_strcmp0 (initial_command, "exit") == 0) {
            g_application_quit (G_APPLICATION (app));
    } else {
        execute_command_async (initial_command, GTK_TEXT_VIEW (text_view));
    }
    g_object_set_data (G_OBJECT (app), "initial-command", NULL);
    }
}

/* -------------------------------------------------------------
   main()
   ------------------------------------------------------------- */
int
main (int argc, char **argv)
{
    adw_init ();   /* vor Gtk Anwendung */

    GtkApplication *app = gtk_application_new (
        "free.toq.miniterm",
        G_APPLICATION_HANDLES_COMMAND_LINE);

    if (!app) {
        g_printerr ("Fehler: Konnte GtkApplication nicht erstellen\n");
        return 1;
    }

    g_signal_connect (app, "activate",
                      G_CALLBACK (on_app_activate), NULL);
    g_signal_connect (app, "command-line",
                      G_CALLBACK (on_command_line), NULL);

    int status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);
    return status;
}
