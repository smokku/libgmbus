INCLUDES = @GLIB_CFLAGS@ -I..
AM_CFLAGS = -ansi -Wreturn-type -Wstrict-prototypes -Wall -g

noinst_PROGRAMS=mbustest mentity mtest crypt rpcserver rpcclient \
	fredtest printll

mtest_SOURCES = mtest.c
mtest_LDFLAGS = @GLIB_LIBS@ $(top_builddir)/gmbus/libgmbus.la

mbustest_SOURCES = mbustest.c
mbustest_LDADD = @GLIB_LIBS@ $(top_builddir)/gmbus/libgmbus.la

mentity_SOURCES = mentity.c
mentity_LDADD = @GLIB_LIBS@ $(top_builddir)/gmbus/libgmbus.la

crypt_SOURCES = crypt.c
crypt_LDADD = @GLIB_LIBS@ $(top_builddir)/gmbus/libgmbus.la

rpcserver_SOURCES = rpcserver.c
rpcserver_LDADD = @GLIB_LIBS@ $(top_builddir)/gmbus/libgmbus.la

rpcclient_SOURCES = rpcclient.c
rpcclient_LDADD = @GLIB_LIBS@ $(top_builddir)/gmbus/libgmbus.la

fredtest_SOURCES = fredtest.c
fredtest_LDADD = @GLIB_LIBS@ $(top_builddir)/gmbus/libgmbus.la

printll_SOURCES = printll.c
printll_LDADD = @GLIB_LIBS@ $(top_builddir)/gmbus/libgmbus.la
