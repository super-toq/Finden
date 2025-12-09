/* FINDEN is part of my learning project;
 * toq 2025  LICENSE: BSD 2-Clause "Simplified"
 *
 *
 *
 * gcc $(pkg-config --cflags gtk4 libadwaita-1) -o finden main.c config.c free.toq.finden.gresource.c $(pkg-config --libs gtk4 libadwaita-1)
 *
 * Please note:
 * The Use of this code and execution of the applications is at your own risk, I accept no liability!
 *
 * Version 0.9.6_2
 */
#include <glib.h>
#include <gtk/gtk.h>
#include <adwaita.h>
#include <locale.h> 
#include <glib/gi18n.h>

#include "icon-gresource.h"
#include "config.h"


/* --- globale Referenzen --- */

/* Erzeugen eines neuen Strukturtyps mit Namen UiRefs */
typedef struct {     // Widget-Zeigern erstellten... 
    GtkEditable     *search_entry; // zeigt auf GtkEditable ...
    GtkCheckButton  *root_check;
    GtkCheckButton  *snapshots_check;
    GtkCheckButton  *exact_check;
} UiRefs;

static gchar       *glob_term_path = NULL;  // terminal Pfad global ermittelt
static const gchar *glob_term_name = NULL;  // term. Name ...
static gchar       *glob_path_miniterm = NULL;  // miniterm Pfad
static const gchar *glob_mini     = NULL;   // miniterm local
static const char  *flatpak_id     = NULL;  // ID...
static gboolean    is_flatpak     = FALSE;  // 1 oder 0 ausgeben


/* Hinweis, aus config.c: 
  g_cfg.miniterm_enable
  g_cfg.test_enable
*/

/* ---------- Initialisierungsfunktion ----------------------- */
static void init_flatpak(void)
{
flatpak_id = getenv("FLATPAK_ID"); //siehe G.Ref.
is_flatpak = (flatpak_id != NULL && flatpak_id[0] != '\0');
//is_flatpak = 1; // zum Testen des Verhaltens einer Flatpak App
}


/* ---------- Terminals ermitteln ------------------------------------------------------- */
static void find_terminals(void)
{
    /* ----- 1. Miniterm-Pfad ermitteln ----------------------------- */
    const gchar *app_dir = get_app_dir();   // app_dir aus config.c holen
    /* --- 1.1 Abbruch, falls init_environment fehlschlägt */
    if (!app_dir) {g_warning ("[t] Abort: variable app_dir was not set!\n"); 
    return; } // gesamte Funktion beenden bei fehlenden app_dir

    /* --- 1.2 Pfad zu miniterm im selben Verzeichnis suchen */
    gchar *miniterm_path = g_build_filename(app_dir, "toq-miniterm", NULL);

    /* --- 1.3 Prüfen, ob miniterm existiert und ausführbar ist */
    if (g_file_test(miniterm_path, G_FILE_TEST_IS_EXECUTABLE)) {
        glob_path_miniterm = g_strdup(miniterm_path);
        glob_mini          = "toq-miniterm";

        g_print("[t1] %s found in %s\n", glob_mini, glob_path_miniterm);
        g_free(miniterm_path);

    }  else {
         glob_path_miniterm = g_find_program_in_path ("toq-miniterm");
         glob_mini          = "toq-miniterm";
         g_print("[t1] %s found in %s\n", glob_mini, glob_path_miniterm);
    }
    /* ----- 2. System-Terminal ermitteln -------------------------- */
    if (!glob_term_name) {
            static const gchar *terminals[] = {
            "konsole",
            "gnome-terminal",
            "kgx",
            "terminator",
            "kitty",
            "xfce4-terminal",
            "tilix",
            "alacritty",
            "kitty",
            "foot",
            "wezterm",
            "xterm",
            NULL
        };
        for (int i = 0; terminals[i] != NULL; i++) {
            glob_term_path = g_find_program_in_path (terminals[i]);
            if (glob_term_path) {
                glob_term_name  = terminals[i];
                g_print ("[t2] %s found in %s\n", glob_term_name , glob_term_path);
                break;
            }
        }
    }
}

/* ----- Message / Alert-Dialog Generisch,  show_alert_dialog (parent,*Titel, *Inhalttext) ----- */
static void on_alert_dialog_response (AdwAlertDialog *dialog, const char *response, gpointer user_data)
{
    if (g_strcmp0 (response, "ok") == 0)
        g_print ("Dialog btn - ok\n");
    else
        g_print ("Dialog btn - cancel\n");
}

/* ----- Callback Alert-Dialog anzeigen (generisch) ------------------------------------- */
static void show_alert_dialog (GtkWindow *parent, const char *title, const char *body)
{
    if (!parent || !GTK_IS_WINDOW (parent)) {
        g_warning ("No valid parent window for alert dialog!\n");
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
    adw_about_dialog_set_application_name (about, "Finden");
    adw_about_dialog_set_version (about, "0.9.6");
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

/* ----- In Einstellungen: Miniterm-Schalter-Toggle ------------------------------------- */
static void on_settings_miniterm_switch_row_toggled (GObject *object, GParamSpec *pspec, gpointer user_data)
{
    AdwSwitchRow *miniterm_switch_row = ADW_SWITCH_ROW(object);
    gboolean active = adw_switch_row_get_active(miniterm_switch_row);
    g_cfg.miniterm_enable = active;
    save_config (); // speichern
    //g_print ("Settings changed: miniterm value %s\n", g_cfg.miniterm_enable ? "true" : "false"); // testen !!
}
/* ----- Einstellungen-Page ------------------------------------------------------------- */
static void show_settings (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    AdwNavigationView *settings_nav = ADW_NAVIGATION_VIEW(user_data);

    /* ----- ToolbarView für Settings-Seite ----- */
    AdwToolbarView *settings_toolbar = ADW_TOOLBAR_VIEW(adw_toolbar_view_new());

    /* ----- Headerbar erzeugen ----- */
    AdwHeaderBar *settings_header = ADW_HEADER_BAR(adw_header_bar_new());
    GtkWidget *settings_label = gtk_label_new(_("Einstellungen"));
    gtk_widget_add_css_class(settings_label, "heading");
    adw_header_bar_set_title_widget(settings_header, settings_label);

    /* ----- Headerbar einfügen ----- */
    adw_toolbar_view_add_top_bar(settings_toolbar, GTK_WIDGET(settings_header));

    /* ----- Haupt-BOX der Settings-Seite ----- */
    GtkWidget *settings_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(settings_box, 12);
    gtk_widget_set_margin_start(settings_box, 12);
    gtk_widget_set_margin_end(settings_box, 12);
    gtk_widget_set_margin_bottom(settings_box, 12);

    /* ----- PreferencesGroup erstellen ----- */
    AdwPreferencesGroup *settings_group = ADW_PREFERENCES_GROUP(adw_preferences_group_new());
    adw_preferences_group_set_title(settings_group, _("Terminaleinstellungen"));
    //adw_preferences_group_set_description (settings_group, _("Zusatzbeschreibung - Platzhalter"));

    /* ----- AdwSwitchRow1-Miniterm - erzeugen ----- */
    AdwSwitchRow *miniterm_switch_row = ADW_SWITCH_ROW(adw_switch_row_new());
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(miniterm_switch_row), 
                                       _("Miniterm als Terminal verwenden:"));
    adw_action_row_set_subtitle(ADW_ACTION_ROW (miniterm_switch_row),
     _("Wenn Sie Miniterm nicht benutzen wollen, wird automatisch das systemeigene Terminal verwendet."));
    /* Schalter-Aktivierung abhängig von gesetzten g_cfg.miniterm_enable Wert: */
    adw_switch_row_set_active(ADW_SWITCH_ROW(miniterm_switch_row), g_cfg.miniterm_enable);

    /* ----- AdwSwitchRow2-Testing - erzeugen ----- */
    AdwSwitchRow *testing_switch_row = ADW_SWITCH_ROW(adw_switch_row_new());
    adw_preferences_row_set_title (ADW_PREFERENCES_ROW(testing_switch_row), 
                                       _("Testeinstellungen verwenden:"));
    adw_action_row_set_subtitle(ADW_ACTION_ROW (testing_switch_row),
     _("Dies ist eine Vorbereitung für eine kommende Funktion."));
    /* Schalter-Aktivierung abhängig von gesetzten g_cfg.testing_enable Wert: */
    //adw_switch_row_set_active(ADW_SWITCH_ROW(miniterm_switch_row), g_cfg.miniterm_enable);
    // Vorerst deaktiv, später löschen:
    adw_switch_row_set_active(testing_switch_row, FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET (testing_switch_row), FALSE);

    /* ----- AdwSwitchRow1-Miniterm verbinden... ----- */
    g_signal_connect(miniterm_switch_row, "notify::active", 
                                          G_CALLBACK(on_settings_miniterm_switch_row_toggled), NULL);

    /* ----- AdwSwitchRow2-Testing verbinden... ----- */
    //g_signal_connect(testing_switch_row, "notify::active", 
    //                                      G_CALLBACK(on_settings_TESTING_switch_row_toggled), NULL);

    /* ----- SwitchRows zur PreferencesGruppe hinzufügen ----- */
    adw_preferences_group_add(settings_group, GTK_WIDGET(miniterm_switch_row));
    adw_preferences_group_add(settings_group, GTK_WIDGET(testing_switch_row));

    /* ----- Pref.Gruppe in die Page einbauen ----- */
    gtk_box_append(GTK_BOX(settings_box), GTK_WIDGET(settings_group));

    /* ----- ToolbarView Inhalt setzen ----- */
    adw_toolbar_view_set_content(settings_toolbar, settings_box);

    /* ----- NavigationPage anlegen ----- */
    AdwNavigationPage *settings_page = 
                      adw_navigation_page_new(GTK_WIDGET(settings_toolbar), _("Einstellungen"));
    gtk_widget_set_size_request(GTK_WIDGET(settings_page), 510, 250);

    /* ----- Page der Settings_nav hinzufügen ----- */
    adw_navigation_view_push(settings_nav, settings_page);
}// Ende Einstellungen-Fenster


/* ---- Kommando "find" zusammenbauen ---> 
 Außerhalb von on_search_button_clicked, da sonst "nested-functions" innerhalb einer Funktion ensteht! --- */
    
/* Global, aber nicht thread übergreifend */
gchar *find_cmd = NULL;
const gchar *iname_option = FALSE;
gchar *iname_query = NULL;

    /* 5.1  find Kommando:   ([A] ohne Root, nur im Homeverzeichnis)  */
static void action_A(const char *find_path, const char *query, UiRefs *refs)  
{ // find_path u. query als Argumente bekommen.

    gboolean exact_active = FALSE;

    if (GTK_IS_CHECK_BUTTON(refs->exact_check)) exact_active = gtk_check_button_get_active(refs->exact_check);
    if (exact_active) {
       iname_option = "-name";
       iname_query = g_strdup_printf ("%s \"%s\"", iname_option, query);
    } else {
       iname_option = "-iname";
       iname_query = g_strdup_printf ("%s \"*%s*\"", iname_option, query);
    }
    find_cmd = g_strdup_printf("%s %s %s", 
       find_path, g_get_home_dir(), iname_query); 
                 /* g_get_home_dir() ist Funktion aus GLib, 
                    welche innerhalb dieser Funktion eine Zeichenkette in "const gchar" ermittelt. */
    g_free (iname_query);
} // Hinweis: g_free(find_cmd) erfolgt in Suchfunktion;

    /* 5.2  find Kommando:   ([B] Root ohne Snapshots)  */
static void action_B(const char *find_path, const char *query, UiRefs *refs) 
{ // find_path u. query als Argumente bekommen.

    gboolean exact_active = FALSE;

    if (GTK_IS_CHECK_BUTTON(refs->exact_check)) exact_active = gtk_check_button_get_active(refs->exact_check);
    if (exact_active) {
       iname_option = "-name";
       iname_query = g_strdup_printf ("%s \"%s\"", iname_option, query);
    } else {
       iname_option = "-iname";
       iname_query = g_strdup_printf ("%s \"*%s*\"", iname_option, query);
    }

    find_cmd = g_strdup_printf(
    "run0 --background=0 --unit=finden --via-shell %s / -path \"/.snapshots\" -prune -o %s",
        find_path, iname_query);

    g_free (iname_query);
} // Hinweis: g_free(find_cmd) erfolgt in Suchfunktion;
        
    /* 5.3  find Kommando:   ([C] Root + Snapshots)  */
static void action_C(const char *find_path, const char *query, UiRefs *refs) 
{ // find_path u. query als Argumente bekommen.

    gboolean exact_active = FALSE;

    if (GTK_IS_CHECK_BUTTON(refs->exact_check)) exact_active = gtk_check_button_get_active(refs->exact_check);
    if (exact_active) {
       iname_option = "-name";
       iname_query = g_strdup_printf ("%s \"%s\"", iname_option, query);
    } else {
       iname_option = "-iname";
       iname_query = g_strdup_printf ("%s \"*%s*\"", iname_option, query);
    }

    find_cmd = g_strdup_printf(
    "run0 --background=0 --unit=finden --via-shell %s / %s", find_path, iname_query); 

    g_free (iname_query);
} // Hinweis: g_free(find_cmd) erfolgt in Suchfunktion;


/* ----- Callback Beenden-Button ------------------------------------------------------- */
static void on_quitbtn_clicked(GtkButton *button, gpointer user_data) 
    { 
    GtkWindow *win = GTK_WINDOW(user_data);
    gtk_window_destroy(win);
    // Aufräumen:
    UiRefs *refs = g_object_get_data (G_OBJECT (button), "ui_refs");
    if (refs) g_free (refs);
    }

/* ----- Callback für beide Kontrollkästchen (Toggle) ----------------------------------- */
static void on_check_button_toggled (GtkCheckButton *toggle_check, gpointer user_data) 
{
    /* UiRefs Zeiger */
    UiRefs *refs = user_data; 

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
        /* Wenn Snapshots aktiviert wird, Root ebenfalls aktivieren */
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
        g_print("Please enter a search term\n");
        return;
    }

    /* 0. ---- Struktur dient für "Schalter" ---- */

    /* 1. ---- Tool-"find"‑ermitteln --------------------------------------------------------- */
    const gchar *find_prog = "find";
    gchar *find_path = g_find_program_in_path (find_prog);
    if (!find_path) {
        const gchar *fallback = "/usr/bin/find";
        if (g_file_test (fallback, G_FILE_TEST_IS_EXECUTABLE)) // auf Ausführbarkeit testen
            find_path = g_strdup (fallback);
    }
    if (!find_path) {
        g_warning ("Abort: tool %s not found!\n", find_prog);
        // Alert-Dialog noch einfügen !!!
        return;
    }

    /* 2. ---- Prüfen, ob die Checkboxen aktiviert ------------------------------------------- */
        gboolean root_active = FALSE;
        gboolean snapshots_active = FALSE;
  
        if (GTK_IS_CHECK_BUTTON(refs->root_check))
            root_active = gtk_check_button_get_active(refs->root_check);
        if (GTK_IS_CHECK_BUTTON(refs->snapshots_check))
            snapshots_active = gtk_check_button_get_active(refs->snapshots_check);


    /* 2.1 ---- Debug-Ausgaben --------------------------------------------------------------- */
    g_print("switch root: %s\n",
        root_active ? "true" : "false");

    g_print("switch snapshots: %s\n",
        snapshots_active ? "true" : "false");

    /* 3. ---- Modus bestimmen --------------------------------------------------------------- */
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

    /* ---- 4. Schalter aus 3.x - für ROOT, RUN0, SNAPSHOTS + Find-Kommande ------------------ */
    void (*cmd_action)(const char *, const char *, UiRefs *) = NULL;
    cmd_action = action_A; // Find-Kommando [A] wird in "static void action_A" definiert
    // Diese Maßnahme war notwendig, da sonst nested-Fuction innerhalb einer Funktion entsteht!

    switch (mode)
    {
        case ROOT_OFF:  // ROOT=0
            g_print("Search function is performed only in user directory\n"); // testen
            /* find Kommando [A] verwenden  */
            cmd_action = action_A; // action_A wird in "static void action_A" definiert
            break;

        case FLATPAK_DISABLE: // FLATPAK=1 - Optionen deaktivieren
        {    
            g_print("Application is Flatpak version; options are disabled!\n");
            
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
            g_print("Service run0 not available\n");

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
            g_print("Service run0 is available\n");
            /* find Kommando [B] verwenden  */
            cmd_action = action_B; // action_B wird in "static void action_B" definiert
            break;

       case ROOT_RUN0_SNAPSHOTS_OK: // ROOT=1, RUN0=1, SNAPSHOTS=1
            g_print("Service run0 is available\nSearching in path \".snapshots\" is activated\n");
            /* find Kommando [C] verwenden  */
            cmd_action = action_C; // action_C wird in "static void action_C" definiert
            break;
    } // Switch(mode) Ende

    /* 5. ---- Aktion für das find-Kommando aufrufen ------------------------------------- */
    if (cmd_action) cmd_action(find_path, query, refs);  

 /* Hinweis - cmd_action beinhaltet:
    action_A:
    "/usr/bin/find ~/ -iname \"*<query>*\""
  
    action_B:
    "run0 --background=0 --unit=finden --via-shell /usr/bin/find / -path \"/.snapshots\" -prune -o -iname \"*<query>*\""
  
    action_C:
    "run0 --background=0 --unit=finden --via-shell /usr/bin/find / -iname \"*<query>*\""
 */
    g_free(find_path);

    /* 6. ---- Terminal im System auswählen ---------------------------------------------- */
    gchar *term_path = NULL;

   /* T1. miniterm = true  */
   if (g_cfg.miniterm_enable) {
        printf("[t] In settings, miniterm is enabled!\n");
        /* t1.1 miniterm-Pfad global hinterlegt? */
        if (glob_mini && g_str_has_prefix(glob_mini, "toq-miniterm")) {
            /* t1.2 miniterm-Pfad als Terminal-Pfad übergeben */
            term_path = glob_path_miniterm;
        }
   //g_print("[t] Known path to miniterm = %s\n", term_path); // testen
   } else {

      /* T2. miniterm = false  */
      term_path = glob_term_path;

      /* kein Terminal */
      if (!term_path) {
            g_warning ("[t] No supported terminal found!\n");
            GtkWindow *parent = GTK_WINDOW (gtk_widget_get_root (GTK_WIDGET (button)));
            /* Alert Dialog */
            if (parent)
                show_alert_dialog (parent,
                               _("Kein Terminal gefunden"),
                               _("Es konnte kein unterstütztes Terminal auf diesem System gefunden werden!"));
            g_free (find_cmd);
            return;
      }
   }

    /* 7. Erweiterte Terminal-Option bestimmen -------------------------------------------- */
    const gchar *term_option;                                       // Terminals mit Option "--"
    if (g_str_has_suffix(glob_term_name, "gnome-terminal") ||
         g_str_has_suffix(glob_term_name, "miniterm") ||
          g_str_has_suffix(glob_term_name, "toq-miniterm") ||
           g_str_has_suffix(glob_term_name, "kgx") ||
            g_str_has_suffix(glob_term_name, "terminator") ||
             g_str_has_suffix(glob_term_name, "tilix") ||
              g_str_has_suffix(glob_term_name, "kitty") ||
               g_str_has_suffix(glob_term_name, "alacritty") ||
                g_str_has_suffix(glob_term_name, "foot") ||
                 g_str_has_suffix(glob_term_name, "wezterm"))
        term_option = "--";
    else if (g_str_has_suffix(glob_term_name, "xfce4-terminal")) {  // Xfce4-Terminal mit Option "-e"
        term_option = "-x"; 
    }
    else
        term_option = "-e";                                 // alle anderen wie KDE-Konsole(<22.12), xterm, Standard-Option: "-e"

    /* 7. ---- Kommando zusammenbauen ------------------------------------------------------ */
    gchar *full_cmd = g_strdup_printf("%s; exec bash", find_cmd);

    /* komplettes Kommando ausgeben */
    g_print ("command: %s %s %s\n", glob_term_name, term_option, full_cmd);  //zum testen !!

    /* 8. ---- Argument-Liste vorbereiten, Beispiel --------------------------------------- */
    /* argv = "/usr/bin/toq-miniterm -- bash -c "/usr/bin/find / -iname \"*example*\"; exec bash"" */
    /* g_strdup   Funktion zum kopieren eines Strings in ein neu angel. Speicherplatz + Zeiger darauf */
    gchar *argv[] = {
        term_path,
        g_strdup(term_option),   // Option für nicht-Gnome-Terminals, siehe Terminal-Option oben
        g_strdup("bash"),
        g_strdup("-c"),
        full_cmd,
        NULL
    };
    /* 9. ---- Terminal starten ---------------------------------------------------------- */
    GError *error = NULL;

    gchar *cwd = g_get_current_dir ();   // Arbeitsverzeichnis
    if (!g_spawn_async (cwd, argv, NULL, G_SPAWN_DEFAULT, NULL, NULL, NULL, &error)) {
        g_warning ("Error starting the terminal: %s", error->message);
        g_error_free (error);
    }
    g_free (cwd);

    /* 10. ---- Speicher freigeben  (v4) ------------------------------------------------- */
    /* argv[1..n] wurden mit g_strdup erzeugt (argv[0] ist term_path, argv[3] = full_cmd)  */
    for (int i = 1; argv[i] != NULL; i++)
        g_free (argv[i]);

    /* term_path war entweder g_strdup(mini_path) oder Rückgabe von g_find_program_in_path() */
    /* term_path ist Ergebnis von g_find_program_in_path(), welches ein Zeiger auf glob_term_path hat!
       Lösung zum Eigentum erlangen und anschließend leeren: */
    term_path = g_strdup(term_path);
    g_free (term_path); 
    /* find_cmd und full_cmd einmalig freigeben (full_cmd wurde bereits durch argv-loop freigegeben!)*/
    g_free (find_cmd);  /* full_cmd) bereits via argv loop */

}

/* -------------------------------------------------------------*/
/*       Aktivierungshandler                                    */
/* -------------------------------------------------------------*/
static void on_activate (AdwApplication *app, gpointer)
{
    /* ----- Adwaita-Fenster ------------------------------------------------------------- */
    AdwApplicationWindow *adw_win = ADW_APPLICATION_WINDOW (adw_application_window_new (GTK_APPLICATION (app))); 

    gtk_window_set_title (GTK_WINDOW(adw_win), "Finden");         // WM-Titel
    gtk_window_set_default_size (GTK_WINDOW(adw_win), 510, 250);  // Standard-Fenstergröße
    gtk_window_set_resizable (GTK_WINDOW (adw_win), FALSE);       // Skalierung nicht erlauben

    /* --- Navigation Root ----- */
    AdwNavigationView *nav_view = ADW_NAVIGATION_VIEW(adw_navigation_view_new());
    adw_application_window_set_content (adw_win, GTK_WIDGET(nav_view));

    /* --- ToolbarBarView als Hauptseite ----- */
    AdwToolbarView *toolbar_view = ADW_TOOLBAR_VIEW(adw_toolbar_view_new());

    /* --- HeaderBar mit TitelWidget erstellt und dem ToolbarView hinzugefügt ------------ */
    AdwHeaderBar *header = ADW_HEADER_BAR (adw_header_bar_new());
    GtkWidget * title_label = gtk_label_new ("Finden");                 // Label für Fenstertitel
    gtk_widget_add_css_class (title_label, "heading");                  // .heading class
    adw_header_bar_set_title_widget (ADW_HEADER_BAR (header), GTK_WIDGET (title_label)); // Label einsetzen
    adw_toolbar_view_add_top_bar (toolbar_view, GTK_WIDGET (header));   // Header‑Bar zur Toolbar‑View hinzuf

    /* --- Nav_View mit Inhalt wird zur Hauptseite */
    AdwNavigationPage *main_page = adw_navigation_page_new(GTK_WIDGET(toolbar_view), "Finden");
    adw_navigation_view_push(nav_view, main_page);

    /* --- Hamburger‑Button innerhalb der Headerbar --- */
    GtkMenuButton *menu_btn = GTK_MENU_BUTTON (gtk_menu_button_new ());
    gtk_menu_button_set_icon_name (menu_btn, "open-menu-symbolic");
    adw_header_bar_pack_start (header, GTK_WIDGET (menu_btn));

    /* --- Popover‑Menu im Hamburger --- */
    GMenu *menu = g_menu_new ();
    g_menu_append (menu, _("Einstellungen     "), "app.show-settings");
    g_menu_append (menu, _("Infos zu Finden       "), "app.show-about");
    GtkPopoverMenu *popover = GTK_POPOVER_MENU (
    gtk_popover_menu_new_from_model (G_MENU_MODEL (menu)));
    gtk_menu_button_set_popover (menu_btn, GTK_WIDGET (popover));

    /* --- Action die den About‑Dialog öffnet --- */
    const GActionEntry about_entry[] = {
     { "show-about", show_about, NULL, NULL, NULL }
    };
    g_action_map_add_action_entries (G_ACTION_MAP(app), about_entry, G_N_ELEMENTS(about_entry), app);

    /* --- Action die die Einstellungen öffnet --- */
    const GActionEntry settings_entry[] = {
     { "show-settings", show_settings, NULL, NULL, NULL }
    }; 
    g_action_map_add_action_entries (G_ACTION_MAP(app), settings_entry, G_N_ELEMENTS(settings_entry), nav_view);

    /* ---- Haupt‑Box erstellen ----------------------------------------------------------- */
    GtkBox *mainbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 12));
    gtk_box_set_spacing(GTK_BOX(mainbox), 6);                // Abstand zwischen allen Elementen (vertikal)
    gtk_widget_set_margin_top    (GTK_WIDGET (mainbox), 12);
    gtk_widget_set_margin_bottom (GTK_WIDGET (mainbox), 12);
    gtk_widget_set_margin_start  (GTK_WIDGET (mainbox), 12);
    gtk_widget_set_margin_end    (GTK_WIDGET (mainbox), 12);
    gtk_widget_set_hexpand (GTK_WIDGET (mainbox), TRUE);
    gtk_widget_set_vexpand (GTK_WIDGET (mainbox), TRUE);

    /* ----- Text-Label erstellen  ----- */
    //GtkLabel *label1 = GTK_LABEL(gtk_label_new (NULL));
    //gtk_label_set_markup (label1, _("<b>Finden statt Suchen</b>")); // Titel in Markup
    //gtk_label_set_use_markup (label1, TRUE);                        // Markup-Parsing aktivieren
    //gtk_widget_set_halign (GTK_WIDGET (label1), GTK_ALIGN_CENTER);
    //gtk_widget_set_valign (GTK_WIDGET (label1), GTK_ALIGN_CENTER);

    /* ----- Smiley‑Image aus der Resource ----- */
    //GtkWidget *smiley = gtk_image_new_from_resource ("/free/toq/finden/smiley1");
    //gtk_image_set_pixel_size (GTK_IMAGE (smiley), 24);   // Smiley Größe

    /* ----- BOX-Widget für Text und Smiley erstellen ----- */
    //GtkBox *smileytext_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6)); // Abstand Icon
    //gtk_widget_set_halign (GTK_WIDGET (smileytext_box), GTK_ALIGN_CENTER);
    //gtk_widget_set_valign (GTK_WIDGET (smileytext_box), GTK_ALIGN_CENTER);

    /* ----- Smiley und TextLabel der Box hinzufügen ----- */ 
    //gtk_box_append (smileytext_box, GTK_WIDGET (label1)); // Label in Widget convertieren
    //gtk_box_append (smileytext_box, smiley); // Reihenfolge von Label und Smiley

    //gtk_box_append (GTK_BOX (mainbox), GTK_WIDGET (smileytext_box));

    /* ----- Suchleiste + Root‑Checkbutton oberhalb der Schaltfläche (horizontal) ---------- */
    GtkWidget *search_entry = NULL;   // Suchleisten-Widget
    GtkWidget *search_box   = NULL;   // Box-Widget in der sich die Suchleiste befindet
    GtkWidget *checkb_box = NULL;     // Box-Widget für Checkboxen

    if (!search_entry) {
    /* Suchfeld */
        search_entry = gtk_search_entry_new ();
        gtk_search_entry_set_placeholder_text (GTK_SEARCH_ENTRY (search_entry), 
                                                  _("Finden statt Suchen"));
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
    /* horizontale Search-BOX in die vertikale Haupt-BOX einfügen */
    gtk_box_append (mainbox, search_box);

    /* --- Horizontales Haupt-Box-Widget für Checkboxen erstellen -------------------------- */
    GtkWidget *cb_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_hexpand (cb_hbox, TRUE);          // Ausdehnung in die Breite
    gtk_widget_set_halign (cb_hbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (cb_hbox, GTK_ALIGN_START);

    /* ----- Linke innere vertikale Box für Checkboxen ------------------------------------- */
    GtkWidget *vbox_inside_left = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_hexpand (vbox_inside_left, FALSE);    // Ausdehnung in die Breite
    gtk_widget_set_halign (vbox_inside_left, GTK_ALIGN_START);

    /* --- (1.)Kontrallkästchen/Checkbox "snapshots" erstellen --- */
    GtkWidget *snapshots_check = gtk_check_button_new_with_label(_("Suche in Snapshots"));
    //gtk_widget_add_css_class (snapshots_check, "selection-mode");
    gtk_check_button_set_active (GTK_CHECK_BUTTON(snapshots_check), FALSE);
    gtk_box_append (GTK_BOX (vbox_inside_left), snapshots_check);

     /* --- (2.)Kontrollkästchen/Checkbox "root" erstellen --- */
    GtkWidget *root_check = gtk_check_button_new_with_label(_("Suche in System"));
    //gtk_widget_add_css_class (root_check, "selection-mode");
    gtk_check_button_set_active (GTK_CHECK_BUTTON(root_check), FALSE);
    gtk_box_append (GTK_BOX (vbox_inside_left), root_check);

    /* ----- Rechte innere vertikale Box für Checkboxen ------------------------------------ */
    GtkWidget *vbox_inside_right = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_hexpand (vbox_inside_right, FALSE);
    gtk_widget_set_halign (vbox_inside_right, GTK_ALIGN_START);

    /* --- (3.)Kontrollkästchen/Checkbox mit Namen "exact match" erstellen --- */
    GtkWidget *exact_check = gtk_check_button_new_with_label(_("Exakte Übereinstimmung"));
    //gtk_widget_add_css_class (exact_check, "selection-mode");
    gtk_check_button_set_active (GTK_CHECK_BUTTON(exact_check), FALSE);
    gtk_box_append (GTK_BOX (vbox_inside_right), exact_check);

    /* ------ Innere Boxen in die Checkboxen-Haupt-Box einfügen ---------------------------- */
    gtk_box_append (GTK_BOX (cb_hbox), vbox_inside_left);
    gtk_box_append (GTK_BOX (cb_hbox), vbox_inside_right);
    gtk_box_append (GTK_BOX (mainbox), cb_hbox);   // und in die Mainbox einfügen


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
    refs->snapshots_check = GTK_CHECK_BUTTON(snapshots_check); // Pointer zur Snapshots-Checkbox
    refs->exact_check     = GTK_CHECK_BUTTON(exact_check);     // ...und Exact_Checkbox.



    /* --- Schaltflächen-WidgetBox hier anlegen: ------------------------------------------- */
    GtkWidget *button_hbox = NULL;
    if (!button_hbox) {
        button_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 48); // Abstand zwischen den Buttons
        gtk_widget_set_margin_top     (GTK_WIDGET (button_hbox), 6); // Abstand zwischen den Boxen
        gtk_widget_set_hexpand(button_hbox, FALSE);                 // nicht ausdehnen!!
        gtk_widget_set_vexpand(button_hbox, FALSE);
        gtk_widget_set_halign(button_hbox, GTK_ALIGN_CENTER);
    }

    /* --- Schaltfläche-Finden in Thema Akzent --------------------------------------------- */
    GtkWidget *search_button = gtk_button_new_with_label (_("     Finden     "));
    gtk_widget_add_css_class (search_button, "suggested-action");

    /* --- Schaltfläche Finden verbinden --- */
    g_signal_connect(search_button, "clicked",
                      G_CALLBACK(on_search_button_clicked), refs); //user_data = refs

    /* --- Suchleiste verbinden für ENTER Taste --- */
    g_signal_connect(search_entry,  "activate", G_CALLBACK(on_search_button_clicked), refs);

    /* ----- Schaltfläche Beenden ---------------------------------------------------------- */
    GtkWidget *quit_button = gtk_button_new_with_label(_("   Beenden   "));

    /* ---- Schaltfläche Signal verbinden ---- */
           // Methode um Anwendung mit jeglichen Instanzen zu schließen:
//            g_signal_connect(quit_button, "clicked", G_CALLBACK(on_quitbtn_clicked), app);
      // Nethode um das Fenster zu schließen, egal wieviele Instanzen existieren:
         g_signal_connect(quit_button, "clicked", G_CALLBACK(on_quitbtn_clicked), adw_win);

    /* ---- Kontrollkästchen Signal verbinden ---- */
    // Toggel für beide Checkboxen, sowie um den Fokus für die Suchleiste wieder neu zu setzen!
    g_signal_connect(snapshots_check, "toggled", G_CALLBACK(on_check_button_toggled), refs);
    g_signal_connect(root_check, "toggled", G_CALLBACK(on_check_button_toggled), refs);
    g_signal_connect(exact_check, "toggled", G_CALLBACK(on_check_button_toggled), refs);

    /* ----- Schaltfläche der Box hinzufügen ----- */
    gtk_box_append(GTK_BOX(button_hbox), quit_button);    
    gtk_box_append(GTK_BOX(button_hbox), search_button);
    gtk_widget_set_margin_top (button_hbox, 24); // Abstand zum vorherigen Element

    /* --- Checkboxen am Search-Button speichern, um diese im Callback abrufen zu können --- */
    g_object_set_data(G_OBJECT(search_button), "root_check", root_check);
    g_object_set_data(G_OBJECT(search_button), "snapshots_check", snapshots_check);
    g_object_set_data(G_OBJECT(search_button), "exact_check", exact_check);

    /* -----  Box zur ToolbarView hinzufügen ----------------------------------------------- */
    gtk_box_append(GTK_BOX(mainbox), button_hbox);
    adw_toolbar_view_set_content(toolbar_view, GTK_WIDGET(mainbox));

    /* ----- Fenster desktop‑konform anzeigen ---------------------------------------------- */
    gtk_window_present(GTK_WINDOW(adw_win));
    gtk_widget_grab_focus(search_entry); //fokus auf Suchleiste

}


/* -------------------------------------------------------------*
 * Anwendungshauptteil, main()                                  *
 * -------------------------------------------------------------*/
int main (int argc, char **argv)
{

    const char *locale_path = NULL;

    /* Resource‑Bundle (finden.gresource) registrieren um den Inhalt verfügbar zu machen */
    g_resources_register (resources_get_resource ()); // reicht für Icon innerhalb der App



    init_environment(); // Environment ermitteln, global in config.c
    init_config();      // Config File laden/erstellen in config.c
    //   g_print ("Settings load miniterm value: %s\n", g_cfg.miniterm_enable ? "true" : "false"); // testen !!
    //   g_print ("Settings load test value: %s\n", g_cfg.test_enable ? "true" : "false"); // testen !!
    //save_config ();     // Config File speichern in config.c // hier noch als Test !!

    /* ----- Erstelle den Pfad zu den locale-Dateien ----------------------------------- */
    init_flatpak();
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
    g_print ("Lokalization path: %s \n", locale_path); // testen

    /* Prüfen ob miniterm im lokalem Pfad */
    find_terminals();

    /* GTK/Adwaita Anwendung: */
    g_autoptr (AdwApplication) app =                        // Instanz erstellen + App-ID + Default-Flags;
        adw_application_new ("free.toq.finden", G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL); // Signal zu on_activate

    /* Event_Loop */
    int ret = g_application_run (G_APPLICATION (app), argc, argv);

    config_cleanup ();         // Config cleanup
    g_free (glob_term_path);   // aus global find_terminals()
    return ret;
}
