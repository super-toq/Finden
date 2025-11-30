/* FINDEN is part of my learning project;
 * toq 2025  LICENSE: BSD 2-Clause "Simplified"
 *
 *
 *
 * gcc $(pkg-config --cflags gtk4 libadwaita-1) -o finden main.c free.toq.finden.gresource.c $(pkg-config --libs gtk4 libadwaita-1)
 *
 * Please note:
 * The Use of this code and execution of the applications is at your own risk, I accept no liability!
 *
 * Version 0.8.5
 */
#include <glib.h>
#include <gtk/gtk.h>
#include <adwaita.h>
#include "icon-gresource.h"
#include <locale.h> 
#include <glib/gi18n.h>


/* --- globale Referenzen --- */
/* Erzeugen eines neuen Strukturtyps mit Namen UiRefs */
typedef struct {     // Widget-Zeigern erstellten... 
    GtkEditable     *search_entry; // zeigt auf GtkEditable ...
    GtkCheckButton  *root_check;
    GtkCheckButton  *snapshots_check;
} UiRefs;

static char   *app_dir    = NULL;     // Ermit. den aktuellen Arbeitsverzeichnis-Pfad
static gchar *glob_term_path = NULL;  // terminal Pfad global ermittelt
static const gchar *glob_term_name = NULL; // term. Name ...
static const char *flatpak_id  = NULL;  
static gboolean    is_flatpak = FALSE; // 1 oder 0 ausgeben



/* ---------- Initialisierungsfunktion ---------- */
static void init_environment(void)
{
flatpak_id = getenv("FLATPAK_ID"); //siehe G.Ref.
is_flatpak = (flatpak_id != NULL && flatpak_id[0] != '\0');
//is_flatpak = 1; // zum Testen des Verhaltens einer Flatpak App

    /* Pfad zum eigenen Executable Verzeichnis */
    gchar *exe_path = g_file_read_link("/proc/self/exe", NULL);
    if (!exe_path) {
        g_warning(_("Der exe_path kann nicht erstellt werden.\n"));
        app_dir = NULL;
        return;
    }
    /* Eigenen Pfad ermitteln: */
    app_dir = g_path_get_dirname(exe_path); // =Global
    g_print (_("Anwendungspfad: %s \n"), app_dir);
    g_free(exe_path);
}

/* ---------- miniterm suchen ------------------- */
static void find_miniterm(void)
{
 /* Abbruch, falls init_environment fehlschlägt */
if (!app_dir) {g_warning (_("[G] Abbruch, Variable 'app_dir' wurde nicht gesetzt!\n")); return;}   

 /* 1. Pfad zu miniterm im selben Verzeichnis suchen */
    gchar *miniterm_path = g_build_filename(app_dir, "free.toq.miniterm", NULL);
    g_print (_("miniterm in Umgebung gefunden: %s \n"), miniterm_path); // testen
    g_free(app_dir);


    /* 2. Prüfen, ob miniterm existiert und ausführbar ist */
    if (g_file_test(miniterm_path, G_FILE_TEST_IS_EXECUTABLE)) {
        glob_term_path = g_strdup(miniterm_path);
        glob_term_name = "free.toq.miniterm";
        g_print(_(" %s gefunden in %s\n"), glob_term_name, glob_term_path);
        g_free(miniterm_path);
        return;                     // miniterm gefunden
    }

    g_free(miniterm_path); 
}
/* ----- Message / Alert-Dialog Generisch,  show_alert_dialog (parent,*Titel, *Inhalttext) ----- */
static void on_alert_dialog_response (AdwAlertDialog *dialog,
                          const char     *response,
                          gpointer        user_data)
{
    if (g_strcmp0 (response, "ok") == 0)
        g_print ("Dialog btn - ok\n");
    else
        g_print ("Dialog btn - cancel\n");
}

/* ----- Callback Alert-Dialog anzeigen (generisch) ----- */
static void
show_alert_dialog (GtkWindow   *parent,
                   const char  *title,
                   const char  *body)
{
    if (!parent || !GTK_IS_WINDOW (parent)) {
        g_warning (_("Kein gültiges Elternfenster für Alert-Dialog.\n"));
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
    adw_about_dialog_set_version (about, "0.8.6");
    adw_about_dialog_set_developer_name (about, "toq");
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
        "Thanks to SVG for sharing their free icons, we appreciate your generosity and respect your work.\n"
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
}//Ende About-Dialog

/* ---- "find"-Kommando zusammenbauen - 
     Außerhalb von on_search_button_clicked, da sonst "nested-functions" innerhalb einer Funktion ensteht! */
    
gchar *find_cmd = NULL;

    /* 5.1  find Kommando:   ([A] ohne Root, nur im Homeverzeichnis)  */
static void action_A(const char *find_path, const char *query) { // find_path u. query als Argumente bekommen.
    find_cmd = g_strdup_printf("%s %s -iname \"*%s*\"", 
       find_path, g_get_home_dir(), query); 
                 /* g_get_home_dir() ist Funktion aus GLib, welche direkt
                    hier innerhalb dieser Funktion eine Zeichenkette in "const gchar" ermittelt. */
}

    /* 5.2  find Kommando:   ([B] Root ohne Sanpashots)  */
static void action_B(const char *find_path, const char *query) { // find_path u. query als Argumente bekommen.
    find_cmd = g_strdup_printf(
        "run0 --background=0 --unit=finden --via-shell %s / -path \"/.snapshots\" -prune -o -iname \"*%s*\"",
        find_path, query); 
}
        
    /* 5.3  find Kommando:   ([C] Root + Snapshots)  */
static void action_C(const char *find_path, const char *query) { // find_path u. query als Argumente bekommen.
    find_cmd = g_strdup_printf(
        "run0 --background=0 --unit=finden --via-shell %s / -iname \"*%s*\"", 
        find_path, query); 
}


/* ----- Callback Beenden-Button ----- */
static void on_quitbtn_clicked(GtkButton *button, gpointer user_data) 
    { 
    GtkWindow *win = GTK_WINDOW(user_data);
    gtk_window_destroy(win);

    UiRefs *refs = g_object_get_data (G_OBJECT (button), "ui_refs");
    if (refs) g_free (refs);
    }

/* ----- Callback für beide Kontrollkästchen (Toggle) ----------------------------------- */
static void on_check_button_toggled (GtkCheckButton *toggle_check, gpointer user_data) 
{
    UiRefs *refs = user_data;               /* jetzt wirklich ein UiRefs‑Zeiger */

    /* Fokus zurück auf das Suchfeld */
    gtk_widget_grab_focus (GTK_WIDGET (refs->search_entry));

    /* Checkboxen-Status herausfinden */
    if (toggle_check == refs->root_check) 
    {
        /* Wenn Root deaktiviert muss Snapshots ebenfalls deaktiviert werden */
        if (!gtk_check_button_get_active (refs->root_check))
            gtk_check_button_set_active (refs->snapshots_check, FALSE);
        return;
    }


    if (toggle_check == refs->snapshots_check)
    {
        /* Optional: wenn Snapshots aktiviert wird, Root aktivieren */
        if (gtk_check_button_get_active (refs->snapshots_check))
            gtk_check_button_set_active (refs->root_check, TRUE);
        return;
    }

}

/* ----- Callback Suchfunktion ausführen | Hauptfunktion --------------------------------- */
static void on_search_button_clicked (GtkButton *button, gpointer user_data)
{
    
    UiRefs *refs = (UiRefs *)user_data; // Behandle user_data als UiRefs*.
    if (!refs) return; // Sicherheitscheck, kein gültiger Zeiger, Abbruch vor Crash.


    const gchar *query = gtk_editable_get_text(GTK_EDITABLE(refs->search_entry));
    if (!query || *query == '\0') {
        g_print(_("Bitte einen Suchbegriff eingeben.\n"));
        return;
    }

    /* 0. ---- Struktur dient für "Schalter" ---- */

    /* 1. ---- Tool-"find"‑ermitteln ---------------------------------------------------- */
    const gchar *find_prog = "find";
    gchar *find_path = g_find_program_in_path (find_prog);
    if (!find_path) {
        const gchar *fallback = "/usr/bin/find";
        if (g_file_test (fallback, G_FILE_TEST_IS_EXECUTABLE))
            find_path = g_strdup (fallback);
    }
    if (!find_path) {
        g_warning (_("Abbruch, Tool %s wurde nicht gefunden!\n"), find_prog);
        // Alert-Dialog noch einfügen !!!
        return;
    }

    /* 2. ---- Prüfen, ob die Checkboxen aktiviert ---- */
        gboolean root_active = FALSE;
        gboolean snapshots_active = FALSE;
  
        if (GTK_IS_CHECK_BUTTON(refs->root_check))
            root_active = gtk_check_button_get_active(refs->root_check);
        if (GTK_IS_CHECK_BUTTON(refs->snapshots_check))
            snapshots_active = gtk_check_button_get_active(refs->snapshots_check);


    /* 2.1 ---- Debug-Ausgaben ------------------------ */
    g_print(_("Root-Schalter %s\n"),
        root_active ? _("aktiviert") : _("nicht aktiviert"));

    g_print(_("Snapshots-Schalter %s\n"),
        snapshots_active ? _("aktiviert") : _("nicht aktiviert"));

    /* 3. ---- Modus bestimmen ----------------------  */
    typedef enum 
    {
        ROOT_OFF = 0,
        FLATPAK_DISABLE,
        ROOT_NO_RUN0,
        ROOT_RUN0_OK,
        ROOT_RUN0_SNAPSHOTS_OK
    } 
    RootMode;
    RootMode mode = ROOT_OFF;

    if (!root_active) {
        /* 3.1  ROOT= 0 */
        mode = ROOT_OFF;
    }
    else if (is_flatpak) {
        /* 3.2  App= Flatpak, alles deaktivieren  */
        mode = FLATPAK_DISABLE;
    }
    else if (!g_file_test("/usr/bin/systemd-run", G_FILE_TEST_EXISTS)) {
        /* 3.3  ROOT=1, RUN0=0 */
        mode = ROOT_NO_RUN0;
    }
    else if (!snapshots_active) {
        /* 3.4  ROOT=1, RUN0=1, SNAPSHOTS=0 */
        mode = ROOT_RUN0_OK;
    }
    else {
        /* 3.5  ROOT=1, RUN0=1, SNAPSHOTS=1 */
        mode = ROOT_RUN0_SNAPSHOTS_OK;
    }

    /* ---- 4. Schalter aus 3.x - für ROOT, RUN0, SNAPSHOTS + Find-Kommande ---- */
    void (*cmd_action)(const char *, const char *) = NULL;
    cmd_action = action_A; // Find-Kommando [A] wird in "static void action_A" definiert
    // Diese Maßnahme war notwendig, da sonst nested-Fuction innerhalb einer Funktion entsteht!

    switch (mode)
    {
        case ROOT_OFF:  // ROOT=0
            g_print(_("Suchfunktion wird nur im Homeverzeichnis durchgeführt.\n")); // testen
            /* find Kommando [A] verwenden  */
            cmd_action = action_A; // action_A wird in "static void action_A" definiert
            break;

        case FLATPAK_DISABLE: // FLATPAK=1 - Optionen deaktivieren
        {    
            g_print(_("Anwendung ist flatpak Version, Optionen sind deaktiviert.\n"));
            
            /* Checkboxen zurücksetzen und deaktivieren */
            gtk_check_button_set_active(refs->root_check, FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(refs->root_check), FALSE);
            gtk_check_button_set_active(refs->snapshots_check, FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(refs->snapshots_check), FALSE);
            /* find Kommando [A] verwenden  */
            cmd_action = action_A; // action_A wird in "static void action_A" definiert
            break;
        }

        case ROOT_NO_RUN0: // ROOT=1, RUN0=0, somit auch keine Snapshots
        {
            g_print(_("Service run0 ist nicht aktiv.\n"));

            /* Checkbox zurücksetzen */
            gtk_check_button_set_active(refs->root_check, FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(refs->root_check), FALSE);

            /* ALert-Dialog aufrufen */
            GtkWindow *parent = GTK_WINDOW (gtk_widget_get_root(GTK_WIDGET(refs->search_entry)));
            if (parent)
            {
                show_alert_dialog(parent, _("Run0 nicht aktiviert"),
                                          _("Der Service Run0 ist nicht aktiv,\n"
                                            "bitte überprüfen Sie Ihre Systemkonfiguration.\n"));
            }
            /* root_active manuell zurücksetzen */
            root_active = FALSE;
            /* find Kommando [A] verwenden  */
            cmd_action = action_A; // action_A wird in "static void action_A" definiert
            break;
        }

       case ROOT_RUN0_OK: // ROOT=1, RUN0=1, SNAPSHOTS=0
            g_print(_("Service run0 vorhanden.\n"));
            /* find Kommando [B] verwenden  */
            cmd_action = action_B; // action_B wird in "static void action_B" definiert
            break;

       case ROOT_RUN0_SNAPSHOTS_OK: // ROOT=1, RUN0=1, SNAPSHOTS=1
            g_print(_("Service run0 vorhanden.\nPfad für .snapshots aktiv.\n"));
            /* find Kommando [C] verwenden  */
            cmd_action = action_C; // action_C wird in "static void action_C" definiert
            break;
    } // Switch(mode) Ende

    /* 5. ---- Aktion für das find-Kommando aufrufen ---- */
    if (cmd_action) cmd_action(find_path, query);  

 /* Hinweis - cmd_action beinhaltet:
    action_A:
    "/usr/bin/find ~/ -iname \"*<query>*\""
  
    action_B:
    "run0 --background=0 --unit=finden --via-shell /usr/bin/find / -path \"/.snapshots\" -prune -o -iname \"*<query>*\""
  
    action_C:
    "run0 --background=0 --unit=finden --via-shell /usr/bin/find / -iname \"*<query>*\""
 */
    g_free(find_path);

    /* 6. ---- Terminal im System ermitteln ---- */

    /* Erstelle Pfad zu miniterm */
    gchar *term_path = NULL;
    const gchar *term_name = NULL;

    if (glob_term_name && g_str_has_prefix(glob_term_name, "free.toq.miniterm")) {
        g_print (_("[L1] %s gefunden in %s\n"), glob_term_name, glob_term_path);
        term_path = glob_term_path;
    } else {
        static const gchar *terminals[] = {
            "free.toq.miniterm",
            "konsole",
            "gnome-terminal2",
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
                g_print (_("[L2] %s gefunden in %s\n"), term_name, term_path);
                break;
            }
        }
    }
//    g_free (mini_path);

    if (!term_path) {
        g_warning (_("Kein unterstütztes Terminal gefunden!\n"));
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
    /* term_path ist Ergebnis von g_find_program_in_path(), welches ein Zeiger auf glob_term_path hat!
       Lösung zum Eigentum erlangen und anschließend leeren: */
    term_path = g_strdup(term_path);
    g_free (term_path); // < fehler
    

    /* find_cmd und full_cmd einmalig freigeben (full_cmd wurde bereits durch argv-loop freigegeben!)*/
    g_free (find_cmd);  /* full_cmd) bereits via argv loop */

}

/* -------------------------------------------------------------*/
/*       Aktivierungshandler                                    */
/* -------------------------------------------------------------*/
static void on_activate (AdwApplication *app, gpointer)
{
    /* ----- Adwaita-Fenster ------------------------ */
    AdwApplicationWindow *adw_win = ADW_APPLICATION_WINDOW (adw_application_window_new (GTK_APPLICATION (app))); 

    gtk_window_set_title (GTK_WINDOW(adw_win), "Finden");         // WM-Titel
    gtk_window_set_default_size (GTK_WINDOW(adw_win), 480, 280);  // Standard-Fenstergröße
    gtk_window_set_resizable (GTK_WINDOW (adw_win), FALSE);       // Skalierung nicht erlauben
    gtk_window_present (GTK_WINDOW(adw_win));                     // Fenster anzeigen lassen

    /* ----- ToolbarView (Root‑Widget) erstellt und als Inhalt des Fensters festgelegt -- */
    AdwToolbarView *toolbar_view = ADW_TOOLBAR_VIEW (adw_toolbar_view_new ());
    adw_application_window_set_content (adw_win, GTK_WIDGET (toolbar_view));

    /* ----- HeaderBar mit TitelWidget erstellt und dem ToolbarView hinzugefügt ------------ */
    AdwHeaderBar *header = ADW_HEADER_BAR (adw_header_bar_new());
    GtkWidget * title_label = gtk_label_new ("Finden");                 // Label für Fenstertitel
    gtk_widget_add_css_class (title_label, "heading");                  // .heading class
    adw_header_bar_set_title_widget (ADW_HEADER_BAR (header), GTK_WIDGET (title_label)); // Label einsetzen
    adw_toolbar_view_add_top_bar (toolbar_view, GTK_WIDGET (header));   // Header‑Bar zur Toolbar‑View hinzuf

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
    GtkBox *mainbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 12));
    gtk_widget_set_margin_top    (GTK_WIDGET (mainbox), 20);
    gtk_widget_set_margin_bottom (GTK_WIDGET (mainbox), 20);
    gtk_widget_set_margin_start  (GTK_WIDGET (mainbox), 20);
    gtk_widget_set_margin_end    (GTK_WIDGET (mainbox), 20);
    gtk_widget_set_hexpand (GTK_WIDGET (mainbox), TRUE);
    gtk_widget_set_vexpand (GTK_WIDGET (mainbox), TRUE);

    /* ----- Text-Label erstellen  ----- */
    GtkLabel *label1 = GTK_LABEL(gtk_label_new (NULL));
    gtk_label_set_markup (label1, _("<b>Finden statt Suchen</b>")); // Titel in Markup
    gtk_label_set_use_markup (label1, TRUE);                        // Markup-Parsing aktivieren
    gtk_widget_set_halign (GTK_WIDGET (label1), GTK_ALIGN_CENTER);
    gtk_widget_set_valign (GTK_WIDGET (label1), GTK_ALIGN_CENTER);

    /* ----- Smiley‑Image aus der Resource ----- */
    GtkWidget *smiley = gtk_image_new_from_resource ("/free/toq/finden/smiley1");
    gtk_image_set_pixel_size (GTK_IMAGE (smiley), 32);   // Smiley Größe

    /* ----- BOX-Widget für Text und Smiley erstellen ----- */
    GtkBox *smileytext_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6));
    gtk_widget_set_halign (GTK_WIDGET (smileytext_box), GTK_ALIGN_CENTER);
    gtk_widget_set_valign (GTK_WIDGET (smileytext_box), GTK_ALIGN_CENTER);

    /* ----- Smiley und TextLabel der Box hinzufügen ----- */ 
    gtk_box_append (smileytext_box, GTK_WIDGET (label1)); // Label in Widget convertieren
    gtk_box_append (smileytext_box, smiley); // Reihenfolge von Label und Smiley

    gtk_box_append (GTK_BOX (mainbox), GTK_WIDGET (smileytext_box));

    /* ----- Suchleiste + Root‑Checkbutton oberhalb der Schaltfläche (horizontal) ----- */
    GtkWidget *search_entry = NULL;   // Suchleisten-Widget
    GtkWidget *search_box   = NULL;   // Box-Widget in der sich die Suchleiste befindet
    GtkWidget *checkb_box = NULL;     // Box-Widget für Checkboxen

    if (!search_entry) {
    /* Suchfeld */
        search_entry = gtk_search_entry_new ();
        gtk_search_entry_set_placeholder_text (GTK_SEARCH_ENTRY (search_entry),
                                       _("Suchbegriff eingeben …"));
        gtk_widget_set_hexpand (search_entry, TRUE);
        gtk_widget_set_halign (search_entry, GTK_ALIGN_FILL);
        gtk_widget_set_size_request (search_entry, 300, -1);

    }

    /* --- Horizontales Box-Widget für Suchleiste hier erzeugen ---------------------------- */
    if (!search_box) {
        search_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 16);
        gtk_widget_set_hexpand (search_box, TRUE);
        /* Widgets nur hier einfügen – danach besitzen sie einen Eltern‑Container */
        gtk_box_append (GTK_BOX (search_box), search_entry);
    }

    /* Das bereits vorbereitete horizontale Box‑Widget in die vertikale Haupt‑Box einfügen */
    gtk_box_append (mainbox, search_box);

    /* --- Horizontales Box-Widget für Checkboxen hier erzeugen ------------------------- */
    if (!checkb_box) { 
        checkb_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6); 
        gtk_widget_set_hexpand(checkb_box, TRUE);

        /* Zentrierung der Widgetbox, wirkt sich auf die darin bef. Kontrollkästchen aus */
        gtk_widget_set_hexpand (checkb_box, FALSE);            //  nicht autom. Ausdehnen
        gtk_widget_set_halign (checkb_box, GTK_ALIGN_CENTER);  // zentrieren
        gtk_widget_set_valign (checkb_box, GTK_ALIGN_CENTER);
    }

    /* --- (1.)Kontrallkästchen/Checkbox mit Namen "Ignoriere Snapshots" erstellen --- */
    GtkWidget *snapshots_check = gtk_check_button_new_with_label(_("Snapshots durchsuchen"));
    gtk_widget_add_css_class (snapshots_check, "selection-mode");
    gtk_check_button_set_active (GTK_CHECK_BUTTON(snapshots_check), FALSE);

     /* --- (2.)Kontrollkästchen/Checkbox mit Namen "root" erstellen --- */
    GtkWidget *root_check = gtk_check_button_new_with_label(_("Root durchsuchen"));
    gtk_widget_add_css_class (root_check, "selection-mode");
    gtk_check_button_set_active (GTK_CHECK_BUTTON(root_check), FALSE);

    /* --- Beide Checkboxen in horizontale Box einfügen (hier gilt die Reihenfolge) --- */
    gtk_box_append(GTK_BOX(checkb_box), GTK_WIDGET(snapshots_check));  
    gtk_box_append(GTK_BOX(checkb_box), GTK_WIDGET(root_check));

    /* Die Widget-Box"checkb_box" der Eltern-WidgetBox"box" hinzugügen */
    gtk_box_append(GTK_BOX(mainbox), checkb_box);

        /* Flatpak-App-Version hat kein Zugriff auf Root, Checkboxen deaktivieren */
        if (is_flatpak) 
        {
           gtk_widget_set_sensitive(GTK_WIDGET(root_check), FALSE);
           gtk_widget_set_sensitive(GTK_WIDGET(snapshots_check), FALSE);
        }

    /* --- Kontext-Struct-Block für die Initialisierung einer UI-Referenzstruktur --- */
    /* "refs" zeigt auf neu erstellten “Behälter” für UI-Elemente */
    UiRefs *refs = g_new0(UiRefs, 1); // Speicherort anlegen, erzeuge 1 UiRefs im Heap.
    refs->search_entry    = GTK_EDITABLE(search_entry); // Pointer zum Eingabefeld im Struct.
    refs->root_check      = GTK_CHECK_BUTTON(root_check); // Pointer zur Root-Checkbox.
    refs->snapshots_check = GTK_CHECK_BUTTON(snapshots_check); // Pointer zur Snapshots-Checkbox.


    /* --- Schaltflächen-WidgetBox hier anlegen: ------------------------------ */
    GtkWidget *button_hbox = NULL;
    if (!button_hbox) {
        button_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_set_hexpand(button_hbox, FALSE);                // nicht ausdehnen
        gtk_widget_set_halign(button_hbox, GTK_ALIGN_CENTER);      // zentriert
    }

    /* --- Schaltfläche-Finden:  ------------------------------------------------- */
    GtkWidget *search_button = gtk_button_new_with_label (_("  Finden  "));

     /* ----- Schaltfläche in Theme-Akzent-Farbe und Pill Class ------------------ */
    gtk_widget_add_css_class (search_button, "suggested-action");
    //gtk_widget_add_css_class (search_button, "pill");

    /* --- Schaltfläche verbinden --- */
    g_signal_connect(search_button, "clicked",
                      G_CALLBACK(on_search_button_clicked), refs); //user_data = refs

    /* --- Suchleiste verbinden für ENTER Taste --- */
    g_signal_connect(search_entry,  "activate", G_CALLBACK(on_search_button_clicked), refs);

    /* ----- Schaltfläche Beenden erzeugen ----- */
    GtkWidget *quit_button = gtk_button_new_with_label(_("Beenden"));
    //gtk_widget_add_css_class (quit_button, "pill");
    //gtk_widget_set_halign(quit_button, GTK_ALIGN_CENTER);
    
    /* ---- Schaltfläche Signal verbinden ---- */
           // Methode um Anwendung mit jeglichen Instanzen zu schließen:
//            g_signal_connect(quit_button, "clicked", G_CALLBACK(on_quitbtn_clicked), app);
      // Nethode um das Fenster zu schließen, egal wieviele Instanzen existieren:
         g_signal_connect(quit_button, "clicked", G_CALLBACK(on_quitbtn_clicked), adw_win);

    /* ---- Kontrollkästchen Signal verbinden ---- */
    // Toggel für beide Checkboxen, sowie um den Fokus für die Suchleiste wieder neu zu setzen!
    g_signal_connect(snapshots_check, "toggled", G_CALLBACK(on_check_button_toggled), refs);
    g_signal_connect(root_check, "toggled", G_CALLBACK(on_check_button_toggled), refs);

    /* ----- Schaltfläche der Box hinzufügen ----- */
    gtk_box_append(GTK_BOX(button_hbox), quit_button);    
    gtk_box_append(GTK_BOX(button_hbox), search_button);

    /* --- Checkboxen am Search-Button speichern, damit im Callback diese auch abrufen werden --- */
    g_object_set_data(G_OBJECT(search_button), "root_check", root_check);
    g_object_set_data(G_OBJECT(search_button), "snapshots_check", snapshots_check);

    /* -----  Box zur ToolbarView hinzufügen ------------ */
    gtk_box_append(GTK_BOX(mainbox), button_hbox);
    adw_toolbar_view_set_content(toolbar_view, GTK_WIDGET(mainbox));

    /* ----- Fenster desktop‑konform anzeigen ----- */
    gtk_window_present(GTK_WINDOW(adw_win));
    gtk_widget_grab_focus(search_entry); //fokus auf Suchleiste

}

/* ---------------------------------------------------------------------------
 * Anwendungshauptteil, main()
 * --------------------------------------------------------------------------- */
int main (int argc, char **argv)
{

    const char *locale_path = NULL;

    /* Resource‑Bundle (finden.gresource) registrieren um den Inhalt verfügbar zu machen */
    g_resources_register (resources_get_resource ()); // reicht für Icon innerhalb der App

    /* ----- Erstelle den Pfad zu den locale-Dateien ----------------------------------- */
    if (is_flatpak)     // App ist FlatpakApp ? (global)
    {
        locale_path = "/app/share/locale"; // Flatpakumgebung /app/share/locale
    } else {
        locale_path = "/usr/share/locale"; // Native Hostumgebung /usr/share/locale
    }
    setlocale(LC_ALL, "");
    textdomain("toq-finden");
    bind_textdomain_codeset("toq-finden", "UTF-8"); // Basisverzeichnis für Übersetzungen
    bindtextdomain("toq-finden", locale_path);
    g_print (_("Lokalisierung in: %s \n"), locale_path); // testen


    init_environment(); // Globale Umgebungsvariablen
    find_miniterm ();   // miniterm Pfad suchen

    /* GTK/Adwaita Anwendung: */
    g_autoptr (AdwApplication) app =                        // Instanz erstellen + App-ID + Default-Flags;
        adw_application_new ("free.toq.finden", G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL); // Signal zu on_activate

    /* Event_Loop */
    int ret = g_application_run (G_APPLICATION (app), argc, argv);

    g_free (app_dir);          // aus global init_environment()
    g_free (glob_term_path);   // aus global find_miniterm()
    return ret;
}
