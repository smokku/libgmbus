INCLUDES = @GLIB_CFLAGS@ -I..

if WITH_GUIDELINES
GUIDE_C = rpctypes.c rpcentity.c
GUIDE_H = rpctypes.h rpcentity.h
else
GUIDE_C=
GUIDE_H=
endif

lib_LTLIBRARIES = libgmbus.la

libgmbus_la_SOURCES = \
	base64.c hmac.c init.c maddress.c mclient.c \
	mcommand.c md5.c mcoder.c mheader.c mlink.c mpayload.c \
	mmessage.c mtrans.c mutil.c udp.c mtypes.c \
	mconfig.c $(GUIDE_C)

pkginclude_HEADERS = mbus.h base64.h hmac.h init.h \
	maddress.h mclient.h mcommand.h md5.h mcoder.h \
	mheader.h mlink.h mpayload.h mmessage.h mtrans.h \
	mutil.h udp.h mtypes.h mconfig.h $(GUIDE_H) gmbus.h

EXTRA_DIST = gmbus.h.in

libgmbus_la_LDFLAGS = @GLIB_LIBS@
