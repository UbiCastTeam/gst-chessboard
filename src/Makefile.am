# plugindir is set in configure

##############################################################################
# change libgst-trackle.la to something more suitable, e.g. libmysomething.la  #
##############################################################################
plugin_LTLIBRARIES = libgstchessfind.la

##############################################################################
# for the next set of variables, rename the prefix if you renamed the .la,   #
#  e.g. libgstplugin_la_SOURCES => libmysomething_la_SOURCES                 #
#       libgstplugin_la_CFLAGS  => libmysomething_la_CFLAGS                  #
#       libgstplugin_la_LIBADD  => libmysomething_la_LIBADD                  #
#       libgstplugin_la_LDFLAGS => libmysomething_la_LDFLAGS                 #
##############################################################################

# sources used to compile this plug-in
libgstchessfind_la_SOURCES = gstchessfind.c

# flags used to compile this plugin
# add other _CFLAGS and _LIBS as needed
libgstchessfind_la_CFLAGS = $(GST_CFLAGS) $(OPENCV_CFLAGS)
libgstchessfind_la_LIBADD = $(GST_LIBS) $(OPENCV_LIBS) $(GST_BASE_LIBS) $(GSTCTRL_LIBS)
libgstchessfind_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstchessfind_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstchessfind.h
