
PIDGIN_TREE_TOP ?= ../pidgin-2.10.11
LIBPURPLE_DIR ?= $(PIDGIN_TREE_TOP)/libpurple
WIN32_DEV_TOP ?= $(PIDGIN_TREE_TOP)/../win32-dev
GTK_TOP ?= $(WIN32_DEV_TOP)/gtk_2_0-2.14

WIN32_CC ?= $(WIN32_DEV_TOP)/mingw-4.7.2/bin/gcc

PKG_CONFIG ?= pkg-config
DIR_PERM = 0755
LIB_PERM = 0755
FILE_PERM = 0644
MAKENSIS ?= makensis

CFLAGS	?= -O2 -g -pipe
LDFLAGS ?= 

# Do some nasty OS and purple version detection
ifeq ($(OS),Windows_NT)
  #only defined on 64-bit windows
  PROGFILES32 = ${ProgramFiles(x86)}
  ifndef PROGFILES32
    PROGFILES32 = $(PROGRAMFILES)
  endif
  PLUGIN_TARGET = paste_image.dll
  PLUGIN_DEST = "$(PROGFILES32)/Pidgin/plugins"
else

  UNAME_S := $(shell uname -s)

  #.. There are special flags we need for OSX
  ifeq ($(UNAME_S), Darwin)
    #
    #.. /opt/local/include and subdirs are included here to ensure this compiles
    #   for folks using Macports.  I believe Homebrew uses /usr/local/include
    #   so things should "just work".  You *must* make sure your packages are
    #   all up to date or you will most likely get compilation errors.
    #
    INCLUDES = -I/opt/local/include -lz $(OS)

    CC = gcc
  else
    INCLUDES = 
    CC ?= gcc
  endif

  ifeq ($(shell $(PKG_CONFIG) --exists purple 2>/dev/null && echo "true"),)
    PLUGIN_TARGET = FAILNOPURPLE
    PLUGIN_DEST =
  else
    PLUGIN_TARGET = paste_image.so
    PLUGIN_DEST = $(DESTDIR)`$(PKG_CONFIG) --variable=plugindir purple`
  endif
endif

WIN32_INCLUDE_PATHS +=	\
			-I$(GTK_TOP)/include \
			-I$(GTK_TOP)/include/gtk-2.0 \
			-I$(GTK_TOP)/include/glib-2.0 \
			-I$(GTK_TOP)/include/pango-1.0 \
			-I$(GTK_TOP)/include/gdk-pixbuf-2.0 \
			-I$(GTK_TOP)/include/atk-1.0 \
			-I$(GTK_TOP)/include/cairo \
			-I$(GTK_TOP)/lib/glib-2.0/include \
			-I$(GTK_TOP)/lib/gtk-2.0/include \
			-I$(PIDGIN_TREE_TOP) \
			-I$(PIDGIN_TREE_TOP)/libpurple \
			-I$(PIDGIN_TREE_TOP)/libpurple/win32 \
			-I$(PIDGIN_TREE_TOP)/pidgin \
			-I$(PIDGIN_TREE_TOP)/pidgin/win32
WIN32_LIB_PATHS += \
    -L$(GTK_TOP)/lib \
    -L$(PIDGIN_TREE_TOP)/libpurple \
    -L$(PIDGIN_TREE_TOP)/pidgin
WIN32_CFLAGS =  $(WIN32_INCLUDE_PATHS) -Wall -Wextra -Werror -Wno-deprecated-declarations -Wno-unused-parameter -fno-strict-aliasing -Wformat -Wno-sign-compare
WIN32_LDFLAGS = $(WIN32_LIB_PATHS) -lpurple -lpidgin -lgtk-win32-2.0 -lglib-2.0 -lgobject-2.0 -lgdk_pixbuf-2.0 -g -ggdb -static-libgcc 

C_FILES = \
	paste_image.c 



.PHONY:	all install FAILNOPURPLE clean translations

all: $(PLUGIN_TARGET)

paste_image.so: $(C_FILES) 
	$(CC) -fPIC $(CFLAGS) -shared -o $@ $^ $(LDFLAGS) $(PROTOBUF_OPTS) `$(PKG_CONFIG) purple pidgin glib-2.0 gtk-2.0 --libs --cflags`  $(INCLUDES) -g -ggdb

paste_image.dll: $(C_FILES)
	$(WIN32_CC) -shared -o $@ $^ $(WIN32_CFLAGS) $(WIN32_LDFLAGS) 

install: $(PLUGIN_TARGET)
	mkdir -m $(DIR_PERM) -p $(PLUGIN_DEST)
	install -m $(LIB_PERM) -p $(PLUGIN_TARGET) $(PLUGIN_DEST)

	
FAILNOPURPLE:
	echo "You need libpurple development headers installed to be able to compile this plugin"

clean:
	rm -f $(PLUGIN_TARGET)

