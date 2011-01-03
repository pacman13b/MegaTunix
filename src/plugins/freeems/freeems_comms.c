/*
 * Copyright (C) 2003 by Dave J. Andruczyk <djandruczyk at yahoo dot com>
 *
 * Linux Megasquirt tuning software
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

#include <config.h>
#include <debugging.h>
#include <defines.h>
#include <freeems_comms.h>
#include <freeems_plugin.h>
#include <gtk/gtk.h>
#include <packet_handlers.h>
#ifdef __WIN32__
#include <winsock2.h>
#else
#include <poll.h>
#endif
#include <serialio.h>
#include <template.h>
#include <sys/select.h>
#include <unistd.h>


extern gconstpointer *global_data;

G_MODULE_EXPORT void *serial_repair_thread(gpointer data)
{
	/* We got sent here because of one of the following occurred:
	 * Serial port isn't opened yet (app just fired up)
	 * Serial I/O errors (missing data, or failures reading/writing)
	 *  - This includes things like pulling the RS232 cable out of the ECU
	 * Serial port disappeared (i.e. device hot unplugged)
	 *  - This includes unplugging the USB side of a USB->Serial adapter
	 *    or going out of bluetooth range, for a BT serial device
	 *
	 * Thus we need to handle all possible conditions if possible
	 */
	static gboolean serial_is_open = FALSE; /* Assume never opened */
	static GAsyncQueue *io_repair_queue = NULL;
	gchar * potential_ports;
	gint len = 0;
	gboolean autodetect = FALSE;
	guchar buf [1024];
	gchar ** vector = NULL;
	guint i = 0;
	Serial_Params *serial_params = NULL;
	void (*unlock_serial_f)(void) = NULL;
	void (*close_serial_f)(void) = NULL;
	gboolean (*open_serial_f)(const gchar *,gboolean) = NULL;
	gboolean (*lock_serial_f)(const gchar *) = NULL;
	void (*setup_serial_params_f)(void) = NULL;

	serial_params = DATA_GET(global_data,"serial_params");

	get_symbol_f("setup_serial_params",(void *)&setup_serial_params_f);
	get_symbol_f("open_serial",(void *)&open_serial_f);
	get_symbol_f("close_serial",(void *)&close_serial_f);
	get_symbol_f("lock_serial",(void *)&lock_serial_f);
	get_symbol_f("unlock_serial",(void *)&unlock_serial_f);
	dbg_func_f(THREADS|CRITICAL,g_strdup(__FILE__": serial_repair_thread()\n\tThread created!\n"));

	if (DATA_GET(global_data,"offline"))
	{
		g_timeout_add(100,(GSourceFunc)queue_function_f,"kill_conn_warning");
		dbg_func_f(THREADS|CRITICAL,g_strdup(__FILE__": serial_repair_thread()\n\tThread exiting, offline mode!\n"));
		g_thread_exit(0);
	}
	if (!io_repair_queue)
		io_repair_queue = DATA_GET(global_data,"io_repair_queue");
	/* IF serial_is_open is true, then the port was ALREADY opened 
	 * previously but some error occurred that sent us down here. Thus
	 * first do a simple comms test, if that succeeds, then just cleanup 
	 * and return,  if not, close the port and essentially start over.
	 */
	if (serial_is_open == TRUE)
	{
		dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Port considered open, but throwing errors\n"));
		freeems_serial_disable();
		close_serial_f();
		unlock_serial_f();
		serial_is_open = FALSE;
		/* Fall through */
	}
	while (!serial_is_open)
	{
		/* If "leaving" flag set, EXIT now */
		if (DATA_GET(global_data,"leaving"))
			g_thread_exit(0);
		dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Port NOT considered open yet.\n"));
		autodetect = (GBOOLEAN) DATA_GET(global_data,"autodetect_port");
		if (!autodetect) /* User thinks he/she is S M A R T */
		{
			potential_ports = (gchar *)DATA_GET(global_data, "override_port");
			if (potential_ports == NULL)
				potential_ports = (gchar *)DATA_GET(global_data,"potential_ports");
		}
		else    /* Auto mode */
			potential_ports = (gchar *)DATA_GET(global_data,"potential_ports");
		vector = g_strsplit(potential_ports,",",-1);
		for (i=0;i<g_strv_length(vector);i++)
		{
			if (DATA_GET(global_data,"leaving"))
			{
				g_strfreev(vector);
				g_thread_exit(0);
			}
			/* Message queue used to exit immediately */
			if (g_async_queue_try_pop(io_repair_queue))
			{
				g_timeout_add(300,(GSourceFunc)queue_function_f,"kill_conn_warning");
				dbg_func_f(THREADS|CRITICAL,g_strdup(__FILE__": serial_repair_thread()\n\tThread exiting, told to!\n"));
				g_thread_exit(0);
			}
			if (!g_file_test(vector[i],G_FILE_TEST_EXISTS))
			{
				dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Port %s does NOT exist\n",vector[i]));

				/* Wait 200 ms to avoid deadlocking */
				g_usleep(200000);
				continue;
			}
			g_usleep(100000);
			dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Attempting to open port %s\n",vector[i]));
			thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("Attempting to open port %s\n"),vector[i]),FALSE,FALSE);
			if (lock_serial_f(vector[i]))
			{
				if (open_serial_f(vector[i],TRUE))
				{
					if (autodetect)
						thread_update_widget_f("active_port_entry",MTX_ENTRY,g_strdup(vector[i]));
					dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Port %s opened\n",vector[i]));
					setup_serial_params_f();
					freeems_serial_enable();

					thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("Searching for ECU\n")),FALSE,FALSE);
					dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Performing ECU comms test via port %s.\n",vector[i]));
					if (comms_test())
					{       /* We have a winner !!  Abort loop */
						thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("Search successfull\n")),FALSE,FALSE);
						serial_is_open = TRUE;
						break;
					}
					else
					{
						dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t COMMS test failed, no ECU found, closing port %s.\n",vector[i]));
						thread_update_logbar_f("comms_view",NULL,g_strdup_printf(_("No ECU found...\n")),FALSE,FALSE);
						freeems_serial_disable();
						close_serial_f();
						unlock_serial_f();
						/*g_usleep(100000);*/
					}
				}
				g_usleep(100000);
			}
			else
			{
				dbg_func_f(SERIAL_RD|SERIAL_WR,g_strdup_printf(__FILE__" serial_repair_thread()\n\t Port %s is open by another application\n",vector[i]));
				thread_update_logbar_f("comms_view","warning",g_strdup_printf(_("Port %s is open by another application\n"),vector[i]),FALSE,FALSE);
			}
		}
		queue_function_f("conn_warning");
	}

	if (serial_is_open)
	{
		queue_function_f("kill_conn_warning");
		thread_update_widget_f("active_port_entry",MTX_ENTRY,g_strdup(vector[i]));
	}
	if (vector)
		g_strfreev(vector);
	dbg_func_f(THREADS|CRITICAL,g_strdup(__FILE__": serial_repair_thread()\n\tThread exiting, device found!\n"));
	g_thread_exit(0);
	return NULL;
}
		

G_MODULE_EXPORT void freeems_serial_disable(void)
{
	GIOChannel *channel = NULL;
	GAsyncQueue *queue = NULL;
	Serial_Params *serial_params = NULL;
	GTimeVal now;
	GCond *cond = NULL;
	GMutex *mutex = g_mutex_new();
	GThread *thread = NULL;
	gboolean res = FALSE;
	gint tmpi = 0;

	DATA_SET(global_data,"serial_abort",GINT_TO_POINTER(TRUE));
	thread = DATA_GET(global_data,"serial_thread_id");
	g_mutex_lock(mutex);
	g_get_current_time(&now);
	/* Wait up to 0.5 seconds for thread to exit */
        g_time_val_add(&now,500000);
        cond = DATA_GET(global_data,"serial_reader_cond");
        res = g_cond_timed_wait(cond,mutex,&now);
	g_thread_join(thread);
	DATA_SET(global_data,"serial_thread_id",NULL);
	/*
	if (res)
		printf("condition signaled\n");
	else
		printf("cond timeout\n");
	*/
	g_mutex_unlock(mutex);
	g_mutex_free(mutex);
}


G_MODULE_EXPORT void freeems_serial_enable(void)
{
	GIOChannel *channel = NULL;
	Serial_Params *serial_params = NULL;
	GThread *thread = NULL;
	gint tmpi = 0;

	serial_params = DATA_GET(global_data,"serial_params");
	if ((!serial_params->open) || (!serial_params->fd))
	{
		dbg_func_f(CRITICAL,g_strdup(_(__FILE__": freeems_serial_setup, serial port is NOT open, or filedescriptor is invalid!\n")));
		return;
	}

	DATA_SET(global_data,"serial_abort",GINT_TO_POINTER(FALSE));
#ifdef __WIN32__
	thread = g_thread_create(win32_reader,GINT_TO_POINTER(serial_params->fd),TRUE,NULL);
	
#else
	thread = g_thread_create(unix_reader,GINT_TO_POINTER(serial_params->fd),TRUE,NULL);
#endif
	DATA_SET(global_data,"serial_thread_id",thread);
	return;
}


G_MODULE_EXPORT gboolean comms_test(void)
{
	GAsyncQueue *queue = NULL;
	FreeEMS_Packet *packet = NULL;
	GCond *cond = NULL;
	gboolean res = FALSE;
	GTimeVal tval;
	gint len = 0;
	/* Packet sends back Interface Version */
	/* START, Header, Payload ID H, PAyload ID L, CKsum, STOP */
	guint8 *buf = NULL;
	/* Raw packet */
	guint8 pkt[INTVER_REQ_PKT_LEN];
	guint8 sum = 0;
	gint tmit_len = 0;
	gint i = 0;

	Serial_Params *serial_params = NULL;

	serial_params = DATA_GET(global_data,"serial_params");
	queue = DATA_GET(global_data,"packet_queue");

	dbg_func_f(SERIAL_RD,g_strdup(__FILE__": comms_test()\n\t Entered...\n"));
	if (!serial_params)
		return FALSE;
	queue = g_async_queue_new();
	register_packet_queue(PAYLOAD_ID,queue,RESPONSE_BASIC_DATALOG);
	g_get_current_time(&tval);
	g_time_val_add(&tval,500000);
	packet = g_async_queue_timed_pop(queue,&tval);
	deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_BASIC_DATALOG);
	if (packet)
	{
		dbg_func_f(SERIAL_RD,g_strdup(__FILE__": comms_test()\n\tFound streaming ECU!!\n"));
		g_async_queue_unref(queue);
		freeems_packet_cleanup(packet);
		DATA_SET(global_data,"connected",GINT_TO_POINTER(TRUE));
		return TRUE;
	}
	else
	{ /* Assume ECU is in non-streaming mode, try and probe it */
		dbg_func_f(SERIAL_RD,g_strdup(__FILE__": comms_test()\n\tRequesting FreeEMS Interface Version\n"));
		register_packet_queue(PAYLOAD_ID,queue,RESPONSE_INTERFACE_VERSION);
		pkt[HEADER_IDX] = 0;
		pkt[H_PAYLOAD_IDX] = (REQUEST_INTERFACE_VERSION & 0xff00 ) >> 8;
		pkt[L_PAYLOAD_IDX] = (REQUEST_INTERFACE_VERSION & 0x00ff );
		for (i=0;i<INTVER_REQ_PKT_LEN-1;i++)
			sum += pkt[i];
		pkt[INTVER_REQ_PKT_LEN-1] = sum;
		buf = finalize_packet((guint8 *)&pkt,INTVER_REQ_PKT_LEN,&tmit_len);

		if (!write_wrapper_f(serial_params->fd, buf, tmit_len, &len))
		{
			g_free(buf);
			deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_INTERFACE_VERSION);
			g_async_queue_unref(queue);
			return FALSE;
		}
		g_free(buf);
		g_get_current_time(&tval);
		g_time_val_add(&tval,500000);
		packet = g_async_queue_timed_pop(queue,&tval);
		deregister_packet_queue(PAYLOAD_ID,queue,RESPONSE_INTERFACE_VERSION);
		if (packet)
		{
			dbg_func_f(SERIAL_RD,g_strdup(__FILE__": comms_test()\n\tFound via probing!!\n"));
			freeems_packet_cleanup(packet);
			g_async_queue_unref(queue);
			DATA_SET(global_data,"connected",GINT_TO_POINTER(TRUE));
			return TRUE; 
		}
	}
	DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));
	dbg_func_f(SERIAL_RD,g_strdup(__FILE__": comms_test()\n\tNo device found...\n"));
	return FALSE;
}


void *win32_reader(gpointer data)
{
	gint fd = (gint)data;
	gint errcount = 0;
	static gsize wanted = 2048;
	gboolean res = FALSE;
	gchar buf[2048];
	gchar *ptr = NULL;
	gsize requested = 2048;
	gsize received = 0;
	gsize read_pos = 0;
	GIOStatus status;
	GError *err = NULL;
	GCond *cond = NULL;

	cond = DATA_GET(global_data,"serial_reader_cond");
	while (TRUE)
	{
		if ((DATA_GET(global_data,"leaving")) || (DATA_GET(global_data,"serial_abort")))
		{
			g_cond_signal(cond);
			g_thread_exit(0);
		}
		read_pos = requested-wanted;
		received = read(fd, &buf[read_pos], wanted);
		g_usleep(10000);
		/*printf("Want %i, got %i,",wanted, received); */
		if (received == -1)
		{
			DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));
			g_cond_signal(cond);
			g_thread_exit(0);
		}

		wanted -= received;
		/*	printf("Still need %i\n",wanted); */
		if (wanted <= 0)
			wanted = 2048;
		/*printf("WIN32 read %i bytes\n",received);*/
		if (received > 0)
			handle_data((guchar *)buf+read_pos,received);
		g_cond_signal(cond);
	}
}


void *unix_reader(gpointer data)
{
	gint fd = (gint)data;
	gint errcount = 0;
	static gsize wanted = 2048;
	gboolean res = FALSE;
	gchar buf[2048];
	gchar *ptr = NULL;
	gsize requested = 2048;
	gsize received = 0;
	gsize read_pos = 0;
	GIOStatus status;
	GError *err = NULL;
	GCond *cond = NULL;
	fd_set readfds;
	struct timeval t;


	cond = DATA_GET(global_data,"serial_reader_cond");
	FD_ZERO(&readfds);
	while (TRUE)
	{
		if ((DATA_GET(global_data,"leaving")) || (DATA_GET(global_data,"serial_abort")))
		{
			g_cond_signal(cond);
			g_thread_exit(0);
		}
		t.tv_sec = 0;
		t.tv_usec = 250000;
		FD_SET(fd,&readfds);
		res = select(fd+1,&readfds, NULL, NULL, &t);
		if (res == -1)
		{
			DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));
			g_cond_signal(cond);
			g_thread_exit(0);
		}
		if (res == 0) /* Timeout */
		{
			g_cond_signal(cond);
			continue;
		}
		if (FD_ISSET(fd,&readfds))
		{
			read_pos = requested-wanted;
			received = read(fd, &buf[read_pos], wanted);
			/*printf("Want %i, got %i,",wanted, received); */
			if (received == -1)
			{
				DATA_SET(global_data,"connected",GINT_TO_POINTER(FALSE));
				g_cond_signal(cond);
				g_thread_exit(0);
			}

			wanted -= received;
			/*	printf("Still need %i\n",wanted); */
			if (wanted <= 0)
				wanted = 2048;
			/*printf("UNIX read %i bytes\n",received);*/
			if (received > 0)
				handle_data((guchar *)buf+read_pos,received);
			g_cond_signal(cond);
		}
	}
}
