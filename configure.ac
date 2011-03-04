#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(maui, 3.3.1, help@supercluster.org)
AC_CONFIG_SRCDIR([src/mcom/MConst.c])
#AC_CONFIG_HEADER([config.h])


# Checks for programs.
AC_PROG_CC
#AC_PROG_INSTALL
AC_PROG_LN_S

# Assign desired prefix default
AC_PREFIX_DEFAULT([/usr/local/maui])

# Handle spooldir (homedir)
AC_ARG_WITH(spooldir,
  [  --with-spooldir=DIR     where config files, log and stat directories are maintained [[/usr/local/maui]]],
      SPOOLDIR=$withval, SPOOLDIR=/usr/local/maui)
AC_SUBST(SPOOLDIR)

# Machine Name
MACHINENAME=`echo ${ac_hostname} | tr '[[:lower:]]' '[[:upper:]]'`
AC_ARG_WITH(machine,
  [  --with-machine=NAME     system name (defaults to uppercased hostname)],
      MACHINE=$withval, MACHINE=$MACHINENAME)
AC_SUBST(MACHINE)

# Handle Checksum seed
AC_ARG_WITH(key,
  [  --with-key=SEED         security checksum key (defaults to a random integer)], SEED=$withval, SEED=$RANDOM)
AC_SUBST(SEED)

# Determine OPSYS
OS=`uname`
if test "$OS" = "AIX"; then
  if test `uname -v` = 4; then
    if test `uname -r` = 1; then
      OPSYS="__AIX41"
    elif test `uname -r` = 2; then
      OPSYS="__AIX42"
    elif test `uname -r` = 3; then
      OPSYS="__AIX43"
    else
      OPSYS="__AIX43"
      RELEASE=`uname -r`
      AC_MSG_WARN([Unexpected OS release 'AIX4$RELEASE', attempting build with OPSYS=$OPSYS])
    fi
  else
    OPSYS=__AIX43
    OSLEVEL=`oslevel`
    AC_MSG_WARN([Unexpected AIX level: $OSLEVEL, attempting build with OPSYS=$OPSYS])
  fi
elif echo "$OS" | grep -q "CYGWIN"; then
  OPSYS="__CYGWIN"
elif test "$OS" = "Linux"; then
  OPSYS="__LINUX"
elif test "$OS" = "HP-UX"; then
  OPSYS="__HPUX"
elif test "$OS" = "HP-UX"; then
  OPSYS="__HPUX"
elif test "$OS" = "IRIX" -o "$OS" = "IRIX64"; then
  OPSYS="__IRIX"
  OS="IRIX"
elif test "$OS" = "NT"; then
  OPSYS="__NT"
  OS="NT"
elif test "$OS" = "OSF1"; then
  OPSYS="__OSF"
  OS="OSF"
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

# LoadLeveler
AC_ARG_WITH(loadl,
	[  --with-loadl[[=VERSION]]  use LoadLeveler Resource Manager (version [[3]])],
	[LLVERSION=$withval],
	[LLVERSION='FALSE'])
if test "$LLVERSION" != 'FALSE'; then
	if test "$LLVERSION" = 'yes'; then
		LLVERSION=3;
	else
		LLVERSION=${withval%.*};
	fi
	if test "$RMTYPE" = ''; then
		RMTYPE=LL
		AC_SUBST(RMTYPE)
		AC_SUBST_FILE(rm_config)
		rm_config=include/maui.cfg.dist.inc.rm
	else
    AC_MSG_ERROR([Only one Resource Manager can be specified (LL, $RMTYPE)])
	fi
	if test "$LLVERSION" -lt "3"; then
		LLDIR="/usr/lpp/LoadL/full"
		LLDEF="-D__MLL22"
	else
		LLDIR="/home/loadl"
		LLDEF="-D__MLL31"
	fi
	AC_SUBST(LLDIR)
	AC_SUBST(LLDEF)
	AC_SUBST_FILE(ll_definitions)
	ll_definitions=include/Makefile.inc.ll
fi

# SDR
AC_ARG_WITH(sdr,
	[  --with-sdr              use the IBM SP2 SDR to obtain node info],
	[USESDR='TRUE'],
	[USESDR='FALSE'])
if test "$USESDR" != 'FALSE'; then
	AC_SUBST_FILE(sdr_definitions)
	sdr_definitions=include/Makefile.inc.sdr
fi

# PBS
AC_ARG_WITH(pbs,
	[  --with-pbs[[=DIR]]        use PBS Resource Manager (target directory (location of pbs lib dir) [[/usr/local]])],
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

# RMS
AC_ARG_WITH(rms,
	[  --with-rms[[=standalone]] use Quadrics RMS (default is not as a standalone Resource Manager [[]])],
	[USERMS=$withval],
	[USERMS='FALSE'])
if test "$USERMS" != 'FALSE'; then
	if test "$USERMS" = 'yes'; then
		USERMS='supporting';
	fi
	if test "$USERMS" = 'standalone'; then
		if test "$RMTYPE" = ''; then
			RMTYPE=RMS
		else
    			AC_MSG_ERROR([Only one Resource Manager can be specified (RMS, $RMTYPE)])
		fi
		#AC_SUBST_FILE(rms_definitions)
		#rms_definitions=include/Makefile.inc.rms
	fi
fi

# SGE
AC_ARG_WITH(sge,
	[  --with-sge[[=DIR]]        use SGE Resource Manager (build directory (location of SGE include and lib directories) [[/usr/local/sge/source]])],
	[SGEBUILDDIR=$withval],
	[SGEBUILDDIR='FALSE'])
if test "$SGEBUILDDIR" != 'FALSE'; then
	if test "$SGEBUILDDIR" = 'yes'; then
		SGEBUILDDIR='/usr/local/sge/source';
	fi
	if test "$RMTYPE" = ''; then
		RMTYPE=SGE
		AC_SUBST(RMTYPE)
		AC_SUBST_FILE(rm_config)
		rm_config=include/maui.cfg.dist.inc.rm
	else
		AC_MSG_ERROR([Only one Resource Manager can be specified (SGE, $RMTYPE)])
	fi
	SGEOSDIR=`(cd $SGEBUILDDIR; ./aimk -no-mk)`
	AC_SUBST(SGEBUILDDIR)
	AC_SUBST(SGEOSDIR)
	AC_SUBST_FILE(sge_definitions)
	sge_definitions=include/Makefile.inc.sge
fi


# LSF

AC_ARG_WITH(lsf,
	[  --with-lsf[[=DIR]]        use LSF Resource Manager (install directory (location of LSF include and lib directories) [[/usr/local/lsf]])],
	[LSFDIR=$withval],
	[LSFDIR='FALSE'])
if test "$LSFDIR" != 'FALSE'; then
	if test "$LSFDIR" = 'yes'; then
		LSFDIR='/usr/local/lsf';
	fi
	if test "$RMTYPE" = ''; then
		RMTYPE=LSF
		AC_SUBST(RMTYPE)
		AC_SUBST_FILE(rm_config)
		rm_config=include/maui.cfg.dist.inc.rm
	else
    AC_MSG_ERROR([Only one Resource Manager can be specified (LSF, $RMTYPE)])
	fi
	if test "$USERMS" = 'supporting'; then
		LSFLIBS="-lextsch -lbat -llsf -lrmsemu -lnsl -ldl"
		LSFDEFS="-D__MLSF -D__MLSFRMS"
	else
		LSFLIBS="-lextsch -lbat -llsf -lnsl -ldl"
		LSFDEFS="-D__MLSF"
	fi
	AC_SUBST(LSFDIR)
	AC_SUBST(LSFLIBS)
	AC_SUBST(LSFDEFS)
	AC_SUBST_FILE(lsf_definitions)
	lsf_definitions=include/Makefile.inc.lsf
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


# Bamboo Job Manager

AC_ARG_WITH(bamboo,
	[  --with-bamboo           use Bamboo Resource Manager],
	[USEBAMBOO='TRUE'],
	[USEBAMBOO='FALSE'])

if test "$USEBAMBOO" != 'FALSE'; then
  if test "$RMTYPE" != ''; then
    AC_MSG_ERROR([Only one Resource Manager can be specified (BAMBOO, $RMTYPE)])
  fi

  RMTYPE=SSS
  RMPORT=" PORT=4204"
  RMHOST=" HOST=$HOSTNAME"
  RMSOCKETPROTOCOL=" SOCKETPROTOCOL=HTTP"
  RMWIREPROTOCOL=" WIREPROTOCOL=SSS2"
  RMVERSION=" VERSION=SSS2.0"
fi


# Other SSS Resource Manager

AC_ARG_WITH(sss-rm,
	[  --with-sss-rm[[=PORT]]    use SSS Resource Manager (default port [[7865]])],
	[SSSRMPORT=$withval],
	[SSSRMPORT='FALSE'])

if test "$SSSRMPORT" != 'FALSE'; then
  if test "$SSSRMPORT" = 'yes'; then
    SSSRMPORT='7865';
  fi

  if test "$RMTYPE" != ''; then
    AC_MSG_ERROR([Only one Resource Manager can be specified (SSS-RM, $RMTYPE)])
  fi

  RMTYPE=SSS
  RMPORT=" PORT=$SSSRMPORT"
  RMHOST=" HOST=$HOSTNAME"
  RMSOCKETPROTOCOL=" SOCKETPROTOCOL=HTTP"
  RMWIREPROTOCOL=" WIREPROTOCOL=SSS2"
  RMVERSION=" VERSION=SSS2.0"
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


# MX
AC_ARG_WITH(mx,
	[  --with-mx               use MX],
	[USEMX='TRUE'],
	[USEMX='FALSE'])
if test "$USEMX" != 'FALSE'; then
	AC_SUBST_FILE(mx_definitions)
	mx_definitions=include/Makefile.inc.mx
fi

# PCRE
AC_ARG_WITH(pcre,
	[  --with-pcre[[=DIR]]       use PCRE (pcre dir (location of PCRE include and lib directories) [[/path/to/pcre]]],
	[PCREDIR=$withval],
	[PCREDIR='FALSE'])
if test "$PCREDIR" != 'FALSE'; then
	if test "$PCREDIR" = 'yes'; then
		PCREDIR='/path/to/pcre';
	fi
	AC_SUBST(PCREDIR)
	AC_SUBST_FILE(pcre_definitions)
	pcre_definitions=include/Makefile.inc.pcre
fi


# CYGRUNSRV
AC_ARG_WITH(cygrunsrv,
	[  --with-cygrunsrv        run as Windows service for Cygwin (do not use a background daemon)],
	[DAEMONDEF="-D__CYGRUNSRV"],
	[DAEMONDEF=""])
AC_SUBST(DAEMONDEF)


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
AC_CONFIG_FILES([include/Makefile.inc.ll
                 include/Makefile.inc.sdr
                 include/Makefile.inc.pbs
                 include/Makefile.inc.sge
                 include/Makefile.inc.lsf
                 include/Makefile.inc.mx
                 include/Makefile.inc.pcre])
AC_CONFIG_FILES([Makefile], [chmod 700 Makefile])
AC_CONFIG_FILES([include/maui.cfg.dist.inc.rm
                 maui.cfg.dist
                 maui-private.cfg.dist])
AC_CONFIG_FILES([etc/maui.sh etc/maui.csh])
AC_CONFIG_FILES([include/moab-local.h], [chmod 600 include/moab-local.h])
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
AC_MSG_NOTICE([NOTE:  link 'docs/mauidocs.html' to your local website for access to user and admin documentation
NOTE:  latest downloads, patches, etc are available at 'http://supercluster.org/maui'

configure successful.])

