Name: literm
Version: 1.3.2
Release: 1
Summary: A terminal emulator with a custom virtual keyboard
Group: System/Base
License: GPLv2
Source0: %{name}-%{version}.tar.gz
URL: https://github.com/rburchell/literm
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Gui)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Quick)
Requires: qt5-qtdeclarative-import-xmllistmodel
Requires: qt5-qtdeclarative-import-window2
Obsoletes: meego-terminal <= 0.2.2
Provides: meego-terminal > 0.2.2

%description
%{summary}.

%files
%defattr(-,root,root,-)
%{_bindir}/*


%prep
%setup -q -n %{name}-%{version}


%build
sed -i 's,/opt/literm/,/usr/,' literm.pro
%qmake_qt5 LITERM_TARGET=desktop
# Inject version number from RPM into source
sed -i -e 's/PROGRAM_VERSION="[^"]*"/PROGRAM_VERSION="%{version}"/g' version.h
make %{?_smp_mflags}


%install
rm -rf %{buildroot}
make INSTALL_ROOT=%{buildroot} install
