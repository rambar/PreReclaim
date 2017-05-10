Name:      pre-reclaim
Summary:   Memory pre-reclamation for performance optimisation
Version:   0.1
Release:   1
Group:     TO_BE/FILLED_IN
License:   GPL2
Source0:    %{name}-%{version}.tar.gz

BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(json-glib-1.0)

%description
Memory pre-reclamation for performance optimisation

%prep
%setup -q

%build

%cmake
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/bin
cp -a run_test %{buildroot}/usr/bin/

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
/usr/bin/run_test
