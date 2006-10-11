/* Christopher Mire, 2006
 *
 * -------------------------------------------------------------------------
 *  Hacked and slashed to hell by David J. Andruczyk in order to bend and
 *  tweak it to my needs for MegaTunix.  Added in rendering ability using
 *  cairo and raw GDK callls for those less fortunate (OS-X)
 *
 *  Was offered a fine contribution by Ari Karhu 
 *  "ari <at> ultimatevw <dot> com" from the msefi.com forums.
 *  His contribution made the gauges look, ohh so much nicer than I could 
 *  have come up with!
 */



#include <config.h>
#ifdef HAVE_CAIRO
#include <cairo/cairo.h>
#endif
#include <gauge-private.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <glib-object.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <string.h>


void mtx_gauge_face_set_value_internal (MtxGaugeFace *gauge, float value)
{
	gauge->value = value;
	mtx_gauge_face_redraw_canvas (gauge);//show new value
}

void mtx_gauge_face_set_units_str_internal (MtxGaugeFace *gauge, gchar * units_str)
{
	if (gauge->units_str)
		g_free(gauge->units_str);
	gauge->units_str = g_strdup(units_str);;
	
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);//show new value
}


void mtx_gauge_face_set_name_str_internal (MtxGaugeFace *gauge, gchar * name_str)
{
	if (gauge->name_str)
		g_free(gauge->name_str);
	gauge->name_str = g_strdup(name_str);;
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);//show new value
}


/* Changes value stored in widget, and gets widget redrawn to show change */
void mtx_gauge_face_set_value (MtxGaugeFace *gauge, float value)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	mtx_gauge_face_set_value_internal (gauge, value);
	g_object_thaw_notify (G_OBJECT (gauge));
}

/* Changes widget precision */
void mtx_gauge_face_set_precision (MtxGaugeFace *gauge, gint precision)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->precision = precision;
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);//show new value
}

/* Sets antialias mode */
void mtx_gauge_face_set_antialias(MtxGaugeFace *gauge, gboolean state)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	gauge->antialias = state;
}

/* Changes value stored in widget, and gets widget redrawn to show change */
void mtx_gauge_face_set_units_str (MtxGaugeFace *gauge, gchar * units_str)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	mtx_gauge_face_set_units_str_internal (gauge, units_str);
	g_object_thaw_notify (G_OBJECT (gauge));
}

/* Changes value stored in widget, and gets widget redrawn to show change */
void mtx_gauge_face_set_name_str (MtxGaugeFace *gauge, gchar * name_str)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	mtx_gauge_face_set_name_str_internal (gauge, name_str);
	g_object_thaw_notify (G_OBJECT (gauge));
}


/* Returns value that needle currently points to */
float mtx_gauge_face_get_value (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->value;
}

/* Returns numerical precision */
gint mtx_gauge_face_get_precision (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->precision;
}

/* Returns antialias status */
gboolean mtx_gauge_face_get_antialias (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), FALSE);
	return gauge->antialias;
}

/* Returns the units string */
gchar * mtx_gauge_face_get_units_str (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), NULL);
	return gauge->units_str;
}

/* Returns the name string */
gchar * mtx_gauge_face_get_name_str (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), NULL);
	return gauge->name_str;
}


/* Sets a range color span dynamically */
void mtx_gauge_face_set_color_range(MtxGaugeFace *gauge, gfloat lower, gfloat upper, GdkColor color)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));

	g_object_freeze_notify (G_OBJECT (gauge));
	MtxColorRange * range = g_new0(MtxColorRange, 1);
	range->lowpoint = lower;
	range->highpoint = upper;
	range->color = color;
	g_array_append_val(gauge->ranges,range);
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
}


/* Changes the lower and upper value bounds */
void mtx_gauge_face_set_bounds (MtxGaugeFace *gauge, float value1, float value2)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->lbound = value1;
	gauge->ubound = value2;
	gauge->span = gauge->ubound -gauge->lbound;
	g_object_thaw_notify (G_OBJECT (gauge));
}

/* returns the lower and upper value bounds */
gboolean mtx_gauge_face_get_bounds(MtxGaugeFace *gauge, float *value1, float *value2)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge),FALSE);
	*value1 = gauge->lbound;
	*value2 = gauge->ubound;
	return TRUE;
}
/* Set number of ticks to be shown in the span of the drawn gauge */
void mtx_gauge_face_set_major_ticks (MtxGaugeFace *gauge, int ticks)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->major_ticks = ticks;
	generate_gauge_background(GTK_WIDGET(gauge));
	g_object_thaw_notify (G_OBJECT (gauge));
}

/* Returns current number of ticks drawn in span of gauge */
int mtx_gauge_face_get_major_ticks (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->major_ticks;
}

/* Set number of ticks to be shown in the span of the drawn gauge */
void mtx_gauge_face_set_minor_ticks (MtxGaugeFace *gauge, int ticks)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->minor_ticks = ticks;
	generate_gauge_background(GTK_WIDGET(gauge));
	g_object_thaw_notify (G_OBJECT (gauge));
}

/* Returns current number of ticks drawn in span of gauge */
int mtx_gauge_face_get_minor_ticks (MtxGaugeFace *gauge)
{
	g_return_val_if_fail (MTX_IS_GAUGE_FACE (gauge), -1);
	return gauge->minor_ticks;
}

/* Changes the span of the gauge, specified in radians the start and stop position. */
/* Right is 0 going clockwise to M_PI (180 Degrees) back to 0 (2 * M_PI) */
void mtx_gauge_face_set_span (MtxGaugeFace *gauge, float start_radian, float stop_radian)
{
	g_return_if_fail (MTX_IS_GAUGE_FACE (gauge));
	g_object_freeze_notify (G_OBJECT (gauge));
	gauge->start_radian = start_radian;
	gauge->stop_radian = stop_radian;
	/* For some un know ndamn reason,  GDK draws it's arcs
	 * counterclockwisein units of 1/64th fo a degree, whereas cairo
	 * doe it clockwise in radians. At least they start at the same place
	 * "3 O'clock" 
	 * */
#ifndef HAVE_CAIRO
	gauge->start_deg = -((start_radian/M_PI) *180.0);
	gauge->stop_deg = -(((stop_radian-start_radian)/M_PI) *180.0);
#endif
	g_object_thaw_notify (G_OBJECT (gauge));
	generate_gauge_background(GTK_WIDGET(gauge));
	mtx_gauge_face_redraw_canvas (gauge);//show new value
}

void mtx_gauge_face_class_init (MtxGaugeFaceClass *class_name)
{
	GObjectClass *obj_class;
	GtkWidgetClass *widget_class;

	obj_class = G_OBJECT_CLASS (class_name);
	widget_class = GTK_WIDGET_CLASS (class_name);

	/* GtkWidget signals */
	widget_class->configure_event = mtx_gauge_face_configure;
	widget_class->expose_event = mtx_gauge_face_expose;
	widget_class->button_press_event = mtx_gauge_face_button_press;
	widget_class->button_release_event = mtx_gauge_face_button_release;

	g_type_class_add_private (obj_class, sizeof (MtxGaugeFacePrivate));
}

void mtx_gauge_face_init (MtxGaugeFace *gauge)
{
	//which events widget receives
	gtk_widget_add_events (GTK_WIDGET (gauge),GDK_BUTTON_PRESS_MASK
			       | GDK_BUTTON_RELEASE_MASK);

	gauge->w = 0;
	gauge->h = 0;
	gauge->xc = 0.0;
	gauge->yc = 0.0;
	gauge->radius = 0.0;
	gauge->value = 0.0;//default values
	gauge->lbound = -1.0;
	gauge->ubound = 1.0;
	gauge->precision = 2;
	gauge->start_radian = 0.75 * M_PI;//M_PI is left, 0 is right
	gauge->stop_radian = 2.25 * M_PI;
	gauge->major_ticks = 9;
	gauge->minor_ticks = 0;  /* B S S S B S S S B  tick style */
	gauge->tick_inset = 0.15;    /* how much in from gauge radius fortick */
	gauge->major_tick_len = 0.1; /* 1 = 100% of radius, so 0.1 = 10% */
	gauge->minor_tick_len = 0.05;/* 1 = 100% of radius, so 0.1 = 10% */
	gauge->needle_width = 0.05;  /* % of radius */
	gauge->needle_tail = 0.083;  /* % of radius */
	gauge->antialias = FALSE;
	gauge->name_font = g_strdup("Bitstream Vera Sans");
	gauge->name_str = NULL;
	gauge->name_font_scale = 0.15;
	gauge->units_font = g_strdup("Bitstream Vera Sans");
	gauge->units_str = NULL;
	gauge->units_font_scale = 0.1;
	gauge->value_font = g_strdup("Bitstream Vera Sans");
	gauge->value_str = g_strdup("000");
	gauge->value_font_scale = 0.2;
	gauge->span = gauge->ubound - gauge->lbound;
#ifdef HAVE_CAIRO
	gauge->cr = NULL;
#endif
	gauge->start_deg = 225;
	gauge->stop_deg = -270;
	gauge->colormap = gdk_colormap_get_system();
	gauge->axis_gc = NULL;	
	gauge->needle_gc = NULL;
	gauge->font_gc = NULL;
	gauge->ranges = g_array_new(FALSE,TRUE,sizeof(MtxColorRange *));
	mtx_gauge_face_redraw_canvas (gauge);
}


void update_gauge_position(GtkWidget *widget)
{
#ifdef HAVE_CAIRO
	cairo_update_gauge_position (widget);
#else
	gdk_update_gauge_position (widget);
#endif
}


void cairo_update_gauge_position (GtkWidget *widget)
{
#ifdef HAVE_CAIRO
	gfloat radian_span = 0.0;
	gfloat tmpf = 0.0;
	gfloat needle_pos = 0.0;
	gchar * message = NULL;
	gint i = 0;
	gfloat n_width = 0.0;
	gfloat n_tail = 0.0;
	gfloat n_tip = 0.0;
	gfloat xc = 0.0;
	gfloat yc = 0.0;
	cairo_t *cr = NULL;
	cairo_text_extents_t extents;

	MtxGaugeFace * gauge = (MtxGaugeFace *)widget;

	/* Copy bacground pixmap to intermediaary for final rendering */
	gdk_draw_drawable(gauge->pixmap,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			gauge->bg_pixmap,
			0,0,
			0,0,
			widget->allocation.width,widget->allocation.height);


	cr = gdk_cairo_create (gauge->pixmap);
	/* gauge hands */
	radian_span = (gauge->stop_radian - gauge->start_radian);
	tmpf = (gauge->value-gauge->lbound)/(gauge->ubound-gauge->lbound);
	needle_pos = gauge->start_radian+(tmpf*radian_span)+M_PI/2.0;

	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_set_line_width (cr, 1);

	n_width = gauge->needle_width * gauge->radius;
	n_tail = gauge->needle_tail * gauge->radius;
	n_tip = gauge->radius * 0.850;
	xc = gauge->xc;
	yc = gauge->yc;

	/* STORE needle coordinates to make the expese event a LOT more 
	 * efficient */
	for (i=0;i<4;i++)
	{
		gauge->last_needle_coords[i].x = gauge->needle_coords[i].x;
		gauge->last_needle_coords[i].y = gauge->needle_coords[i].y;
	}
	gauge->needle_coords[0].x = xc + (n_tip) * sin (needle_pos);
	gauge->needle_coords[0].y = yc + (n_tip) * -cos (needle_pos);

	gauge->needle_coords[1].x = xc + (n_width) *cos(needle_pos);
	gauge->needle_coords[1].y = yc + (n_width) *sin(needle_pos);

	gauge->needle_coords[2].x = xc + (n_tail) * -sin (needle_pos);
	gauge->needle_coords[2].y = yc + (n_tail) * cos (needle_pos);
	gauge->needle_coords[3].x = xc - (n_width) * cos (needle_pos);
	gauge->needle_coords[3].y = yc - (n_width) * sin (needle_pos);
	gauge->needle_polygon_points = 4;

	cairo_move_to (cr, gauge->needle_coords[0].x,gauge->needle_coords[0].y);
	cairo_line_to (cr, gauge->needle_coords[1].x,gauge->needle_coords[1].y);
	cairo_line_to (cr, gauge->needle_coords[2].x,gauge->needle_coords[2].y);
	cairo_line_to (cr, gauge->needle_coords[3].x,gauge->needle_coords[3].y);
	cairo_fill_preserve (cr);

	/* Update the VALUE text */
	cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
	cairo_select_font_face (cr, gauge->value_font, CAIRO_FONT_SLANT_NORMAL,
			CAIRO_FONT_WEIGHT_NORMAL);

	cairo_set_font_size (cr, (gauge->radius * gauge->value_font_scale));

	message = g_strdup_printf("%.*f", gauge->precision,gauge->value);

	cairo_text_extents (cr, message, &extents);

	cairo_move_to (cr, gauge->xc-(extents.width/2), gauge->yc+(0.55 * gauge->radius));
	cairo_show_text (cr, message);
	g_free(message);

	cairo_stroke (cr);

	cairo_destroy(cr);
#endif
}

void gdk_update_gauge_position (GtkWidget *widget)
{
	gint i= 0;
	gfloat xc = 0.0;
	gfloat yc = 0.0;
	gfloat tmpf = 0.0;
	gfloat radian_span = 0.0;
	gfloat needle_pos = 0.0;
	gint n_width = 0;
	gint n_tail = 0;
	gint n_tip = 0;
	gchar * message = NULL;
	gchar * tmpbuf = NULL;
	PangoRectangle logical_rect;

	MtxGaugeFace * gauge = (MtxGaugeFace *)widget;

	/* Copy bacground pixmap to intermediaary for final rendering */
	gdk_draw_drawable(gauge->pixmap,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			gauge->bg_pixmap,
			0,0,
			0,0,
			widget->allocation.width,widget->allocation.height);

	//	lwidth = MIN (w/2, h/2)/100 < 1 ? 1: MIN (w/2, h/2)/100;
	gdk_gc_set_line_attributes(gauge->axis_gc,1,
			GDK_LINE_SOLID,
			GDK_CAP_ROUND,
			GDK_JOIN_ROUND);

	/* gauge hands */
	radian_span = (gauge->stop_radian - gauge->start_radian);
	tmpf = (gauge->value-gauge->lbound)/(gauge->ubound-gauge->lbound);
	needle_pos = gauge->start_radian+(tmpf*radian_span)+M_PI/2.0;
//	printf("start_rad %f, stop_rad %f\n",gauge->start_radian, gauge->stop_radian);
//	printf("tmpf %f, radian span %f, position, %f\n",tmpf,radian_span,needle_pos);

	xc= gauge->xc;
	yc= gauge->yc;
	n_width = gauge->needle_width * gauge->radius;
	n_tail = gauge->needle_tail * gauge->radius;
	n_tip = gauge->radius * 0.850;

	/* Four POINT needle,  point 0 is the tip (easiest to find) */
	for (i=0;i<4;i++)
	{
		gauge->last_needle_coords[i].x = gauge->needle_coords[i].x;
		gauge->last_needle_coords[i].y = gauge->needle_coords[i].y;
	}
	gauge->needle_coords[0].x = xc + (n_tip) * sin (needle_pos);
	gauge->needle_coords[0].y = yc + (n_tip) * -cos (needle_pos);

	gauge->needle_coords[1].x = xc + (n_width) *cos(needle_pos);
	gauge->needle_coords[1].y = yc + (n_width) *sin(needle_pos);

	gauge->needle_coords[2].x = xc + (n_tail) * -sin (needle_pos);
	gauge->needle_coords[2].y = yc + (n_tail) * cos (needle_pos);
	gauge->needle_coords[3].x = xc - (n_width) * cos (needle_pos);
	gauge->needle_coords[3].y = yc - (n_width) * sin (needle_pos);
	gauge->needle_polygon_points = 4;

	/* Draw the needle */
	gdk_draw_polygon(gauge->pixmap,
			widget->style->white_gc,
			TRUE,gauge->needle_coords,4);

	message = g_strdup_printf("%.*f", gauge->precision,gauge->value);

	tmpbuf = g_strdup_printf("%s %i",gauge->value_font,(gint)(gauge->radius *gauge->value_font_scale));
	gauge->font_desc = pango_font_description_from_string(tmpbuf);
	g_free(tmpbuf);
	pango_layout_set_font_description(gauge->layout,gauge->font_desc);
	pango_layout_set_text(gauge->layout,message,-1);
	pango_layout_get_pixel_extents(gauge->layout,NULL,&logical_rect);
	g_free(message);


	gdk_draw_layout(gauge->pixmap,gauge->font_gc,
			gauge->xc-(logical_rect.width/2),
			gauge->yc+(0.55 * gauge->radius)-logical_rect.height,gauge->layout);

}

gboolean mtx_gauge_face_configure (GtkWidget *widget, GdkEventConfigure *event)
{
	GdkColor color;
	MtxGaugeFace * gauge = MTX_GAUGE_FACE(widget);

	if(widget->window)
	{
		gauge->w = widget->allocation.width;
		gauge->h = widget->allocation.height;

		if (gauge->axis_gc)
			g_object_unref(gauge->axis_gc);
		if (gauge->needle_gc)
			g_object_unref(gauge->needle_gc);
		if (gauge->font_gc)
			g_object_unref(gauge->font_gc);
		if (gauge->layout)
			g_object_unref(gauge->layout);
		/* Backing pixmap (copy of window) */
		if (gauge->pixmap)
			g_object_unref(gauge->pixmap);
		gauge->pixmap=gdk_pixmap_new(widget->window,
				gauge->w,gauge->h,
				gtk_widget_get_visual(widget)->depth);
		gdk_draw_rectangle(gauge->pixmap,
				widget->style->black_gc,
				TRUE, 0,0,
				gauge->w,gauge->h);
		/* Static Background pixmap */
		if (gauge->bg_pixmap)
			g_object_unref(gauge->bg_pixmap);
		gauge->bg_pixmap=gdk_pixmap_new(widget->window,
				gauge->w,gauge->h,
				gtk_widget_get_visual(widget)->depth);
		gdk_draw_rectangle(gauge->bg_pixmap,
				widget->style->black_gc,
				TRUE, 0,0,
				gauge->w,gauge->h);

		gdk_window_set_back_pixmap(widget->window,gauge->pixmap,0);
		gauge->layout = gtk_widget_create_pango_layout(GTK_WIDGET(&gauge->parent),NULL);	
		gauge->axis_gc = gdk_gc_new(gauge->bg_pixmap);
		gauge->needle_gc = gdk_gc_new(gauge->pixmap);
		gauge->font_gc = gdk_gc_new(gauge->pixmap);
		gdk_gc_set_colormap(gauge->axis_gc,gauge->colormap);
		gdk_gc_set_colormap(gauge->needle_gc,gauge->colormap);
		gdk_gc_set_colormap(gauge->font_gc,gauge->colormap);

		color.red=0.8 * 65536;
		color.green=0.8 * 65536;
		color.blue=0.8 * 65536;
		gdk_gc_set_rgb_fg_color(gauge->font_gc,&color);

		gauge->xc = gauge->w / 2;
		gauge->yc = gauge->h / 2;
		gauge->radius = MIN (gauge->w/2, gauge->h/2) - 5;
	}
	generate_gauge_background(widget);
	update_gauge_position(widget);

	return TRUE;
}


gboolean mtx_gauge_face_expose (GtkWidget *widget, GdkEventExpose *event)
{
	MtxGaugeFace * gauge = MTX_GAUGE_FACE(widget);

	gdk_draw_drawable(widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			gauge->pixmap,
			event->area.x, event->area.y,
			event->area.x, event->area.y,
			event->area.width, event->area.height);

	return FALSE;
}


void generate_gauge_background(GtkWidget *widget)
{
#ifdef HAVE_CAIRO
	cairo_generate_gauge_background(widget);
#else
	gdk_generate_gauge_background(widget);
#endif
}

void cairo_generate_gauge_background(GtkWidget *widget)
{
#ifdef HAVE_CAIRO
	cairo_t *cr = NULL;
	gfloat arc = 0.0;
	gfloat radians_per_major_tick = 0.0;
	gfloat radians_per_minor_tick = 0.0;
	gint w = 0;
	gint h = 0;
	gint i = 0;
	gint j = 0;
	gfloat counter = 0;
	gfloat subcounter = 0;
	gint inset = 0;
	gint insetfrom = 0;
	gfloat lwidth = 0.0;
	gfloat angle1, angle2;
	cairo_pattern_t *gradient = NULL;
	cairo_text_extents_t extents;
	MtxColorRange *range = NULL;

	MtxGaugeFace * gauge = (MtxGaugeFace *)widget;

	w = widget->allocation.width;
	h = widget->allocation.height;
	/* get a cairo_t */
	cr = gdk_cairo_create (gauge->bg_pixmap);
	/* Background set to black */
	cairo_rectangle (cr,
			0,0,
			widget->allocation.width, widget->allocation.height);
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_fill(cr);
	if (gauge->antialias)
		cairo_set_antialias(cr,CAIRO_ANTIALIAS_DEFAULT);
	else
		cairo_set_antialias(cr,CAIRO_ANTIALIAS_NONE);

	/* Create that cool gradient that gives the shadow illusion */
	gradient = cairo_pattern_create_linear(0,gauge->w,gauge->w,0);
	cairo_pattern_add_color_stop_rgb(gradient, 0, 1, 1, 1);
	cairo_pattern_add_color_stop_rgb(gradient, gauge->radius, 0, 0, 0);

	/* Filled Arcs */
	cairo_arc(cr, gauge->xc, gauge->yc, (0.985 * gauge->radius), 0, 2*M_PI);
	cairo_set_source_rgb(cr, 0.53, 0.53, 0.53);
	cairo_fill(cr);

	cairo_arc(cr, gauge->xc, gauge->yc, (0.985 * gauge->radius), 0, 2 * M_PI);
	cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
	cairo_stroke(cr);

	cairo_arc(cr, gauge->xc, gauge->yc, (0.934 * gauge->radius), 0, 2 * M_PI);
	cairo_set_source(cr, gradient);
	cairo_fill(cr);
		    	
	cairo_arc(cr, gauge->xc, gauge->yc, (0.880 * gauge->radius), 0, 2 * M_PI);
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_fill(cr);

	/* The warning color ranges */
	arc = (gauge->stop_radian - gauge->start_radian) / (2 * M_PI);
	for (i=0;i<gauge->ranges->len;i++)
	{
		range = g_array_index(gauge->ranges,MtxColorRange *, i);
		cairo_set_source_rgb(cr,range->color.red/65536.0,
				range->color.green/65536.0,
				range->color.blue/65536.0);
		/* percent of full scale is (lbound-range_lbound)/(fullspan)*/
		angle1 = (range->lowpoint-gauge->lbound)/(gauge->ubound-gauge->lbound);
		angle2= (range->highpoint-gauge->lbound)/(gauge->ubound-gauge->lbound);
		//printf("gauge color range should be from %f, to %f of full scale\n",angle1, angle2);
		lwidth = MIN (w/2, h/2)/20 < 1 ? 1: MIN (w/2, h/2)/20;
		cairo_set_line_width (cr, lwidth);
		cairo_arc(cr, gauge->xc, gauge->yc, (0.775 * gauge->radius),gauge->start_radian+(angle1*(gauge->stop_radian-gauge->start_radian)), gauge->start_radian+(angle2*(gauge->stop_radian-gauge->start_radian)));

		cairo_stroke(cr);

	}

	/* gauge ticks */
	cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
	radians_per_major_tick = (gauge->stop_radian - gauge->start_radian)/(float)(gauge->major_ticks-1);
	radians_per_minor_tick = radians_per_major_tick/(float)(1+gauge->minor_ticks);
	/* Major ticks first */
	insetfrom = gauge->radius * gauge->tick_inset;

	counter = gauge->start_radian;
	for (i=0;i<gauge->major_ticks;i++)
	{
		inset = (gint) (gauge->major_tick_len * gauge->radius);
		lwidth = MIN (w/2, h/2)/80 < 1 ? 1: MIN (w/2, h/2)/80;
		cairo_set_line_width (cr, lwidth);
		cairo_move_to (cr,
				gauge->xc + (gauge->radius - insetfrom) * cos (counter),
				gauge->yc + (gauge->radius - insetfrom) * sin (counter));
		cairo_line_to (cr,
				gauge->xc + (gauge->radius - insetfrom - inset) * cos (counter),
				gauge->yc + (gauge->radius - insetfrom - inset) * sin (counter));
		cairo_stroke (cr);
		/* minor ticks */
		if ((gauge->minor_ticks > 0) && (i < (gauge->major_ticks-1)))
		{
			//cairo_save (cr); /* stack-pen-size */
			inset = (gint) (gauge->minor_tick_len * gauge->radius);
			lwidth = MIN (w/2, h/2)/160 < 1 ? 1: MIN (w/2, h/2)/160;
			cairo_set_line_width (cr, lwidth);
			for (j=1;j<=gauge->minor_ticks;j++)
			{
				subcounter = j*radians_per_minor_tick;
				cairo_move_to (cr,
						gauge->xc + (gauge->radius - insetfrom) * cos (counter+subcounter),
						gauge->yc + (gauge->radius - insetfrom) * sin (counter+subcounter));
				cairo_line_to (cr,
						gauge->xc + (gauge->radius - insetfrom - inset) * cos (counter+subcounter),
						gauge->yc + (gauge->radius - insetfrom - inset) * sin (counter+subcounter));
				cairo_stroke (cr);
			}
			//cairo_restore (cr); /* stack-pen-size */
		}
		counter += radians_per_major_tick;
	}
	
	/* The units string */
	cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
	if (gauge->units_str)
	{
		cairo_select_font_face (cr, gauge->units_font, CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);

		cairo_set_font_size (cr, (gauge->radius * gauge->units_font_scale));
		cairo_text_extents (cr, gauge->units_str, &extents);
		cairo_move_to (cr, gauge->xc-(extents.width/2), gauge->yc+(0.75 * gauge->radius));
		cairo_show_text (cr, gauge->units_str);
	}

	/* The name string */
	if (gauge->name_str)
	{
		cairo_select_font_face (cr, gauge->name_font, CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);

		cairo_set_font_size (cr, (gauge->radius * gauge->name_font_scale));
		cairo_text_extents (cr, gauge->name_str, &extents);
		cairo_move_to (cr, gauge->xc-(extents.width/2), gauge->yc-(0.35 * gauge->radius));
		cairo_show_text (cr, gauge->name_str);
	}

	cairo_destroy (cr);
#endif
}


void gdk_generate_gauge_background(GtkWidget *widget)
{
	gfloat radians_per_major_tick = 0.0;
	gfloat radians_per_minor_tick = 0.0;
	gint w = 0;
	gint h = 0;
	gint i = 0;
	gint j = 0;
	gint lwidth = 0;
	gfloat arc = 0.0;
	gint inset = 0;
	gint insetfrom = 0;
	gfloat counter = 0.0;
	gfloat subcounter = 0.0;
	gchar * tmpbuf = NULL;
	gfloat angle1 = 0.0;
	gfloat angle2 = 0.0;
	gfloat start_pos = 0.0;
	gfloat stop_pos = 0.0;
	gfloat start_angle = 0.0;
	gfloat span = 0.0;
	MtxColorRange *range = NULL;
	GdkColor color;
	PangoRectangle logical_rect;

	MtxGaugeFace * gauge = (MtxGaugeFace *)widget;
	w = widget->allocation.width;
	h = widget->allocation.height;

	/* Wipe the display, black */
	gdk_draw_rectangle(gauge->bg_pixmap,
			widget->style->black_gc,
			TRUE, 0,0,
			w,h);

	/* The main grey (will have a thin overlay on top of it) 
	 * This is a FILLED circle */
	
	color.red=(gint)(0.53 * 65535);
	color.green=(gint)(0.53 * 65535);
	color.blue=(gint)(0.53 * 65535);
	gdk_gc_set_rgb_fg_color(gauge->axis_gc,&color);
	/* Create the border stuff */
	gdk_draw_arc(gauge->bg_pixmap,gauge->axis_gc,TRUE,
			gauge->xc-gauge->radius*0.985,
			gauge->yc-gauge->radius*0.985,
			2*(gauge->radius*0.985),
			2*(gauge->radius*0.985),
			0,360*64);

	/* Outermost thin "ring" NOTE!! not filled */
	lwidth = MIN (w/2, h/2)/120 < 1 ? 1: MIN (w/2, h/2)/120;
	gdk_gc_set_line_attributes(gauge->axis_gc,lwidth,
			GDK_LINE_SOLID,
			GDK_CAP_BUTT,
			GDK_JOIN_BEVEL);
	color.red=(gint)(0.8 * 65535);
	color.green=(gint)(0.8 * 65535);
	color.blue=(gint)(0.8 * 65535);
	gdk_gc_set_rgb_fg_color(gauge->axis_gc,&color);

	gdk_draw_arc(gauge->bg_pixmap,gauge->axis_gc,FALSE,
			gauge->xc-gauge->radius*0.985,
			gauge->yc-gauge->radius*0.985,
			2*(gauge->radius*0.985),
			2*(gauge->radius*0.985),
			0,360*64);

	lwidth = MIN (gauge->xc,gauge->yc)/20 < 1 ? 1: MIN (gauge->xc,gauge->yc)/20;
	gdk_gc_set_line_attributes(gauge->axis_gc,lwidth,
			GDK_LINE_SOLID,
			GDK_CAP_BUTT,
			GDK_JOIN_BEVEL);


	/* Funky hack to get a pretty gradient sorta like the cairo version*/
	for(i=0;i<36;i++)
	{
		color.red=(gint)(((i+6)/48.0) * 65535);
		color.green=(gint)(((i+6)/48.0) * 65535);
		color.blue=(gint)(((i+6)/48.0) * 65535);
		gdk_gc_set_rgb_fg_color(gauge->axis_gc,&color);

		gdk_draw_arc(gauge->bg_pixmap,gauge->axis_gc,FALSE,
				gauge->xc-gauge->radius*0.903,
				gauge->yc-gauge->radius*0.903,
				2*(gauge->radius*0.903),
				2*(gauge->radius*0.903),
				(45+(i*5))*64,5*64);
	}
	for(i=0;i<36;i++)
	{
		color.red=(gint)(((42-i)/48.0) * 65535);
		color.green=(gint)(((42-i)/48.0) * 65535);
		color.blue=(gint)(((42-i)/48.0) * 65535);
		gdk_gc_set_rgb_fg_color(gauge->axis_gc,&color);

		gdk_draw_arc(gauge->bg_pixmap,gauge->axis_gc,FALSE,
				gauge->xc-gauge->radius*0.903,
				gauge->yc-gauge->radius*0.903,
				2*(gauge->radius*0.903),
				2*(gauge->radius*0.903),
				(225+(i*5))*64,5*64);
	}
	


	/* Create the INNER filled black arc to draw the ticks and everything
	 * else onto
	 */
	gdk_gc_set_line_attributes(gauge->axis_gc,1,
			GDK_LINE_SOLID,
			GDK_CAP_BUTT,
			GDK_JOIN_BEVEL);
	gdk_draw_arc(gauge->bg_pixmap,widget->style->black_gc,TRUE,
			gauge->xc-gauge->radius*0.880,
			gauge->yc-gauge->radius*0.880,
			2*(gauge->radius*0.880),
			2*(gauge->radius*0.880),
			0,360*64);

	/* The warning color ranges */
	arc = (gauge->stop_radian - gauge->start_radian) / (2 * M_PI);
	for (i=0;i<gauge->ranges->len;i++)
	{
		range = g_array_index(gauge->ranges,MtxColorRange *, i);
		gdk_gc_set_rgb_fg_color(gauge->axis_gc,&range->color);
		/* percent of full scale is (lbound-range_lbound)/(fullspan)*/
		span = gauge->stop_radian - gauge->start_radian;
		angle1 = (range->lowpoint-gauge->lbound)/(gauge->span);
		angle2 = (range->highpoint-gauge->lbound)/(gauge->span);

		/* positions of the range in radians */
		start_pos = gauge->start_radian+(angle1*span);
		stop_pos = gauge->start_radian+(angle2*span);
		/* Converted to funky GDK units */
		start_angle = -start_pos*(180/M_PI)*64;
		span = - (stop_pos-start_pos)*(180/M_PI)*64;

		lwidth = MIN (w/2, h/2)/20 < 1 ? 1: MIN (w/2, h/2)/20;
		gdk_gc_set_line_attributes(gauge->axis_gc,lwidth,
				GDK_LINE_SOLID,
				GDK_CAP_BUTT,
				GDK_JOIN_BEVEL);
		gdk_draw_arc(gauge->bg_pixmap,gauge->axis_gc,FALSE, 
				gauge->xc-gauge->radius*0.775, 
				gauge->yc-gauge->radius*0.775,
				2*(gauge->radius*0.775),
				2*(gauge->radius*0.775),
				start_angle,
				span);
	}


	/* gauge ticks */
	lwidth = MIN (w/2, h/2)/80 < 1 ? 1: MIN (w/2, h/2)/80;
	color.red=0.8 * 65535;
	color.green=0.8 * 65535;
	color.blue=0.8 * 65535;
	gdk_gc_set_rgb_fg_color(gauge->axis_gc,&color);
	gdk_gc_set_line_attributes(gauge->axis_gc,lwidth,
			GDK_LINE_SOLID,
			GDK_CAP_BUTT,
			GDK_JOIN_BEVEL);

	radians_per_major_tick = (gauge->stop_radian - gauge->start_radian)/(float)(gauge->major_ticks-1);
        radians_per_minor_tick = radians_per_major_tick/(float)(1+gauge->minor_ticks);
        /* Major ticks first */
        insetfrom = gauge->radius * gauge->tick_inset;

	counter = gauge->start_radian;
        for (i=0;i<gauge->major_ticks;i++)
	{
		inset = (gint) (gauge->major_tick_len * gauge->radius);
		lwidth = MIN (w/2, h/2)/80 < 1 ? 1: MIN (w/2, h/2)/80;
		gdk_gc_set_line_attributes(gauge->axis_gc,lwidth,
				GDK_LINE_SOLID,
				GDK_CAP_BUTT,
				GDK_JOIN_BEVEL);

		gdk_draw_line(gauge->bg_pixmap,gauge->axis_gc,

				gauge->xc + (gauge->radius - insetfrom) * cos (counter),
				gauge->yc + (gauge->radius - insetfrom) * sin (counter),
				gauge->xc + ((gauge->radius - insetfrom - inset) * cos (counter)),
				gauge->yc + ((gauge->radius - insetfrom - inset) * sin (counter)));
		/* Now the minor ticks... */
		if ((gauge->minor_ticks > 0) && (i < (gauge->major_ticks-1)))
		{
			inset = (gint) (gauge->minor_tick_len * gauge->radius);
			lwidth = MIN (w/2, h/2)/160 < 1 ? 1: MIN (w/2, h/2)/160;
			gdk_gc_set_line_attributes(gauge->axis_gc,lwidth,
					GDK_LINE_SOLID,
					GDK_CAP_BUTT,
					GDK_JOIN_BEVEL);
			for (j=1;j<=gauge->minor_ticks;j++)
			{
				subcounter = j*radians_per_minor_tick;
				gdk_draw_line(gauge->bg_pixmap,gauge->axis_gc,

						gauge->xc + (gauge->radius - insetfrom) * cos (counter+subcounter),
						gauge->yc + (gauge->radius - insetfrom) * sin (counter+subcounter),
						gauge->xc + ((gauge->radius - insetfrom - inset) * cos (counter+subcounter)),
						gauge->yc + ((gauge->radius - insetfrom - inset) * sin (counter+subcounter)));

			}

		}
		counter += radians_per_major_tick;

	}

	/* Units String */
	if (gauge->units_str)
	{
		tmpbuf = g_strdup_printf("%s %i",gauge->units_font,(gint)(gauge->radius*gauge->units_font_scale));
		gauge->font_desc = pango_font_description_from_string(tmpbuf);
		g_free(tmpbuf);
		pango_layout_set_font_description(gauge->layout,gauge->font_desc);
		pango_layout_set_text(gauge->layout,gauge->units_str,-1);
		pango_layout_get_pixel_extents(gauge->layout,NULL,&logical_rect);

		gdk_draw_layout(gauge->bg_pixmap,gauge->axis_gc,
				gauge->xc-(logical_rect.width/2),
				gauge->yc+(0.75 * gauge->radius)+logical_rect.height,gauge->layout);
	}

	/* Name String */
	if (gauge->name_str)
	{
		tmpbuf = g_strdup_printf("%s %i",gauge->name_font,(gint)(gauge->radius*gauge->name_font_scale));
		gauge->font_desc = pango_font_description_from_string(tmpbuf);
		g_free(tmpbuf);
		pango_layout_set_font_description(gauge->layout,gauge->font_desc);
		pango_layout_set_text(gauge->layout,gauge->name_str,-1);
		pango_layout_get_pixel_extents(gauge->layout,NULL,&logical_rect);


		gdk_draw_layout(gauge->bg_pixmap,gauge->axis_gc,
				gauge->xc-(logical_rect.width/2),
				gauge->yc-(0.35 * gauge->radius)-logical_rect.height,gauge->layout);
	}


}

gboolean mtx_gauge_face_button_press (GtkWidget *gauge,
					     GdkEventButton *event)
{
	MtxGaugeFacePrivate *priv;
	priv = MTX_GAUGE_FACE_GET_PRIVATE (gauge);

	return FALSE;
}

void mtx_gauge_face_redraw_canvas (MtxGaugeFace *gauge)
{
	GtkWidget *widget;

	widget = GTK_WIDGET (gauge);

	if (!widget->window) return;

	update_gauge_position(widget);
	gdk_window_clear(widget->window);
}

gboolean mtx_gauge_face_button_release (GtkWidget *gauge,
					       GdkEventButton *event)
{
	MtxGaugeFacePrivate *priv;
	priv = MTX_GAUGE_FACE_GET_PRIVATE (gauge);
	return FALSE;
}

GtkWidget *mtx_gauge_face_new ()
{
	return GTK_WIDGET (g_object_new (MTX_TYPE_GAUGE_FACE, NULL));
}
