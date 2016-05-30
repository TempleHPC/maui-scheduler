Name:           maui-scheduler
Version:        3.5
Release:        1%{?dist}
Summary:        Maui Cluster Scheduler for Torque

Group:          System Environment/Daemons
License:        Cluster Resourced End User Open Source License
Vendor:         Temple HPC Team
URL:            http://github.com/TempleHPC/maui-scheduler
Source0:        %{name}-%{version}-TempleHPC.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  torque-devel
#Requires:       

%description
This package contains the Maui Scheduler, an advance reservation based HPC 
batch scheduler with extensive job management and resource optimization
features.  This version of Maui requires the Moab Local Scheduling System 
(2.3 or higher) from Cluster Resources, Inc, which is also bundled in this 
distribution.

This version has been modified by the HPC team at Temple University for
local use and additional stability and reliability.


%prep
%setup -q -n %{name}-%{version}-TempleHPC


%build
%configure --with-spooldir=%{_sharedstatedir}/maui --with-pbs --with-machine=localhost
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
rm -rf ${RPM_BUILD_ROOT}/usr/include/moab.h
rm -rf ${RPM_BUILD_ROOT}/usr/lib/libmcom.a
rm -rf ${RPM_BUILD_ROOT}/usr/lib/libmoab.a

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%{_bindir}/canceljob
%{_bindir}/changeparam
%{_bindir}/checkjob
%{_bindir}/checknode
%{_bindir}/diagnose
%{_bindir}/mbal
%{_bindir}/mclient
%{_bindir}/mdiag
%{_bindir}/mjobctl
%{_bindir}/mnodectl
%{_bindir}/mprof
%{_bindir}/mschedctl
%{_bindir}/mstat
%{_bindir}/releasehold
%{_bindir}/releaseres
%{_bindir}/resetstats
%{_bindir}/runjob
%{_bindir}/schedctl
%{_bindir}/sethold
%{_bindir}/setqos
%{_bindir}/setres
%{_bindir}/setspri
%{_bindir}/showbf
%{_bindir}/showconfig
%{_bindir}/showgrid
%{_bindir}/showhold
%{_bindir}/showq
%{_bindir}/showres
%{_bindir}/showstart
%{_bindir}/showstate
%{_bindir}/showstats
%{_sbindir}/maui
%{_unitdir}/maui-scheduler.service
%config(noreplace) %{_sharedstatedir}/maui/maui-private.cfg
%config(noreplace) %{_sharedstatedir}/maui/maui.cfg

%doc LICENSE LICENSE.mcompat README CHANGELOG ChangeLog.TempleHPC

%changelog
* Mon May 30 2016 Axel Kohlmeyer <akohlmey@gmail.com> - 3.5-1
- Update version to 3.5

* Fri May 20 2016 Axel Kohlmeyer <akohlmey@gmail.com> - 3.4-1
- Initial version of rpm spec file. Tested on Fedora 23

