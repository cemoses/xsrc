dnl Process this file with autoconf to create configure.

# Initialize Autoconf
AC_PREREQ([2.60])
AC_INIT([xhost], [1.0.5],
        [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg], [xhost])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADERS([config.h])

# Initialize Automake
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AM_MAINTAINER_MODE

# Require X.Org macros 1.8 or later for MAN_SUBSTS set by XORG_MANPAGE_SECTIONS
m4_ifndef([XORG_MACROS_VERSION],
	  [m4_fatal([must install xorg-macros 1.8 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.8)
XORG_DEFAULT_OPTIONS

AC_TYPE_SIGNAL

# Checks for pkg-config packages
# - Both CFLAGS & LIBS needed:
PKG_CHECK_MODULES(XHOST, x11 xmuu)
# - Only CFLAGS needed, not libs:
PKG_CHECK_MODULES(XAU, xau)

# Transport selection macro from xtrans.m4
XTRANS_CONNECTION_FLAGS

# Secure RPC detection macro from xtrans.m4
XTRANS_SECURE_RPC_FLAGS

AC_CONFIG_FILES([
	Makefile
	man/Makefile])
AC_OUTPUT
