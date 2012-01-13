/*
 *  Wikipedia Lookup - A third-party Pidgin plug-in which offers
 *					  you the possibility to look up received and
 *					  typed words on Wikipedia.
 *
 *  Copyright (C) 2011 Hendrik Kunert kunerd@users.sourceforge.net
 *
 *  This file is part of wplookup.
 *
 *  wplookup is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Foobar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with wplookup.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WPL_PIDGIN_PREVIEW_H
#define WPL_PIDGIN_PREVIEW_H

#include <gtk/gtk.h>
#include <gtkutils.h>
#include <webkit/webkit.h>

#include "wparticle.h"


// TODO: remove if no more needed
struct PreviewData {
        GtkWidget *parent_window;
        gchar *search_text;
};

typedef struct {
    WikipediaArticle *article;
    gchar *search_text;
    GtkWidget *parent_window;
} WplPidginPreview;

WplPidginPreview *WplPidginPreview_construct();
void WplPidginPreview_destruct(WplPidginPreview *o);

void wpview_open_preview_window(void *preview_data);

void wpview_right_click_popup(GtkTextView *text_view, GtkMenu *menu, WikipediaLookup *wpl);

#endif
