Name:         input
License:      GPLv2+
Version:      1.3
Release:      1%{?dist}
Summary:      linux input tools
Group:        Applications/System
URL:          http://www.kraxel.org/blog/linux/%{name}/
Source:       http://www.kraxel.org/releases/%{name}/%{name}-%{version}.tar.gz

BuildRequires: gcc

%description
Tools to handle linux input devices (evdev).
List devices, dump input events, ...

%prep
%setup -q

%build
export CFLAGS="%{optflags}"
make prefix=/usr

%install
make prefix=/usr DESTDIR=%{buildroot} STRIP="" install

%files
%doc COPYING
%{_bindir}/*
%{_mandir}/man8/*.8*

%changelog
* Wed Feb 22 2017 Gerd Hoffmann <kraxel@redhat.com> 1.3-1
- new package built with tito

