#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(Maui Cluster Scheduler, 3.5-TempleHPC, hpc@temple.edu, maui-scheduler, https://github.com/TempleHPC/maui-scheduler)
AC_CONFIG_SRCDIR([src/MConst.c])
#AC_CONFIG_HEADER([config.h])


# Checks for programs.
AC_PROG_CC
#AC_PROG_INSTALL
AC_PROG_LN_S

# Assign desired prefix default
AC_PREFIX_DEFAULT([/usr/local])

# Handle spooldir (homedir)
AC_ARG_WITH(spooldir,
  [  --with-spooldir=DIR     where config files, log and stat directories are maintained [[/var/spool/maui]]],
      SPOOLDIR=$withval, SPOOLDIR=/var/spool/maui)
AC_SUBST(SPOOLDIR)

# Machine Name
AC_ARG_WITH(machine,
  [  --with-machine=NAME     system name (defaults to LOCALHOST)],
      MACHINE=$withval, MACHINE=LOCALHOST)
AC_SUBST(MACHINE)

# Handle Checksum seed
AC_ARG_WITH(key,
  [  --with-key=SEED         security checksum key (defaults to a random integer)], SEED=$withval, SEED=$RANDOM)
AC_SUBST(SEED)

# Determine OPSYS
OS=`uname`
if test "$OS" = "AIX"; then
  OPSYS=__AIX43
elif test "$OS" = "Linux"; then
  OPSYS="__LINUX"
elif test "$OS" = "HP-UX"; then
  OPSYS="__HPUX"
elif test "$OS" = "IRIX" -o "$OS" = "IRIX64"; then
  OPSYS="__IRIX"
  OS="IRIX"
else
  OPSYS="__LINUX"
  AC_MSG_WARN([Unsupported OS: $OS, attempting build with OPSYS=$OPSYS])
  OS="LINUX"
fi
AC_SUBST(OPSYS)
AC_SUBST(OS)

# Make generic variable Substitutions
AC_SUBST([PACKAGE_NAME])
AC_SUBST([PACKAGE_VERSION])
AC_SUBST([PACKAGE_TARNAME])
AC_SUBST([HOSTNAME], ${ac_hostname})
AC_SUBST([USER], `whoami`)
AC_SUBST([BUILDDATE], `date`)
AC_SUBST([BUILDDIR], `pwd`)


# PBS
AC_ARG_WITH(pbs,
	[  --with-pbs[[=DIR]]        use PBS/Torque Resource Manager (target directory (location of pbs lib dir) [[/usr/local]])],
	[PBSTARGETDIR=$withval],
	[PBSTARGETDIR='FALSE'])
if test "$PBSTARGETDIR" != 'FALSE'; then
	if test "$PBSTARGETDIR" = 'yes'; then
		if pbs-config --prefix 1>/dev/null 2>&1;then
			PBSTARGETDIR=`pbs-config --prefix`
		else
			PBSTARGETDIR='/usr/local'
		fi
	elif test -f "$PBSTARGETDIR/bin/pbs-config"; then
		PBSTARGETDIR=`$PBSTARGETDIR/bin/pbs-config --prefix`
	elif test -f "$PBSTARGETDIR/../bin/pbs-config"; then
		PBSTARGETDIR=`$PBSTARGETDIR/../bin/pbs-config --prefix`
	elif test -f "$PBSTARGETDIR/pbs-config"; then
		PBSTARGETDIR=`$PBSTARGETDIR/pbs-config --prefix`
	elif test -f "$PBSTARGETDIR"; then
		PBSTARGETDIR=`$PBSTARGETDIR --prefix`
	fi
	if test "$RMTYPE" = ''; then
		RMTYPE=PBS
		AC_SUBST(RMTYPE)
		AC_SUBST_FILE(rm_config)
		rm_config=include/maui.cfg.dist.inc.rm
	else
		AC_MSG_ERROR([Only one Resource Manager can be specified (PBS, $RMTYPE)])
	fi
	AC_SUBST(PBSTARGETDIR)
	AC_SUBST_FILE(pbs_definitions)
	pbs_definitions=include/Makefile.inc.pbs
fi



# Wiki

AC_ARG_WITH(wiki,
	[  --with-wiki             use Wiki Resource Manager],
	[USEWIKI='TRUE'],
	[USEWIKI='FALSE'])

if test "$USEWIKI" != 'FALSE'; then
  if test "$RMTYPE" != ''; then
    AC_MSG_ERROR([Only one Resource Manager can be specified (WIKI, $RMTYPE)])
  fi

  RMTYPE=WIKI
fi

if test "$RMTYPE" = ''; then
       AC_MSG_WARN([Resource Manager not specified - attempting build with pbs])
       RMTYPE=PBS
       AC_SUBST(RMTYPE)
       AC_SUBST_FILE(rm_config)
       rm_config=include/maui.cfg.dist.inc.rm
 
       if pbs-config --prefix 1>/dev/null 2>&1;then
               PBSTARGETDIR=`pbs-config --prefix`
       else
               PBSTARGETDIR='/usr/local';
       fi
fi

if test "$RMTYPE" = "PBS" ;then
       if $PBSTARGETDIR/bin/pbs-config --cflags 1>/dev/null 2>&1;then
               PBSCFLAGS=`$PBSTARGETDIR/bin/pbs-config --cflags`
       else
               PBSCFLAGS=-I$PBSTARGETDIR/include
       fi
       if $PBSTARGETDIR/bin/pbs-config --libs 1>/dev/null 2>&1;then
               PBSLDFLAGS=`$PBSTARGETDIR/bin/pbs-config --libs`
       elif test -f $PBSTARGETDIR/lib/libpbs.a ;then
	       if test "$OS" = "OSF"; then
		       PBSLDFLAGS="$PBSTARGETDIR/ilib/libnet.a -L$PBSTARGETDIR/lib -lpbs"
	       else
		       PBSLDFLAGS="-L$PBSTARGETDIR/lib -lnet -lpbs"
	       fi
       else
               AC_MSG_ERROR([can't find pbs-config or libpbs.a])
       fi
       AC_SUBST(PBSCFLAGS)
       AC_SUBST(PBSLDFLAGS)
       AC_SUBST_FILE(pbs_definitions)
       pbs_definitions=include/Makefile.inc.pbs

fi


# Warehouse Node Monitor

AC_ARG_WITH(warehouse,
	[  --with-warehouse        use Warehouse System Monitor],
	[USEWAREHOUSE='TRUE'],
	[USEWAREHOUSE='FALSE'])

if test "$USEWAREHOUSE" != 'FALSE'; then
  if test "$NMTYPE" = ''; then
    RMNMPORT=" NMPORT=12321"
    RMNMHOST=" NMHOST=$HOSTNAME"
  fi
fi

AC_SUBST(RMTYPE)
AC_SUBST(RMPORT)
AC_SUBST(RMHOST)
AC_SUBST(RMSOCKETPROTOCOL)
AC_SUBST(RMWIREPROTOCOL)
AC_SUBST(RMVERSION)
AC_SUBST(RMNMPORT)
AC_SUBST(RMNMHOST)

# END RM CONFIG --------------------------------------

# START AM CONFIG ------------------------------------

# QBank

AC_ARG_WITH(qbank,
	[  --with-qbank            use QBank Allocation Manager],
	[USEQBANK='TRUE'],
	[USEQBANK='FALSE'])

if test "$USEQBANK" != 'FALSE'; then
  if test "$AMTYPE" != ''; then
    AC_MSG_ERROR([Only one Allocation Manager can be specified (QBANK, $AMTYPE)])
  fi

  AMTYPE=QBANK
  AMHOST=" HOST=$HOSTNAME"
  AMPORT=" PORT=7111"
  AMOTHER=" CHARGEPOLICY=DEBITALLWC JOBFAILUREACTION=NONE TIMEOUT=15"
fi


# Gold
AC_ARG_WITH(gold,
	[  --with-gold             use Gold Allocation Manager],
	[USEGOLD='TRUE'],
	[USEGOLD='FALSE'])

if test "$USEGOLD" != 'FALSE'; then
  if test "$AMTYPE" != ''; then
    AC_MSG_ERROR([Only one Allocation Manager can be specified (GOLD, $AMTYPE)])
  fi

  AMTYPE=GOLD
  AMHOST=" HOST=$HOSTNAME"
  AMPORT=" PORT=7112"
  AMSOCKETPROTOCOL=" SOCKETPROTOCOL=HTTP"
  AMWIREPROTOCOL=" WIREPROTOCOL=XML"
  AMOTHER=" CHARGEPOLICY=DEBITALLWC JOBFAILUREACTION=NONE TIMEOUT=15"
fi


# Other SSS Allocation Manager

AC_ARG_WITH(sss-am,
	[  --with-sss-am[[=PORT]]    use SSS Allocation Manager (default port [[8865]])],
	[SSSAMPORT=$withval],
	[SSSAMPORT='FALSE'])

if test "$SSSAMPORT" != 'FALSE'; then
  if test "$SSSAMPORT" = 'yes'; then
    SSSAMPORT='8865';
  fi

  if test "$AMTYPE" != ''; then
    AC_MSG_ERROR([Only one Allocation Manager can be specified (SSS-AM, $AMTYPE)])
  fi

  AMTYPE=SSS
  AMHOST=" HOST=$HOSTNAME"
  AMPORT=" PORT=$SSSAMPORT"
  AMOTHER=" CHARGEPOLICY=DEBITALLWC JOBFAILUREACTION=NONE TIMEOUT=15"
fi

if test "$AMTYPE" = ''; then
  AMTYPE=NONE
fi

AC_SUBST(AMTYPE)
AC_SUBST(AMHOST)
AC_SUBST(AMPORT)
AC_SUBST(AMSOCKETPROTOCOL)
AC_SUBST(AMWIREPROTOCOL)
AC_SUBST(AMOTHER)

# END AM CONFIG ---------------------------------


# PCRE
pcre_definitions=""
AC_ARG_WITH(pcre,
	[  --with-pcre[[=DIR]]       use PCRE (pcre dir (location of PCRE include and lib directories) [[/path/to/pcre]]],
	[PCREDIR=$withval],
	[PCREDIR='FALSE'])
if test "$PCREDIR" != 'FALSE'; then
	if test "$PCREDIR" = 'yes'; then
		PCREDIR='/path/to/pcre';
	fi
        AC_SUBST(PCREDIR)
        pcre_definitions=include/Makefile.inc.pcre
fi
AC_SUBST_FILE(pcre_definitions)


# Make sure one RM was specified
if test "$RMTYPE" = ''; then
  AC_MSG_ERROR([At least one Resource Manager must be specified])
fi

# Checks for libraries.
AC_CHECK_LIB([m], [sqrt])

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h locale.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/socket.h sys/time.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STAT
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_PID_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_SETPGRP
AC_FUNC_SETVBUF_REVERSED
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([gethostbyaddr gethostbyname gethostname gettimeofday inet_ntoa memchr memset pow putenv regcmp regcomp select setlocale socket strchr strdup strerror strrchr strstr strtol strtoul])

# Configure the output files, and generate them.
AC_CONFIG_FILES([include/Makefile.inc.pbs
                 include/Makefile.inc.pcre])
AC_CONFIG_FILES([Makefile], [chmod 700 Makefile])
AC_CONFIG_FILES([include/maui.cfg.dist.inc.rm
                 maui.cfg.dist
                 maui-private.cfg.dist])
AC_CONFIG_FILES([etc/maui.sh etc/maui.csh])
AC_CONFIG_FILES([include/moab-local.h], [chmod 600 include/moab-local.h])
AC_CONFIG_FILES([include/msched-version.h], [chmod 600 include/msched-version.h])
AC_CONFIG_FILES([docs/Doxyfile])
AC_CONFIG_FILES([maui-scheduler.service])
AC_OUTPUT

# Do miscellaneous configure steps
touch maui.ck
touch maui-private.cfg
test -d bin || mkdir bin
test -d lib || mkdir lib

# Display final messages
if test -z "`echo $PATH | grep \"$prefix\" 2>/dev/null`"; then
	AC_MSG_NOTICE([
Remember to add $prefix/bin to your PATH])
fi

