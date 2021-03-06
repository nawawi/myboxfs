Name:		squidclam
Version:	0.22
Release:	1
Summary:	Redirect program for squid antivirus scanning
Group:		System Environment/Daemons
License:	GPL
URL:		http://squidclam.sourceforge.net/
Source0:	squidclam-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-root-%(%{__id_u} -n)
BuildRequires:	curl-devel clamav-devel
Requires:	squid curl clamav

%description
squidclam is going to be a program to scan files served to win32
machines by a squid proxy. It hast du be fast and small to get
this job done. Also it has to be secure to not weaken the server
system.
At the moment squidclam is pretty small, scans files for viruses 
and is doing it's job pretty well for me.

%prep
%setup -q


%build
./configure
%{__make} %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT/
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/
mkdir -p $RPM_BUILD_ROOT%{_sbindir}/
install sample.conf $RPM_BUILD_ROOT%{_sysconfdir}/squidclam.conf
install squidclam $RPM_BUILD_ROOT%{_sbindir}/


%clean
rm -rf $RPM_BUILD_ROOT


%post


%preun


%postun


%files
%defattr(0644,root,root,0755)
%doc Changelog README TODO
%doc antivir.php
%config(noreplace) %{_sysconfdir}/squidclam.conf
%attr(0755,root,root) %{_sbindir}/squidclam


%changelog
* Mon May 02 2005 Daniel Lord 0.11
- Changed Version
- Removed the old 100kB Limit (adjustable now)

* Wed Apr 27 2005 Petr Kri�tof <Petr|Kristof_CZ> 0.10-1
- Update to 0.10
- Add /etc/squidclam.conf

* Thu Mar 17 2005 Petr Kri�tof <Petr|Kristof_CZ> 0.9-0
- Initial RPM release

