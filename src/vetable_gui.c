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

#include <3d_vetable.h>
#include <config.h>
#include <defines.h>
#include <debugging.h>
#include <enums.h>
#include <gui_handlers.h>
#include <structures.h>
#include <vetable_gui.h>

struct DynamicMisc misc;
extern struct DynamicButtons buttons;
extern struct DynamicLabels labels;
extern GtkWidget *ve_widgets[];

void build_vetable_1(GtkWidget *parent_frame)
{
	gint x,y;
	gint index;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *basetable;
	GtkWidget *table;
	GtkWidget *spinner;
	GtkWidget *frame;
	GtkWidget *button;
	GtkAdjustment *adj = NULL;
	extern GList *store_controls;
	extern GtkTooltips *tip;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);


	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),vbox2,TRUE,TRUE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2),0);

	label = gtk_label_new("VE Table 1 (All MS Variants)");
	gtk_box_pack_start(GTK_BOX(vbox2),label,FALSE,TRUE,0);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,FALSE,0);

	basetable = gtk_table_new(2,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(basetable),5);
	gtk_table_set_row_spacings(GTK_TABLE(basetable),0);
	gtk_box_pack_start(GTK_BOX(hbox),basetable,FALSE,FALSE,0);


	frame = gtk_frame_new(NULL);
	misc.p0_map_tps_frame = frame;
	label = gtk_label_new("MAP Bins");
	gtk_frame_set_label_widget(GTK_FRAME(frame),label);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (basetable), frame, 0, 1, 0, 1,
			(GtkAttachOptions) (0),
			(GtkAttachOptions) (0), 0, 0);

	table = gtk_table_new(9,1,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),2);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	/* KPA/TPS spinbuttons */
	label = gtk_label_new("Kpa");
	labels.p0_map_tps_lab = label;
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	index = 0;
	for (y=0;y < 8;y++)
	{
		adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,255,1,10,0);
		spinner = gtk_spin_button_new(adj,1,0);
		ve_widgets[VE1_KPA_BINS_OFFSET+index] = spinner;
		gtk_widget_set_size_request(spinner,45,-1);
		g_object_set_data(G_OBJECT(spinner),"offset", 
				GINT_TO_POINTER(VE1_KPA_BINS_OFFSET+index));
		g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
				GINT_TO_POINTER(1*100));
		g_object_set_data(G_OBJECT(spinner),"conv_type",
				GINT_TO_POINTER(CONV_NOTHING));
		g_object_set_data(G_OBJECT(spinner),"dl_type",
				GINT_TO_POINTER(IMMEDIATE));
		g_signal_connect (G_OBJECT(spinner), "value_changed",
				G_CALLBACK (spin_button_handler),
				GINT_TO_POINTER(GENERIC));
		/* Bind data to object for handlers */
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
		gtk_table_attach (GTK_TABLE (table), spinner, 
				0, 1, (7 - y) + 1, (7 - y) + 2,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);
		index++;

	}

	frame = gtk_frame_new("Volumetric Efficiency (%)");
	gtk_table_attach (GTK_TABLE (basetable), frame, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	table = gtk_table_new(9,8,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),2);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	/* VeTable spinbuttons */
	label = gtk_label_new(" ");
	gtk_table_attach (GTK_TABLE (table), label, 0, 8, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	index = 0;
	for (y=0;y<8;y++)
	{
		for (x=0;x<8;x++)
		{
			adj =  (GtkAdjustment *) gtk_adjustment_new(
					0.0,0.0,255,1,10,0);
			spinner = gtk_spin_button_new(adj,1,0);
			gtk_widget_add_events(spinner,
					GDK_BUTTON_PRESS_MASK);
			ve_widgets[VE1_TABLE_OFFSET+index] = spinner;
			g_object_set_data(G_OBJECT(spinner),"offset", 
					GINT_TO_POINTER(VE1_TABLE_OFFSET+index));
			g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
					GINT_TO_POINTER(1*100));
			g_object_set_data(G_OBJECT(spinner),"conv_type",
					GINT_TO_POINTER(CONV_NOTHING));
			g_object_set_data(G_OBJECT(spinner),"dl_type",
					GINT_TO_POINTER(IMMEDIATE));
			g_signal_connect (G_OBJECT(spinner), "button_press_event",
					G_CALLBACK (spin_button_grab),
					NULL);
			g_signal_connect (G_OBJECT(spinner), "value_changed",
					G_CALLBACK (spin_button_handler),
					GINT_TO_POINTER(GENERIC));
			/* Bind data to object for handlers */
			gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), 
					FALSE);
			gtk_table_attach (GTK_TABLE (table), spinner, 
					x, x+1, (7 - y) + 1, (7 - y) + 2,
					(GtkAttachOptions) (GTK_EXPAND),
					(GtkAttachOptions) (0), 0, 0);
			index++;
		}
	}


	/* RPM Table */
	frame = gtk_frame_new("RPM Bins");
	gtk_table_attach (GTK_TABLE (basetable), frame, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	table = gtk_table_new(1,8,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),1);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	for(x=0;x<8;x++)
	{
		adj =  (GtkAdjustment *) gtk_adjustment_new(100.0,100.0,25500,100,100,0);
		spinner = gtk_spin_button_new(adj,1,0);
		ve_widgets[VE1_RPM_BINS_OFFSET+x] = spinner;
		gtk_widget_set_size_request(spinner,54,-1);
		g_object_set_data(G_OBJECT(spinner),"offset", 
				GINT_TO_POINTER(VE1_RPM_BINS_OFFSET+x));
		g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
				GINT_TO_POINTER(100*100));
		g_object_set_data(G_OBJECT(spinner),"conv_type",
				GINT_TO_POINTER(CONV_DIV));
		g_object_set_data(G_OBJECT(spinner),"dl_type",
				GINT_TO_POINTER(IMMEDIATE));
		g_signal_connect (G_OBJECT(spinner), "value_changed",
				G_CALLBACK (spin_button_handler),
				GINT_TO_POINTER(GENERIC));
		/* Bind data to object for handlers */
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
		gtk_table_attach (GTK_TABLE (table), spinner, x, x+1, 0, 1,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);

	}

	button = gtk_button_new_with_label("3D View");
	g_signal_connect (G_OBJECT(button), "clicked",
			G_CALLBACK (create_3d_view),
			GINT_TO_POINTER(1));
	gtk_table_attach (GTK_TABLE (basetable), button, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);


	frame = gtk_frame_new("Commands");
	gtk_box_pack_end(GTK_BOX(vbox),frame,FALSE,TRUE,0);

	table = gtk_table_new(1,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	button = gtk_button_new_with_label("Get Data from ECU");
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			GINT_TO_POINTER(READ_VE_CONST));

	button = gtk_button_new_with_label("Permanently Store Data in ECU");
	store_controls = g_list_append(store_controls,(gpointer)button);
	gtk_tooltips_set_tip(tip,button,
			"Even though MegaTunix writes data to the MS as soon as its changed it, has only written it to the MegaSquirt's RAM, thus you need to select this to burn all variables to flash so on next power up things are as you set them.  We don't want to burn to flash with every variable change as there is the possibility of exceeding the max number of write cycles to the flash memory.", NULL);
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			GINT_TO_POINTER(BURN_MS_FLASH));

	return;
}

void build_vetable_2(GtkWidget *parent_frame)
{
	gint x,y;
	gint index;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *basetable;
	GtkWidget *table;
	GtkWidget *spinner;
	GtkWidget *frame;
	GtkWidget *button;
	GtkAdjustment *adj;
	extern GList *store_controls;
	extern GtkTooltips *tip;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
	gtk_container_add(GTK_CONTAINER(parent_frame),vbox);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),vbox2,TRUE,TRUE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2),0);

	label = gtk_label_new("VE Table 2 (DualTable Code Variantss)");
	gtk_box_pack_start(GTK_BOX(vbox2),label,FALSE,TRUE,0);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,FALSE,0);

	basetable = gtk_table_new(2,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(basetable),5);
	gtk_table_set_row_spacings(GTK_TABLE(basetable),0);
	gtk_box_pack_start(GTK_BOX(hbox),basetable,FALSE,FALSE,0);


	frame = gtk_frame_new(NULL);
	misc.p0_map_tps_frame = frame;
	label = gtk_label_new("MAP Bins");
	gtk_frame_set_label_widget(GTK_FRAME(frame),label);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (basetable), frame, 0, 1, 0, 1,
			(GtkAttachOptions) (0),
			(GtkAttachOptions) (0), 0, 0);

	table = gtk_table_new(9,1,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),2);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	/* KPA/TPS spinbuttons */
	label = gtk_label_new("Kpa");
	labels.p0_map_tps_lab = label;
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);
	index = 0;
	for (y=0;y < 8;y++)
	{
		adj =  (GtkAdjustment *) gtk_adjustment_new(1.0,1.0,255,1,10,0);
		spinner = gtk_spin_button_new(adj,1,0);
		ve_widgets[VE2_KPA_BINS_OFFSET+index] = spinner;
		gtk_widget_set_size_request(spinner,45,-1);
		g_object_set_data(G_OBJECT(spinner),"offset", 
				GINT_TO_POINTER(VE2_KPA_BINS_OFFSET+index));
		g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
				GINT_TO_POINTER(1*100));
		g_object_set_data(G_OBJECT(spinner),"conv_type",
				GINT_TO_POINTER(CONV_NOTHING));
		g_object_set_data(G_OBJECT(spinner),"dl_type",
				GINT_TO_POINTER(IMMEDIATE));
		g_signal_connect (G_OBJECT(spinner), "value_changed",
				G_CALLBACK (spin_button_handler),
				GINT_TO_POINTER(GENERIC));
		/* Bind data to object for handlers */
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
		gtk_table_attach (GTK_TABLE (table), spinner, 
				0, 1, (7 - y) + 1, (7 - y) + 2,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);
		index++;

	}

	frame = gtk_frame_new("Volumetric Efficiency (%)");
	gtk_table_attach (GTK_TABLE (basetable), frame, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	table = gtk_table_new(9,8,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),2);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	/* VeTable spinbuttons */
	label = gtk_label_new(" ");
	gtk_table_attach (GTK_TABLE (table), label, 0, 8, 0, 1,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	index = 0;
	for (y=0;y<8;y++)
	{
		for (x=0;x<8;x++)
		{
			adj =  (GtkAdjustment *) gtk_adjustment_new(
					0.0,0.0,255,1,10,0);
			spinner = gtk_spin_button_new(adj,1,0);
			gtk_widget_add_events(spinner,
					GDK_BUTTON_PRESS_MASK);
			ve_widgets[VE2_TABLE_OFFSET+index] = spinner;
			g_object_set_data(G_OBJECT(spinner),"offset", 
					GINT_TO_POINTER(VE2_TABLE_OFFSET+index));
			g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
					GINT_TO_POINTER(1*100));
			g_object_set_data(G_OBJECT(spinner),"conv_type",
					GINT_TO_POINTER(CONV_NOTHING));
			g_object_set_data(G_OBJECT(spinner),"dl_type",
					GINT_TO_POINTER(IMMEDIATE));
			g_signal_connect (G_OBJECT(spinner), "button_press_event",
					G_CALLBACK (spin_button_grab),
					NULL);
			g_signal_connect (G_OBJECT(spinner), "value_changed",
					G_CALLBACK (spin_button_handler),
					GINT_TO_POINTER(GENERIC));
			/* Bind data to object for handlers */
			gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), 
					FALSE);
			gtk_table_attach (GTK_TABLE (table), spinner, 
					x, x+1, (7 - y) + 1, (7 - y) + 2,
					(GtkAttachOptions) (GTK_EXPAND),
					(GtkAttachOptions) (0), 0, 0);
			index++;
		}
	}


	/* RPM Table */
	frame = gtk_frame_new("RPM Bins");
	gtk_table_attach (GTK_TABLE (basetable), frame, 1, 2, 1, 2,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (0), 0, 0);

	table = gtk_table_new(1,8,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),1);
	gtk_table_set_row_spacings(GTK_TABLE(table),2);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	for(x=0;x<8;x++)
	{
		adj =  (GtkAdjustment *) gtk_adjustment_new(100.0,100.0,25500,100,100,0);
		spinner = gtk_spin_button_new(adj,1,0);
		ve_widgets[VE2_RPM_BINS_OFFSET+x] = spinner;
		gtk_widget_set_size_request(spinner,54,-1);
		g_object_set_data(G_OBJECT(spinner),"offset", 
				GINT_TO_POINTER(VE2_RPM_BINS_OFFSET+x));
		g_object_set_data(G_OBJECT(spinner),"conv_factor_x100",
				GINT_TO_POINTER(100*100));
		g_object_set_data(G_OBJECT(spinner),"conv_type",
				GINT_TO_POINTER(CONV_DIV));
		g_object_set_data(G_OBJECT(spinner),"dl_type",
				GINT_TO_POINTER(IMMEDIATE));
		g_signal_connect (G_OBJECT(spinner), "value_changed",
				G_CALLBACK (spin_button_handler),
				GINT_TO_POINTER(GENERIC));
		/* Bind data to object for handlers */
		gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
		gtk_table_attach (GTK_TABLE (table), spinner, x, x+1, 0, 1,
				(GtkAttachOptions) (GTK_EXPAND),
				(GtkAttachOptions) (0), 0, 0);

	}

	button = gtk_button_new_with_label("3D View");
	g_signal_connect (G_OBJECT(button), "clicked",
			G_CALLBACK (create_3d_view),
			GINT_TO_POINTER(2));
	gtk_table_attach (GTK_TABLE (basetable), button, 0, 1, 1, 2,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);

	frame = gtk_frame_new("Commands");
	gtk_box_pack_end(GTK_BOX(vbox),frame,FALSE,TRUE,0);

	table = gtk_table_new(1,2,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),5);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_container_set_border_width (GTK_CONTAINER (table), 5);
	gtk_container_add(GTK_CONTAINER(frame),table);

	button = gtk_button_new_with_label("Get Data from ECU");
	gtk_table_attach (GTK_TABLE (table), button, 0, 1, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			GINT_TO_POINTER(READ_VE_CONST));

	button = gtk_button_new_with_label("Permanently Store Data in ECU");
	store_controls = g_list_append(store_controls,(gpointer)button);
	gtk_tooltips_set_tip(tip,button,
			"Even though MegaTunix writes data to the MS as soon as its changed it, has only written it to the MegaSquirt's RAM, thus you need to select this to burn all variables to flash so on next power up things are as you set them.  We don't want to burn to flash with every variable change as there is the possibility of exceeding the max number of write cycles to the flash memory.", NULL);
	gtk_table_attach (GTK_TABLE (table), button, 1, 2, 0, 1,
			(GtkAttachOptions) (GTK_EXPAND),
			(GtkAttachOptions) (0), 0, 0);
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(std_button_handler),
			GINT_TO_POINTER(BURN_MS_FLASH));

	return;
}

void hilite_ve_entries(gint rpm, gint map, gint table)
{
	/* Highlights the VEtable entries around the current engine
	 * Operating point to assist with tuning the table.
	 */
	extern unsigned char *ms_data;
	extern GdkColor red;
	extern GdkColor white;
	struct Ve_Const_Std *ve_const = NULL;
	static struct Indexes kpa_index[2],rpm_index[2];
	static struct Indexes l_kpa_index[2],l_rpm_index[2];
	static gint index[2][4] = {{-1,-1,-1,-1},{-1,-1,-1,-1}};
	gint offset = 0;
	gint i = 0;


	ve_const = (struct Ve_Const_Std *) ms_data;
	if (table == 2)
		offset = MS_PAGE_SIZE;

	get_indexes(KPA,map, &kpa_index[table],table);
	get_indexes(RPM,rpm/100, &rpm_index[table],table);

	/* If nothing has changed, exit now saving CPU time */
	if ((l_kpa_index[table].low == kpa_index[table].low) &&
			(l_kpa_index[table].high == kpa_index[table].high) &&
			(l_rpm_index[table].low == rpm_index[table].low) &&
			(l_rpm_index[table].high == rpm_index[table].high))
		return;

	for (i=0;i<4;i++)
	{
		if (index[table][i] >= 0)
		{
			gtk_widget_modify_base(ve_widgets[offset+index[table][i]],
					GTK_STATE_NORMAL,&white);
		}
	}
	dbg_func(g_strdup_printf(__FILE__": hilite_ve_entries() rpm %i,%i, kpa %i,%i\n",rpm_index[table].low,rpm_index[table].high,kpa_index[table].low,kpa_index[table].high),IO_PROCESS);

	index[table][0] = (kpa_index[table].low * 8) + rpm_index[table].low;
	index[table][1] = (kpa_index[table].low * 8) + rpm_index[table].high;
	index[table][2] = (kpa_index[table].high * 8) + rpm_index[table].low;
	index[table][3] = (kpa_index[table].high * 8) + rpm_index[table].high;

	for (i=0;i<4;i++)
	{
		gtk_widget_modify_base(ve_widgets[offset+index[table][i]],
				GTK_STATE_NORMAL,&red);
	}

	/* save last run */
	l_kpa_index[table].low = kpa_index[table].low;
	l_kpa_index[table].high = kpa_index[table].high;
	l_rpm_index[table].low = rpm_index[table].low;
	l_rpm_index[table].high = rpm_index[table].high;

	return;

}

void get_indexes(TableType type, gint value, void *ptr,gint table)
{
	extern unsigned char *ms_data;
	struct Indexes *index = (struct Indexes *) ptr;
	gint start = -1;
	const gint span = 8;
	gint i = -1;

	if (type == KPA)
		start = VE1_KPA_BINS_OFFSET;
	else if (type == RPM)
		start = VE1_RPM_BINS_OFFSET;
	else
	{
		dbg_func(__FILE__": get_indexes(), Invalid TableType passed to get_indexes()\n",CRITICAL);
		index->low = -1;
		index->high = -1;
		return;
	}
	if (table == 2)
		start += MS_PAGE_SIZE;
	
	for (i=0;i<span-1;i++)
	{
		/* If value is out of table bounds on low side */
		if (value < ms_data[start])
		{
			index->low=0;
			index->high=0;
			break;
		}
		/* Somewhere inside table.... */
		if ((value >= ms_data[start+i])&&(value <= ms_data[start+i+1]))
		{	
			index->low=i;
			index->high=i+1;
			break;
		}
		/* Above table bounds */
		index->low = span-1;
		index->high = span-1;
	}

	return;
}
