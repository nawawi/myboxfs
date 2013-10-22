Summary: System logging and kernel message trapping daemons.
Name: sysklogd
Version: 1.4.1
Release: 22
Copyright: GPL
Group: System Environment/Daemons
Source: sysklogd-%{version}rh.tar.gz
Prereq: fileutils /sbin/chkconfig /etc/init.d
Conflicts: logrotate < 3.5.2
Requires: bash >= 2.0
BuildRoot: /var/tmp/syslog-root

%description
The sysklogd package contains two system utilities (syslogd and klogd)
which provide support for system logging.  Syslogd and klogd run as
daemons (background processes) and log system messages to different
places, like sendmail logs, security logs, error logs, etc.

%prep
%setup -q -n sysklogd-%{version}rh
%ifarch s390 s390x
perl -pi -e 's/-fpie/-fPIE/' Makefile
%endif

%build
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT{/etc,%{_bindir},%{_mandir}/man{5,8},/usr/sbin}
mkdir -p $RPM_BUILD_ROOT/sbin

make install TOPDIR=$RPM_BUILD_ROOT MANDIR=$RPM_BUILD_ROOT%{_mandir} \
	MAN_OWNER=`id -nu`

install -m644 redhat/syslog.conf.rhs $RPM_BUILD_ROOT/etc/syslog.conf

mkdir -p $RPM_BUILD_ROOT/etc/{rc.d/init.d,logrotate.d,sysconfig}
install -m755 redhat/syslog.init $RPM_BUILD_ROOT/etc/rc.d/init.d/syslog
install -m644 redhat/syslog.log $RPM_BUILD_ROOT/etc/logrotate.d/syslog
install -m644 redhat/syslog $RPM_BUILD_ROOT/etc/sysconfig/syslog

chmod 755 $RPM_BUILD_ROOT/sbin/syslogd
chmod 755 $RPM_BUILD_ROOT/sbin/klogd


%clean
rm -rf $RPM_BUILD_ROOT

%post
for n in /var/log/{messages,secure,maillog,spooler}
do
	[ -f $n ] && continue
	touch $n
	chmod 600 $n
done
/sbin/chkconfig --add syslog

%preun
if [ $1 = 0 ]; then
   service syslog stop >/dev/null 2>&1
   /sbin/chkconfig --del syslog
fi

%postun
if [ "$1" -ge "1" ]; then
	service syslog condrestart > /dev/null 2>&1
fi	

%triggerpostun -- sysklogd < 1.3.31-17
/sbin/chkconfig --add syslog

%triggerpostun -- sysklogd < 1.3.33-5
/sbin/chkconfig syslog reset

%files
%defattr(-,root,root)
%doc ANNOUNCE CHANGES README* NEWS INSTALL redhat/ChangeLog
%config(noreplace) /etc/syslog.conf
%config(noreplace) /etc/sysconfig/syslog
%config /etc/logrotate.d/syslog
%config /etc/rc.d/init.d/syslog
/sbin/*
%{_mandir}/*/*

%changelog
* Mon Jun 28 2004 Bill Nottingham <notting@redhat.com> 1.4.1rh-21
- async logging for mail (#73306)

* Sun Jun 20 2004 Florian La Roche <Florian.LaRoche@redhat.de>
- do not set bsd compat on sockets #123912
- fix empty log lines #125679

* Fri Jun 11 2004 Florian La Roche <laroche@redhat.com> 1.4.1rh-18
- make the race for -HUP a little bit smaller

* Wed Jun  9 2004 Bill Nottingham <notting@redhat.com> 1.4.1rh-17
- don't escape UTF-8 (#89292, #71170, #112519)

* Mon May  3 2004 Bill Nottingham <notting@redhat.com> 1.4.1rh-16
- add Owl patch for crunch_list function, fixes potential crashes (#120453)

* Wed Apr  7 2004 Bill Nottingham <notting@redhat.com> 1.4.1rh-15
- fix recvfrom() on 64-bit big-endian platforms (#120201)

* Mon Mar  8 2004 Bill Nottingham <notting@redhat.com> 1.4.1rh-14
- rebuild (#117696)

* Thu Feb 12 2004 Thomas Woerner <twoerner@redhat.com> 1.4.1rh-13
- make sysklogd pie

* Fri Feb  7 2003 Tim Powers <timp@redhat.com> 1.4.1rh-12
- rebuild

* Wed Jan  8 2003 Tim Powers <timp@redhat.com> 1.4.1rh-11
- bump release number

* Fri Dec 20 2002 Elliot Lee <sopwith@redhat.com> 1.4.1rh-10
- _smp_mflags

* Mon Jun 17 2002 Bill Nottingham <notting@redhat.com> 1.4.1rh-9
- don't forcibly strip binaries

* Wed Apr 17 2002 Bill Nottingham <notting@redhat.com> 1.4.1rh-8
- revert loglevel setting to previous behavior (#63664)

* Tue Mar 12 2002 Bill Nottingham <notting@redhat.com> 1.4.1rh-7
- don't *require* logrotate, but conflict with older versions
- fix fd leak of System.map (#52901)
- switch to -x for klogd by default; we have kksymoops
- provide LSB facility

* Wed Aug 15 2001 Bill Nottingham <notting@redhat.com>
- enable LFS for log files

* Tue Aug 14 2001 Bill Nottingham <notting@redhat.com>
- fix comments in config file (#51678)

* Fri Aug  3 2001 Bill Nottingham <notting@redhat.com>
- require a specific version of logrotate (#50794)
- fix %preun for the case when it's not running (#50123)

* Sun Jul  8 2001 Bill Nottingham <notting@redhat.com>
- merge with 1.4.1

* Wed Feb  7 2001 Bill Nottingham <notting@redhat.com>
- i18n tweaks

* Tue Jan 23 2001 Bill Nottingham <notting@redhat.com>
- new translation stuff

* Fri Jan 19 2001 Bill Nottingham <notting@redhat.com>
- adapt /etc/sysconfig/syslog for specification of arbitrary options (#23171)
- fix translation string slightly (#24088)

* Mon Dec 18 2000 Bill Nottingham <notting@redhat.com>
- don't set owner/group on manpages on install
- read /etc/sysconfig/syslog if present for some configuration paramters
- fix build with new kernel headers

* Tue Dec 12 2000 Bill Nottingham <notting@redhat.com>
- start klogd with '-2'

* Mon Dec 11 2000 Bill Nottingham <notting@redhat.com>
- update to 1.4

* Fri Dec 01 2000 Bill Nottingham <notting@redhat.com>
- rebuild because of broken fileutils

* Mon Oct 13 2000 Bill Nottingham <notting@redhat.com>
- don't log cron in two separate places (#18122)

* Thu Sep 14 2000 Bill Nottingham <notting@redhat.com>
- more fixes from solar@false.com, dan@debian.org; fix the security fix

* Tue Sep 12 2000 Jakub Jelinek <jakub@redhat.com>
- never ever call syslog with a user supplied string as second argument

* Mon Aug  7 2000 Bill Nottingham <notting@redhat.com>
- put cron logs in /var/log/cron; rotate them

* Wed Aug  2 2000 Bill Nottingham <notting@redhat.com>
- start at position 12, not 30 (we *need* to be before nfslock now)

* Fri Jul 28 2000 Bill Nottingham <notting@redhat.com>
- um, how did %preun get tacked onto %post?
- fix condrestart stuff

* Fri Jul 21 2000 Bill Nottingham <notting@redhat.com>
- add a umask call to the initscript

* Fri Jul 14 2000 Bill Nottingham <notting@redhat.com>
- move initscript back

* Fri Jul 14 2000 Florian La Roche <Florian.LaRoche@redhat.com>
- update to 1.3.33

* Wed Jul 12 2000 Prospector <bugzilla@redhat.com>
- automatic rebuild

* Tue Jul 11 2000 Bill Nottingham <notting@redhat.com>
- fix logrotate script (#13698)

* Thu Jul  6 2000 Bill Nottingham <notting@redhat.com>
- prereq /etc/init.d

* Thu Jun 29 2000 Bill Nottingham <notting@redhat.com>
- fix init script

* Tue Jun 27 2000 Bill Nottingham <notting@redhat.com>
- require, not prereq new initscripts

* Mon Jun 26 2000 Bill Nottingham <notting@redhat.com>
- initscript munging

* Wed Jun 14 2000 Nalin Dahyabhai <nalin@redhat.com>
- modify logrotate configuration to use the PID file

* Sun Jun 11 2000 Bill Nottingham <notting@redhat.com>
- rebuild in new environment, FHS fixes

* Mon Mar 27 2000 Bill Nottingham <notting@redhat.com>
- handle bad directories passed to '-a' without behaving strangely (#10363)
- remove compatibility chkconfig links

* Thu Feb  3 2000 Bill Nottingham <notting@redhat.com>
- handle compressed man pages

* Wed Dec  1 1999 Bill Nottingham <notting@redhat.com>
- add patch to fix segfaults in ksym-less cases from HP

* Mon Nov 15 1999 Bill Nottingham <notting@redhat.com>
- fix ECONNRESETs from security patch (olaf)

* Fri Sep 25 1999 Bill Nottingham <notting@redhat.com>
- eek. The security patch broke *two* things...

* Wed Sep 22 1999 Bill Nottingham <notting@redhat.com>
- make klogd actually work.

* Wed Sep  8 1999 Bill Nottingham <notting@redhat.com>
- rotate boot.log

* Tue Sep 07 1999 Cristian Gafton <gafton@redhat.com>
- add patch to fix a possible DoS (thanks Olaf Kirch)

* Mon Aug 16 1999 Bill Nottingham <notting@redhat.com>
- initscript munging

* Mon Aug  9 1999 Bill Nottingham <notting@redhat.com>
- patch to fix non-null terminated stuff in klogd

* Mon Jun 21 1999 Bill Nottingham <notting@redhat.com>
- move (sys|k)logd to /sbin

* Tue Apr 13 1999 Bill Nottingham <notting@redhat.com>
- log boot messages to boot.log
- actually put the sysklogd links in the new place

* Mon Apr 05 1999 Cristian Gafton <gafton@redhat.com>
- disable mark ticks by default

* Thu Apr  1 1999 Bill Nottingham <notting@redhat.com>
- stop klogd/syslogd as late as possible.

* Fri Mar 26 1999 Bill Nottingham <notting@redhat.com>
- twiddle initscript to avoid confusion

* Sun Mar 21 1999 Cristian Gafton <gafton@redhat.com> 
- auto rebuild in the new build environment (release 2)

* Wed Feb 24 1999 Bill Nottingham <notting@redhat.com>
- update to sysklogd-1.3-31
- stop klogd *before* syslogd

* Tue Feb  9 1999 Jeff Johnson <jbj@redhat.com>
- escape naked percent chars in kernel messages (#1088).

* Thu Dec 17 1998 Jeff Johnson <jbj@redhat.com>
- rework last-gasp address-in-module oops trace for both 2.0.x/2.1.x modules.

* Mon Dec  7 1998 Jakub Jelinek <jj@ultra.linux.cz>
- make klogd translate SPARC register dumps and oopses.

* Tue Aug 11 1998 Jeff Johnson <jbj@redhat.com>
- add %clean

* Tue Aug  4 1998 Chris Adams <cadams@ro.com>
- only log to entries that are USER_PROCESS (fix #822)

* Mon Jul 27 1998 Jeff Johnson <jbj@redhat.com>
- remove RPM_BUILD_ROOT from %post

* Wed Apr 29 1998 Cristian Gafton <gafton@redhat.com>
- patch to support Buildroot
- package is now buildrooted

* Wed Apr 29 1998 Michael K. Johnson <johnsonm@redhat.com>
- Added exit patch so that a normal daemon exit is not flagged as an error.

* Mon Apr 27 1998 Prospector System <bugs@redhat.com>
- translations modified for de, fr, tr

* Wed Oct 29 1997 Donnie Barnes <djb@redhat.com>
- added (missingok) to init symlinks

* Thu Oct 23 1997 Donnie Barnes <djb@redhat.com>
- added status|restart support to syslog.init
- added chkconfig support
- various spec file cleanups

* Tue Jun 17 1997 Erik Troan <ewt@redhat.com>
- built against glibc
