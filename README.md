==========================================================================
==================Gstreamer plugin gst-chessboard=========================
==========================================================================

Copyright: 2014 UbiCast (http://ubicast.eu)
License: http://www.gnu.org/copyleft/gpl.html GNU GPL v3 or later



Description:
    Detect chessboard corners in a video or image and draw it using Opencv functions and Gstreamer. 


Install:
    Tested on Ubuntu 10.04
    To compile, additionally to the basic development tools (from build-essential, automake, autoconf, libtool), you will need (from apt on Ubuntu/Debian systems):
    libgstreamer-plugins-base0.10-dev libgstreamer0.10-dev Opencv 2.0 or more 
    
    ./autogen.sh
    make
    cp src/.libs/libgstchessfind.so ~/.gstreamer-0.10/plugins/

Checking
    Now you can use gst-inspect to verify the plugin has been built and installed successfully:
    gst-inspect chessfind 
    
Options
    gst-inspect chessfind:
      display : Sets whether the detected chessboard should be highlighted in
          the output flags: accès en lecture, accès en écriture Boolean. Default: true Current: true 
      rows : Number of chessboard's rows
          flags: accès en lecture, accès en écriture Integer. Range: 4 - 2147483647 Default: 4 Current: 4 
      columns : Number of chessboard's columns
          flags: accès en lecture, accès en écriture Integer. Range: 4 - 2147483647 Default: 6 Current: 6 

Example pipelines
    With a video : gst-launch -v -m filesrc location=video.ogg ! decodebin ! ffmpegcolorspace ! queue ! chessfind ! queue ! ffmpegcolorspace ! ximagesink 
