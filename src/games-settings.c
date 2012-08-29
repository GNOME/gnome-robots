/*
 *  Copyright © 2007, 2010 Christian Persch
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 3, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope conf it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include "games-settings.h"

#include <gtk/gtk.h>

#define I_(string) g_intern_static_string (string)

#define SCHEMA_NAME           I_("org.gnome.Games.WindowState")

#define STATE_KEY_MAXIMIZED   I_("maximized")
#define STATE_KEY_FULLSCREEN  I_("fullscreen")
#define STATE_KEY_WIDTH       I_("width")
#define STATE_KEY_HEIGHT      I_("height")

typedef struct {
  GSettings *settings;
  GtkWindow *window;
  int width;
  int height;
  guint is_maximised : 1;
  guint is_fullscreen : 1;
} WindowState;

static void
free_window_state (WindowState *state)
{
  /* Now store the settings */
  g_settings_set_int (state->settings, STATE_KEY_WIDTH, state->width);
  g_settings_set_int (state->settings, STATE_KEY_HEIGHT, state->height);
  g_settings_set_boolean (state->settings, STATE_KEY_MAXIMIZED, state->is_maximised);
  g_settings_set_boolean (state->settings, STATE_KEY_FULLSCREEN, state->is_fullscreen);

  g_settings_apply (state->settings);

  g_object_unref (state->settings);

  g_slice_free (WindowState, state);
}

static gboolean
window_configure_event_cb (GtkWidget *widget,
                           GdkEventConfigure *event,
                           WindowState *state)
{
  if (!state->is_maximised && !state->is_fullscreen) {
    state->width = event->width;
    state->height = event->height;
  }

  return FALSE;
}

static gboolean
window_state_event_cb (GtkWidget *widget,
                       GdkEventWindowState *event,
                       WindowState *state)
{
  if (event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED) {
    state->is_maximised = (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;
  }
  if (event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN) {
    state->is_fullscreen = (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0;
  }

  return FALSE;
}

/**
 * games_settings_set_keyval:
 * @settings: a #GSettings
 * @key: the key name
 * @keyval: the value to store
 * @modifiers: key modifiers with @keyval
 *
 * Associates @keyval with the key @key in group @group.
 *
 * It is a programmer error to pass a key that isn't valid for settings.
 *
 * Returns: TRUE if setting the key succeeded, FALSE if the key was not writable
 */
gboolean
games_settings_set_keyval (GSettings *settings,
                           const char *key,
                           guint keyval,
                           GdkModifierType modifiers)
{
  char *value;
  gboolean rv;

  g_return_val_if_fail (G_IS_SETTINGS (settings), FALSE);
  g_return_val_if_fail (key != NULL && key[0] != '\0', FALSE);

  value = gtk_accelerator_name (keyval, modifiers);
  rv = g_settings_set_string (settings, key, value);
  g_free (value);

  return rv;
}

/**
 * games_settings_bind_window_state:
 * @path: a valid #GSettings path
 * @window: a #GtkWindow
 *
 * Restore the window configuration, and persist changes to the window configuration:
 * window width and height, and maximised and fullscreen state.
 * @window must not be realised yet.
 *
 * To make sure the state is saved at exit, g_settings_sync() must be called.
 */
void
games_settings_bind_window_state (const char *path,
                                  GtkWindow *window)
{
  WindowState *state;
  int width, height;
  gboolean maximised, fullscreen;

  g_return_if_fail (GTK_IS_WINDOW (window));
  g_return_if_fail (!gtk_widget_get_realized (GTK_WIDGET (window)));

  state = g_slice_new0 (WindowState);

  state->window = window;
  state->settings = g_settings_new_with_path (SCHEMA_NAME, path);

  /* We delay storing the state until exit */
  g_settings_delay (state->settings);

  g_object_set_data_full (G_OBJECT (window), "GamesSettings::WindowState",
                          state, (GDestroyNotify) free_window_state);

  g_signal_connect (window, "configure-event",
                    G_CALLBACK (window_configure_event_cb), state);
  g_signal_connect (window, "window-state-event",
                    G_CALLBACK (window_state_event_cb), state);

  maximised = g_settings_get_boolean (state->settings, STATE_KEY_MAXIMIZED);
  fullscreen = g_settings_get_boolean (state->settings, STATE_KEY_FULLSCREEN);
  width = g_settings_get_int (state->settings, STATE_KEY_WIDTH);
  height = g_settings_get_int (state->settings, STATE_KEY_HEIGHT);

  if (width > 0 && height > 0) {
    gtk_window_set_default_size (GTK_WINDOW (window), width, height);
  }
  if (maximised) {
    gtk_window_maximize (GTK_WINDOW (window));
  }
  if (fullscreen) {
    gtk_window_fullscreen (GTK_WINDOW (window));
  }
}
