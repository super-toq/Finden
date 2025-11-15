/* FINDEN is part of a learning project;
 * toq 2025  LICENSE: BSD 2-Clause "Simplified"
 *
 *
 *
 * gcc $(pkg-config --cflags gtk4 libadwaita-1) -o finden main.c $(pkg-config --libs gtk4 libadwaita-1)
 *
 * Please note:
 * The Use of this code and execution of the applications is at your own risk, I accept no liability!
 *
 * Version 0.6.5
 */
#include <glib.h>
#include <gtk/gtk.h>
#include <adwaita.h>
#include <locale.h> 
#include <glib/gi18n.h>
//#include "finden.gresource.h"   /* enthält resources_get_resource() */

/* globale Referenz, wird beim UI-Aufbau gesetzt */
static GtkCheckButton *root_check = NULL;
static gboolean dialog_finished = FALSE;


/* ----- Message / Alert-Dialog Generisch,  show_alert_dialog (parent,*Titel, *Inhalttext) ----- */
static void on_alert_dialog_response (AdwAlertDialog *dialog,
                          const char     *response,
                          gpointer        user_data)
{
    if (g_strcmp0 (response, "ok") == 0)
        g_print ("Dialog btn - ok\n");
    else
        g_print ("Dialog btn - cancel\n");

    /* **Wichtig:** hier kein g_object_unref(dialog) ! */
}

/* ----- Callback Alert-Dialog anzeigen (generisch) ----- */
static void
show_alert_dialog (GtkWindow   *parent,
                   const char  *title,
                   const char  *body)
{
    if (!parent || !GTK_IS_WINDOW (parent)) {
        g_warning (_("Kein gültiges Elternfenster für Alert-Dialog"));
        return;
    }

    /* Dialog erzeugen – Titel und Body werden übergeben */
    AdwAlertDialog *dialog = ADW_ALERT_DIALOG (adw_alert_dialog_new (title, body));

    /* Buttons hinzufügen */
    adw_alert_dialog_add_response (dialog, "cancel", _("Abbrechen"));
    adw_alert_dialog_add_response (dialog, "ok",     "OK");
    adw_alert_dialog_set_default_response (dialog, "ok");

    /* Antwort‑Signal verbinden */
    g_signal_connect (dialog, "response",
                      G_CALLBACK (on_alert_dialog_response), NULL);

    /* Dialog präsentieren */
    adw_dialog_present (ADW_DIALOG (dialog), GTK_WIDGET (parent));
}

/* ---- Callback: About-Dialog öffnen ----- */
static void show_about (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    AdwApplication *app = ADW_APPLICATION (user_data);
    /* About‑Dialog anlegen */
    AdwAboutDialog *about = ADW_ABOUT_DIALOG (adw_about_dialog_new ());
    //adw_about_dialog_set_body(about, "Hierbei handelt es sich um ein klitzekleines Testprojekt."); //nicht in meiner adw Version?
    adw_about_dialog_set_application_name (about, "Finden");
    adw_about_dialog_set_version (about, "0.6.5");
    adw_about_dialog_set_developer_name (about, "toq (super-toq)");
    adw_about_dialog_set_website (about, "https://github.com/super-toq");

    /* Lizenz – MIT wird als „custom“ angegeben */
    adw_about_dialog_set_license_type (about, GTK_LICENSE_CUSTOM);
    adw_about_dialog_set_license (about,
        "BSD 2-Clause License\n\n"
        "Copyright (c) 2025, toq\n"
        "Redistribution and use in source and binary forms, with or without "
        "modification, are permitted provided that the following conditions are met: "
        "1. Redistributions of source code must retain the above copyright notice, this "
        "list of conditions and the following disclaimer.\n"
        "2. Redistributions in binary form must reproduce the above copyright notice, "
        "this list of conditions and the following disclaimer in the documentation"
        "and/or other materials provided with the distribution.\n\n"
        "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ''AS IS'' "
        "AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE "
        "IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE "
        "DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE "
        "FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL "
        "DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR "
        "SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER "
        "CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, "
        "OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE "
        "OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n\n"
        "Application Icon by SVG. \n"
        "https://www.svgrepo.com \n"
        "Respect and thanks to SVG for free use. \n"
        "LICENSE for the icon: \n"
        "CC Attribution License \n"
        "Follow the link to view details of the CC Attribution License: \n"
        "https://www.svgrepo.com/page/licensing/#CC%20Attribution \n");

//    adw_about_dialog_set_translator_credits (about, "toq: deutsch\n toq: englisch");
    adw_about_dialog_set_application_icon (about, "free.toq.finden");   //IconName

    /* Dialog innerhalb (modal) des Haupt-Fensters anzeigen */
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(
    gtk_application_get_active_window(GTK_APPLICATION(app)) )));
    adw_dialog_present(ADW_DIALOG(about), GTK_WIDGET(parent));

}

/* ----- Callback Beenden-Button ----- */
static void on_quitbtn_clicked (GtkButton *button, gpointer user_data)
{
    g_application_quit (G_APPLICATION (user_data));
}

/* ----- Callback Suchfunktion ausführen | Hauptfunktion --------------------------------- */
static void on_search_button_clicked (GtkButton *button, gpointer user_data)
{
    GtkEntry *search_entry = GTK_ENTRY (user_data);
    const gchar *query = gtk_editable_get_text (GTK_EDITABLE (search_entry));

    if (!query || *query == '\0') {
        g_warning (_("Bitte einen Suchbegriff eingeben."));
        return;
    }

    /* 1. ---- Tool-"find"‑ermitteln ---------------------------------------------------- */
    const gchar *find_prog = "find";
    gchar *find_path = g_find_program_in_path (find_prog);
    if (!find_path) {
        const gchar *fallback = "/usr/bin/find";
        if (g_file_test (fallback, G_FILE_TEST_IS_EXECUTABLE))
            find_path = g_strdup (fallback);
    }
    if (!find_path) {
        g_warning (_("Abbruch, Tool %s wurde nicht gefunden!"), find_prog);
        return;
    }

    /* 2. ---- Prüfen, ob die Root‑Checkbox aktiv ist ----------------------------------- */
    gboolean root_active = FALSE;
    if (GTK_IS_CHECK_BUTTON (root_check))
        root_active = gtk_check_button_get_active (root_check);
    g_print (_("Root-Schalter %s\n"), root_active ? _("aktiviert") : _("nicht aktiviert"));

    /* 3. ---- Wenn Checkbox Root aktiviert wurde, führe nächsten Schritt aus, anderenfalls... */
    if (root_active) 
    {
         /* 3.1. --- App ist Flatpak-Version ---- */
         if (getenv("FLATPAK_ID")) 
         {
         /* --- ? --- */


         } else // App ist nicht Flatpak-Version, weiter mit 3.2
          {
             /* 3.2. ---- Prüfe, ob systemd-run0 existiert ----------------------------------------- */
             if (!g_file_test ("/usr/bin/systemd-run", G_FILE_TEST_EXISTS)) 
             {
                 g_print(_("Service run0 ist nicht aktiv. Bitte überprüfen Sie Ihre Systemkonfiguration.\n"));
                 gtk_check_button_set_active(root_check, FALSE);            // setze Checkbox auf unangeklickt
                 gtk_widget_set_sensitive(GTK_WIDGET (root_check), FALSE);  // setze Checkbox auf deaktiviert
                 root_active = FALSE;                                       // zuvor ermittelter Wert zurückgesetzt
                 /* ---- Alert Dialog erzeugen ---- */
                 GtkWindow *parent = GTK_WINDOW (gtk_widget_get_root (GTK_WIDGET (search_entry)));
                 if (parent) 
                  {
                     show_alert_dialog (parent,
                               _("Run0 nicht aktiviert"),
                               _("Der Service Run0 ist nicht aktiv,\n"
                                 "Bitte überprüfen Sie Ihre Systemkonfiguration.\n"));
                  }
              } 
                else 
                   {
                      g_print (_("Service run0 vorhanden\n"));
                   }
           } // beendet aus else zu App ist Flatpak-Version
    } else {
    g_print("Placeholder(1) \n"); // Platzhalter
    }

    /* 4. ---- Kommando für "find" zusammenbauen ---- */
    gchar *find_cmd;
    /* 4.1 - bei aktiver Root-Checkbox sowie Run0, benutze Kommando, anderenfalls 4.2 - */
    if (root_active) {
        find_cmd = g_strdup_printf ("run0 --background=0 --unit=finden --via-shell %s / -iname \"*%s*\"",
                                    find_path,
                                    query);
    /* 4.2 ---- bei nicht aktiver "Root"-Checkbox folgendes Kommando benutzen ----------*/
    } else {
        find_cmd = g_strdup_printf ("%s %s -iname \"*%s*\"",
                                    find_path, g_get_home_dir (), query);
    }
    /* 5. --- grep einbauen --- */
     // grep hier einbauen !!!!

    /* 6. ---- Terminal im System ermitteln ---- */
    gchar *exe_path = g_file_read_link ("/proc/self/exe", NULL);
    if (!exe_path) {
        g_warning (_("Der exe_path kann nicht erstellt werden"));
        g_free (find_cmd);
        return;
    }
    gchar *app_dir = g_path_get_dirname (exe_path);
    g_free (exe_path);

    /* Erstelle Pfad zu miniterm */
    gchar *mini_path = g_build_filename (app_dir, "miniterm", NULL);
    g_free (app_dir);

    gchar *term_path = NULL;
    const gchar *term_name = NULL;

    if (g_file_test (mini_path, G_FILE_TEST_IS_EXECUTABLE)) {
        term_path = g_strdup (mini_path);
        term_name = "free.toq.miniterm";
        g_print (_("%s gefunden in %s\n"), term_name, term_path);
    } else {
        static const gchar *terminals[] = {
            "free.toq.miniterm",
            "konsole",
            "gnome-terminal",
            "kgx",
            "xterm",
            "terminator",
            "kitty",
            NULL
        };
        for (int i = 0; terminals[i] != NULL; i++) {
            term_path = g_find_program_in_path (terminals[i]);
            if (term_path) {
                term_name = terminals[i];
                g_print (_("%s gefunden in %s\n"), term_name, term_path);
                break;
            }
        }
    }
    g_free (mini_path);

    if (!term_path) {
        g_warning (_("Kein unterstütztes Terminal gefunden!"));
        GtkWindow *parent = GTK_WINDOW (gtk_widget_get_root (GTK_WIDGET (button)));
        if (parent)
            show_alert_dialog (parent,
                               _("Kein Terminal gefunden"),
                               _("Es konnte kein unterstütztes Terminal auf diesem System gefunden werden."));
        g_free (find_cmd);
        return;
    }

    /* 7. ---- Terminal starten ---------------------------------------------------------------------- */
    /* mit "exec bash" neuer Bash-Shell starten, damit Term. aktiv halten */
    gchar *full_cmd = g_strdup_printf ("%s; exec bash", find_cmd);
//    g_print ("full_cmd: %s\n", full_cmd);  //testen

    /* ----- Argumentliste vorbereiten --------------------*/
    /* argv = "/usr/bin/gnome-terminal -- bash -c "/usr/bin/find / -iname \"*example*\"; exec bash"" */
    /* g_strdup   Funktion zum kopieren eines Strings in ein neu angel. Speicherplatz + Zeiger darauf */
    gchar *argv[] = {
        term_path,
        g_strdup ("--"),     //Option die sicherstellt, alle nachfolg. Arg. für das Terminal u. nicht für die Terminal-Umgebung interpr. werden.
        g_strdup ("bash"),  //auszuführendes Kommando
        g_strdup ("-c"),    // Option zum Anweisen der Bash, alle folgenden Kommandos als Argumente zu interpretieren
        full_cmd,
        NULL               //Array mit NULL derminieren
    };

    GError *error = NULL;
    gchar *cwd = g_get_current_dir ();   /* Arbeitsverzeichnis */
    if (!g_spawn_async (cwd, argv, NULL, G_SPAWN_DEFAULT, NULL, NULL, NULL, &error)) {
        g_warning (_("Fehler beim Starten des Terminals: %s"), error->message);
        g_error_free (error);
    }
    g_free (cwd);

    /* ---- Speicher freigeben  (v4) ---- */
    /* argv[1..n] wurden mit g_strdup erzeugt (argv[0] ist term_path, argv[3] = full_cmd) */
    for (int i = 1; argv[i] != NULL; i++)
        g_free (argv[i]);

    /* term_path war entweder g_strdup(mini_path) oder Rückgabe von g_find_program_in_path() */
    g_free (term_path);

    /* find_cmd und full_cmd einmalig freigeben (full_cmd wurde bereits durch argv-loop freigegeben!)*/
    g_free (find_cmd);  /* full_cmd) bereits via argv loop */

}

/* --------------------------------------------------------------------------- */
/*       Aktivierungshandler                                                   */
/* ----- CALLBACK-Funktion wird aufgerufen wenn Anwendung aktiviert wird ----- */
static void on_activate (AdwApplication *app, gpointer)
{
    /* ----- Adwaita-Fenster ------------------------ */
    AdwApplicationWindow *win = ADW_APPLICATION_WINDOW (adw_application_window_new (GTK_APPLICATION (app))); 

    gtk_window_set_title (GTK_WINDOW(win), "Finden");   // Fenstertitel
    gtk_window_set_default_size (GTK_WINDOW(win), 490, 260);  // Standard-Fenstergröße
    gtk_window_present (GTK_WINDOW(win));                     // Fenster anzeigen lassen

    /* ----- ToolbarView (Root‑Widget) erstellt und als Inhalt des Fensters festgelegt -- */
    AdwToolbarView *toolbar_view = ADW_TOOLBAR_VIEW (adw_toolbar_view_new ());
    adw_application_window_set_content (win, GTK_WIDGET (toolbar_view));

    /* ----- HeaderBar mit TitelWidget erstellt und dem ToolbarView hinzugefügt ------------ */
    AdwHeaderBar *header = ADW_HEADER_BAR (adw_header_bar_new());
    /* Label mit Pango‑Markup erzeugen */
    GtkLabel *title_label = GTK_LABEL(gtk_label_new (NULL));
    gtk_label_set_markup (title_label, "<b>Finden</b>");                  // Fenstertitel in Markup
    gtk_label_set_use_markup (title_label, TRUE);                        //Markup‑Parsing aktivieren
    adw_header_bar_set_title_widget (header, GTK_WIDGET (title_label)); //Label als Title‑Widget einsetzen
    adw_toolbar_view_add_top_bar (toolbar_view, GTK_WIDGET (header));  //Header‑Bar zur Toolbar‑View hinzuf

    /* --- Hamburger‑Button innerhalb der Headerbar --- */
    GtkMenuButton *menu_btn = GTK_MENU_BUTTON (gtk_menu_button_new ());
    gtk_menu_button_set_icon_name (menu_btn, "open-menu-symbolic");
    adw_header_bar_pack_start (header, GTK_WIDGET (menu_btn));

    /* --- Popover‑Menu im Hamburger --- */
    GMenu *menu = g_menu_new ();
    g_menu_append (menu, _("Über Finden"), "app.show-about");
    GtkPopoverMenu *popover = GTK_POPOVER_MENU (
        gtk_popover_menu_new_from_model (G_MENU_MODEL (menu)));
    gtk_menu_button_set_popover (menu_btn, GTK_WIDGET (popover));

    /* --- Action die den About‑Dialog öffnet --- */
    const GActionEntry entries[] = {
        { "show-about", show_about, NULL, NULL, NULL }
    };
    g_action_map_add_action_entries (G_ACTION_MAP (app), entries, G_N_ELEMENTS (entries), app);


    /* ---- Haupt‑Box erstellen ----------------------------------------------------------- */
    GtkBox *box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 12));
    gtk_widget_set_margin_top    (GTK_WIDGET (box), 20);
    gtk_widget_set_margin_bottom (GTK_WIDGET (box), 20);
    gtk_widget_set_margin_start  (GTK_WIDGET (box), 20);
    gtk_widget_set_margin_end    (GTK_WIDGET (box), 20);
    gtk_widget_set_hexpand (GTK_WIDGET (box), TRUE);
    gtk_widget_set_vexpand (GTK_WIDGET (box), TRUE);

    /* ----- Text-Label erstellen  ----- */
    GtkWidget *label = gtk_label_new(_("Finden statt Suchen 😉"));
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);

    /* ----- Label als Inhalt zur hinzufügen ----- */ 
    gtk_box_append (box, label);

    /* ----- Suchleiste + Root‑Checkbutton oberhalb der Schaltfläche (horizontal) ----- */
    static GtkWidget *search_entry = NULL;   /* Suchleiste */
    static GtkWidget *search_box   = NULL;   /* Gtk-Box */

    if (!search_entry) {
    /* Suchfeld */
        search_entry = gtk_entry_new ();
        gtk_entry_set_placeholder_text (GTK_ENTRY (search_entry),
                                        _("Suchbegriff eingeben …"));
        gtk_widget_set_hexpand (search_entry, TRUE);
        gtk_widget_set_halign (search_entry, GTK_ALIGN_FILL);
        gtk_widget_set_size_request (search_entry, 300, -1);

        g_signal_connect (search_entry, "activate",
                          G_CALLBACK (on_search_button_clicked),
                          search_entry);
    }

     /* Kontrollkästchen/Checkbox mit Namen "root" */
    if (!root_check) {
        root_check = GTK_CHECK_BUTTON (gtk_check_button_new_with_label ("Root"));
        gtk_check_button_set_active (root_check, FALSE);
    }

    /* Horizontales Box‑Widget nur einmal erzeugen */
    if (!search_box) {
        search_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_widget_set_hexpand (search_box, TRUE);
        /* Beide Widgets nur hier einfügen – danach besitzen sie einen Eltern‑Container */
        gtk_box_append (GTK_BOX (search_box), search_entry);
        gtk_box_append (GTK_BOX (search_box), GTK_WIDGET (root_check));
    }

    /* Das bereits vorbereitete horizontale Box‑Widget in die vertikale Haupt‑Box einfügen */
    gtk_box_append (box, search_box);

    /* Flatpak-App-Version hat kein Zugriff auf Root, Checkbox deaktivieren */
    if (getenv("FLATPAK_ID")) {
     gtk_widget_set_sensitive(GTK_WIDGET(root_check), FALSE);
    }

    /* --- Schaltfläche-Finden:  ------------------------------------------------- */
    GtkWidget *new_button = gtk_button_new_with_label (_("  Finden  "));
    gtk_widget_set_halign (new_button, GTK_ALIGN_CENTER);
    g_signal_connect (new_button, "clicked",
                      G_CALLBACK (on_search_button_clicked),
                      search_entry);          /* user_data = entry */
    gtk_box_append (box, new_button);

    /* ----- Schaltfläche Beenden erzeugen ----- */
    GtkWidget *quit_btn = gtk_button_new_with_label(_("Beenden"));
    gtk_widget_set_halign(quit_btn, GTK_ALIGN_CENTER);

    /* ----- Callback auslösen ----- */
    g_signal_connect(quit_btn, "clicked", G_CALLBACK(on_quitbtn_clicked), app);

    /* ----- Schaltfläche der Box hinzufügen ----- */
    gtk_box_append(box, quit_btn);

    /* -----  Box zur ToolbarView hinzufügen ------------ */
    adw_toolbar_view_set_content(toolbar_view, GTK_WIDGET(box));

    /* ----- Fenster desktop‑konform anzeigen ----- */
    gtk_window_present(GTK_WINDOW(win));
    gtk_widget_grab_focus(search_entry); //fokus auf Suchleiste

}

/* ---------------------------------------------------------------------------
 * Anwendungshauptteil, main()
 * --------------------------------------------------------------------------- */
int main (int argc, char **argv)
{
    char *app_dir = g_get_current_dir();  // Ermit. den aktuellen Arbeitsverzeichnis-Pfad
    const char *locale_path = NULL;
    const char *flatpak_id = getenv("FLATPAK_ID"); //flatpak string free.toq.finden anderenfalls NULL !

    /* Resource‑Bundle (finden.gresource) registrieren um den Inhalt verfügbar zu machen */
    //g_resources_register (resources_get_resource ());

    /* ----- Erstelle den Pfad zu den locale-Dateien ----------------------------------- */
    setlocale(LC_ALL, "");
    textdomain("toq-finden");
    bind_textdomain_codeset("toq-finden", "UTF-8"); // Basisverzeichnis für Übersetzungen
    if (flatpak_id != NULL && flatpak_id[0] != '\0')  // Wenn ungleich NULL:
    {
        locale_path = "/app/share/locale"; // Flatpakumgebung /app/share/locale
    } else {
        locale_path = "/usr/share/locale"; // Native Hostumgebung /usr/share/locale
    }
    bindtextdomain("toq-finden", locale_path);
    g_print (_("Lokalisierung in: %s \n"), locale_path); // testen

    g_autoptr (AdwApplication) app =                        // Instanz erstellen + App-ID + Default-Flags;
        adw_application_new ("free.toq.finden", G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL); // Signal mit on_activate verbinden
    /* --- g_application_run startet Anwendung u. wartet auf Ereignis --- */
    return g_application_run (G_APPLICATION (app), argc, argv);
}