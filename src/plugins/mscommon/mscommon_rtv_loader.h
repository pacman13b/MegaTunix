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

#ifndef __MSCOMMON_RTV_LOADER_H__
#define __MSCOMMON_RTV_LOADER_H__

#include <defines.h>
#include <gtk/gtk.h>
#include <threads.h>

/* Externs */
extern gint (*translate_string_f)(const gchar *);
/* Externs */

/* Prototypes */
void common_rtv_loader(gconstpointer *, ConfigFile *, gchar *, gint, ComplexExprType);
/* Prototypes */

#endif