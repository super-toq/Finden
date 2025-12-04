/*
 * Implementierung: • init_environment() ermittelt app_dir und config_path. • init_config() lädt (oder erzeugt) die *.cfg‑Datei, füllt g_cfg.miniterm_enable. • save_config() schreibt zurück. • config_cleanup() räumt auf.
 *
 */

#include "config.h"
#include <glib.h>

/* ----- Globale Variablen ------------------ */
static gchar   *app_dir     = NULL;    /* Pfad zum Executable‑Verzeichnis */
static gchar   *config_path = NULL;    /* Vollständiger Pfad zur .cfg‑Datei */
static GKeyFile *key_file   = NULL;    /* In‑Memory‑Repräsentation der Datei */

/* ----- Globale Struktur der Keys ---------- */
FindConfig g_cfg = {
    .miniterm_enable = FALSE,  // Standard-Wert, falls alles fehlschlägt
    .test_enable     = FALSE
};
/* ----- Getter für app_dir ---------------- */
const gchar *get_app_dir (void)
{
    return app_dir;
}

/* ----- Environment ----- */
void init_environment (void)
{


/* ----- Executable-Pfad ----------------------------------------------------------- */
    gchar *exe_path = g_file_read_link ("/proc/self/exe", NULL);
    if (!exe_path) {
        g_warning ("[C] The exe_path cannot be created");
        return;
    }
    app_dir = g_path_get_dirname (exe_path);
    g_print ("[C] Application path: %s\n", app_dir);
    g_free (exe_path);

    /* ----- Home-Verzeichnis und Config-Pfad ----- */
    const gchar *home = g_get_home_dir ();
    if (!home) {
        g_warning ("[C] HOME directory not determinable");
        return;
    }

    config_path = g_build_filename (home, ".config", "toq-finden", "finden.cfg", NULL);
}


/* ----- Config‑Initialisierung - Laden / Anlegen ----------------------------------------- */
void init_config (void)
{
    if (!config_path) {
        g_warning ("[C] Abort, the config_path has not been set!");
        return;
    }

    /* -----  1. Prüfen, ob das Verzeichnis schon existiert oder angelegt werden muss ---- */
    gchar *config_dir = g_path_get_dirname (config_path);
    if (!g_file_test (config_dir, G_FILE_TEST_IS_DIR))
        g_mkdir_with_parents (config_dir, 0700);
    g_free (config_dir);

    /* ----- GKeyFile anlegen ------------------------------ */
    key_file = g_key_file_new ();

    /* ----- Datei laden (falls vorhanden) ----- */
    if (g_file_test (config_path, G_FILE_TEST_IS_REGULAR)) {
        GError *err = NULL;
        if (!g_key_file_load_from_file (key_file, config_path, G_KEY_FILE_KEEP_COMMENTS, &err)) {
            g_warning ("[C] Configuration loading failed: %s - using default value", err->message);
            g_error_free (err);
        }
    }

    /* ----- Default-Werte setzen, falls Schlüssel fehlen ----- */
    // Key1 Miniterm
     if (!g_key_file_has_key (key_file, "General", "miniterm_enable", NULL)) {
        g_key_file_set_boolean (key_file, "General", "miniterm_enable", TRUE);
    }
    // Key2 Test
    if (!g_key_file_has_key (key_file, "General", "test_enable", NULL)) {
        g_key_file_set_boolean (key_file, "General", "test_enable", FALSE);
    }


    /* ----- Laden der Werte in die globale Struktur "g_cfg.x" ----- */
    // Key1
    g_cfg.miniterm_enable = g_key_file_get_boolean (key_file, "General", "miniterm_enable", NULL);
    // Key2
    g_cfg.test_enable = g_key_file_get_boolean (key_file, "General", "test_enable", NULL);
}


/* ----- Config speichern (aus globaler Struktur) ---------------------------------------- */
void save_config (void)
{
    if (!key_file || !config_path) return;

    /* Werte aus g_cfg in die GKeyFile‑Instanz schreiben */
    // Key1
    g_key_file_set_boolean (key_file, "General", "miniterm_enable", g_cfg.miniterm_enable);
    // Key2
    g_key_file_set_boolean (key_file, "General", "test_enable", g_cfg.test_enable);


    /* Fehlerbehandlung */
    GError *err = NULL;
    if (!g_key_file_save_to_file (key_file, config_path, &err)) {
        g_warning ("[C] Failed to save configuration: %s", err->message);
        g_error_free (err);
    }
}


/* ------ Aufräumen -------------------------------------------------------------------- */
void config_cleanup (void)
{
    if (key_file) g_key_file_free (key_file);
    g_free (app_dir);
    g_free (config_path);
}
