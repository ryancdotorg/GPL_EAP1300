Summary: A collection of programs for manipulating patch files.
Name: patchutils
Version: 0.3.4
Release: 0.1
License: GPL
Group: Applications/System
URL: http://cyberelk.net/tim/patchutils/
Source0: http://cyberelk.net/tim/data/patchutils/stable/%{name}-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot
Obsoletes: interdiff
Provides: interdiff

%description
This is a collection of programs that can manipulate patch files in
useful ways such as interpolating between two pre-patches, combining
two incremental patches, fixing line numbers in hand-edited patches,
and simply listing the files modified by a patch.

%prep
%setup -q

%build
%configure
make
make check

%install
rm -rf %{buildroot}
%makeinstall

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc AUTHORS ChangeLog README COPYING BUGS NEWS
%{_bindir}/*
%{_mandir}/*/*

%changelog
* Tue Jan  6 2004 Tim Waugh <twaugh@redhat.com>
- Ship AUTHORS and ChangeLog as well.

* Wed Sep  3 2003 Tim Waugh <twaugh@redhat.com>
- Remove buildroot before installing.

* Fri May 10 2002 Tim Waugh <twaugh@redhat.com>
- The archive is now distributed in .tar.bz2 format.

* Tue Apr 23 2002 Tim Waugh <twaugh@redhat.com>
- Run tests after build step.

* Mon Mar  4 2002 Tim Waugh <twaugh@redhat.com>
- Obsolete (and provide) interdiff.

* Tue Nov 27 2001 Tim Waugh <twaugh@redhat.com>
- Initial spec file.
