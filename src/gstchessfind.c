/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2010 lavi <<user@hostname.org>>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-chessfind
 *
 * FIXME:Describe chessfind here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! chessfind ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <gst/video/video.h>
#include "gstchessfind.h"

GST_DEBUG_CATEGORY_STATIC (gst_chessfind_debug);
#define GST_CAT_DEFAULT gst_chessfind_debug
#define DEFAULT_ROWS 4
#define DEFAULT_COLUMNS 6

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_DISPLAY,
  PROP_SILENT,
  PROP_ROWS,
  PROP_COLUMNS
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_RGB ";")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_RGB ";")
    );

GST_BOILERPLATE (Gstchessfind, gst_chessfind, GstElement,
    GST_TYPE_ELEMENT);

static void gst_chessfind_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_chessfind_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_chessfind_set_caps (GstPad * pad, GstCaps * caps);
static GstFlowReturn gst_chessfind_chain (GstPad * pad, GstBuffer * buf);

/* GObject vmethod implementations */

static void
gst_chessfind_base_init (gpointer gclass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);

  gst_element_class_set_details_simple(element_class,
    "chessfind",
    "OpenCV",
    "Performs chessboard detection on videos and images, send message when found, draw chessboard corners",
    "lavi <<flavie.lancereau@ubicast.eu>>");

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_factory));
}

/* initialize the chessfind's class */
static void
gst_chessfind_class_init (GstchessfindClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_chessfind_set_property;
  gobject_class->get_property = gst_chessfind_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
				   g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
							 FALSE, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_DISPLAY,
				   g_param_spec_boolean ("display", "Display",
							 "Sets whether the detected chessboard should be highlighted in the output",
							 TRUE,
							 G_PARAM_READWRITE));


  g_object_class_install_property (gobject_class, PROP_ROWS,
				   g_param_spec_int ("rows", "Number of rows",
						     "Number of chessboard's rows",
						     4, G_MAXINT, DEFAULT_ROWS,
						     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_COLUMNS,
				   g_param_spec_int ("columns", "Number of columns",
						     "Number of chessboard's columns",
						     4, G_MAXINT, DEFAULT_COLUMNS,
						     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_chessfind_init (Gstchessfind * filter,
    GstchessfindClass * gclass)
{
  filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
  gst_pad_set_setcaps_function (filter->sinkpad,
				GST_DEBUG_FUNCPTR(gst_chessfind_set_caps));
  gst_pad_set_getcaps_function (filter->sinkpad,
				GST_DEBUG_FUNCPTR(gst_pad_proxy_getcaps));
  gst_pad_set_chain_function (filter->sinkpad,
			      GST_DEBUG_FUNCPTR(gst_chessfind_chain));

  filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  gst_pad_set_getcaps_function (filter->srcpad,
				GST_DEBUG_FUNCPTR(gst_pad_proxy_getcaps));

  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);
  filter->silent = FALSE;
  filter->display = TRUE;
  filter->rows = DEFAULT_ROWS;
  filter->columns = DEFAULT_COLUMNS;
}

static void
gst_chessfind_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  Gstchessfind *filter = GST_CHESSFIND (object);

  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      break;
    case PROP_DISPLAY:
      filter->display = g_value_get_boolean (value);
      break;
    case PROP_ROWS:
      filter->rows = g_value_get_int (value);
      break;
    case PROP_COLUMNS:
      filter->columns = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_chessfind_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  Gstchessfind *filter = GST_CHESSFIND (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    case PROP_DISPLAY:
      g_value_set_boolean (value, filter->display);
      break;
    case PROP_ROWS:
      g_value_set_int (value, filter->rows);
      break;
    case PROP_COLUMNS:
      g_value_set_int (value, filter->columns);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles the link with other elements */
static gboolean
gst_chessfind_set_caps (GstPad * pad, GstCaps * caps)
{
  Gstchessfind *filter;
  GstPad *otherpad;
  gint width, height;
  GstStructure *structure;

  filter = GST_CHESSFIND (gst_pad_get_parent (pad));
  otherpad = (pad == filter->srcpad) ? filter->sinkpad : filter->srcpad;
  gst_object_unref (filter);

  structure = gst_caps_get_structure (caps, 0);
  gst_structure_get_int (structure, "width", &width);
  gst_structure_get_int (structure, "height", &height);
  filter->width=width;
  filter->height=height;

  filter->currentImage = cvCreateImage(cvSize(filter->width, filter->height), IPL_DEPTH_8U, 3);
  filter->grayImage = cvCreateImage(cvSize(filter->width, filter->height), IPL_DEPTH_8U, 1);
  return gst_pad_set_caps (otherpad, caps);
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_chessfind_chain (GstPad * pad, GstBuffer * buf)
{
  Gstchessfind *filter;

  filter = GST_CHESSFIND (GST_OBJECT_PARENT (pad));
  filter->currentImage->imageData = (char *) GST_BUFFER_DATA (buf);
  cvCvtColor(filter->currentImage, filter->grayImage, CV_RGB2GRAY);

  CvPoint2D32f *corners = (CvPoint2D32f*) malloc(sizeof(CvPoint2D32f)*(filter->rows-1)*(filter->columns-1));
  int corner_count;
  cvAdaptiveThreshold(filter->grayImage, filter->grayImage, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 151, 1);
  int found = cvFindChessboardCorners(filter->grayImage,
				       cvSize( filter->rows-1 , filter->columns-1 ),
				       corners,
				       &corner_count,
				       0 );

  if (found && corner_count == (filter->rows-1)*(filter->columns-1))
    {
      GstStructure *s = gst_structure_new ("chessboard",
					   "timestamp", G_TYPE_UINT64,  GST_BUFFER_TIMESTAMP(buf) ,
					   NULL);

      GstMessage *m = gst_message_new_element (GST_OBJECT (filter), s);
      gst_element_post_message (GST_ELEMENT (filter), m);

      if (filter->display)
	cvDrawChessboardCorners(filter->currentImage,
				cvSize( filter->rows-1 , filter->columns-1 ),
				corners,
				corner_count,
				found );
    }
  free(corners);
  /* just push out the incoming buffer without touching it */
  return gst_pad_push (filter->srcpad, buf);
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
chessfind_init (GstPlugin * chessfind)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template chessfind' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_chessfind_debug, "chessfind",
      0, "Template chessfind");

  return gst_element_register (chessfind, "chessfind", GST_RANK_NONE,
      GST_TYPE_CHESSFIND);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstchessfind"
#endif

/* gstreamer looks for this structure to register chessfinds
 *
 * exchange the string 'Template chessfind' with your chessfind description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "chessfind",
    "Template chessfind",
    chessfind_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/"
)
