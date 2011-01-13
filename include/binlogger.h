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

#ifndef __BINLOGGER_H__
#define __BINLOGGER_H__

#include <gtk/gtk.h>

/* Prototypes */
void open_binary_logs(void);
void close_binary_logs(void);
gboolean flush_binary_logs(gpointer);
/* Prototypes */

#endif