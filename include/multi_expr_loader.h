/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute, etc. this as long as all the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#ifndef __MULTI_EXPR_LOADER_H__
#define __MULTI_EXPR_LOADER_H__

#include <configfile.h>
#include <gtk/gtk.h>
#include <structures.h>

/* Prototypes */
void load_multi_expressions(GObject * ,ConfigFile * ,gchar * );
void free_multi_expr(gpointer);
void free_multi_source(gpointer);

/* Prototypes */

#endif
