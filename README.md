# genericImg
Some basic functions (some related to imaging) used by my projects.

## Minimum requirements

- a compiler supporting C++20 (preferable Gcc >= 13, others may work as well, check source for "gcc","GNUC" and the compiler options in Makefile.am)
- Gnu autotools
- The configure step is where needed dependencies will popup
(so look for "checking for ..." and see if the answer is "yes"
if not install additional packages).

## Debian

Use the following commands to get the prerequisits (run with sudo or as root):
<pre>
apt-get install git build-essential automake libtool
apt-get install libgtkmm-3.0-dev
apt-get install libexif-dev
apt-get install libjson-glib-dev
</pre>

### C++20 Support 
If you stick to an older gcc version e.g. &lt; 13
(the major issue is the missing support for std::format) 
you may add: 
<pre>
apt-get install libfmt-dev
</pre>
and check configure.ac for "uncomment to use libfmt...".
The header "psc_format.hpp" provides a switch between these format variants.

## Any Linux

To build use from project dir use 
(the out of tree compile is preferred,
in tree may work as well, but is not tested):
<pre>
autoreconf -fis
mkdir build
cd build
../configure --prefix=/usr
make
</pre>
Using <code>/usr</code> is a suggestion, as other locations may require some lib/pkg/-path tweaking
for later steps to find this lib, so use it, unless you know what you are doing, as always ;)

## Windows

For windows (get msys2 https://www.msys2.org/ ).
Msys2 support multiple prefixes e.g. ucrt64 should be fine
(see https://www.msys2.org/docs/package-naming/).

The names use variables to adapt to your favorite shell.
But once you have selected a flavor you have to stick with it.

First install the prerequisits:
<pre>
pacman -S git
pacman -S base-devel
pacman -S ${MINGW_PACKAGE_PREFIX}-gcc
pacman -S ${MINGW_PACKAGE_PREFIX}-autotools
pacman -S ${MINGW_PACKAGE_PREFIX}-gtkmm3
pacman -S ${MINGW_PACKAGE_PREFIX}-libexif
</pre>
Then it should be possible to clone&build the project:
<pre>
autoreconf -fis
mkdir build
cd build
../configure --prefix=${MINGW_PREFIX}
make 
</pre>
It may save you time when switching enviroments to do a fresh clone,
as some prefixes may be embedded into intermediate files.
I tried to adapt the following readme to use the
the ${MINGW...} environment, but if i missed one, replace
"mingw-w64-x86_64" with ${MINGW_PACKAGE_PREFIX}.

## Any system

The lib requires to be installed before use so (on linux run as root):
<pre>
make install
</pre>
If you don't like it (on linux run as root):
<pre>
make uninstall
</pre>

### Install handling 

The following may apply if you are not expecting to install this once.  
The above method depends on using install and uninstall symmetrically.
To make this clearer say you misspell `./configure --prefix=/use` (that's my favorite)
and `make install` will do whatever it was told and create `/use`.
You realize your mistake and correct the `/use` to `/usr` and install 
into to correct location.
If you are attentive you may do a `make uninstall` for the incorrect location,
if not the wrong files will sit there forever.

Another issue arises if you are a bit lazy leave out the `make uninstall`
on an update and just use `make install`. For most cases that will be fine...
But if with the upgrade a file should have been removed, it will stay in its
location and cause various issues e.g. an include-file in an unexpected location
may ruin your day.

My suggestion is to use a package manager for you operating system 
(at least for linux this should not be too difficult). 
As an example there is a `PKGBUILD` that works for me with Arch-Linux 
(it uses a separate `build` directory, and is unmaintained). 
Requires the extra step to install the package but avoids the issues mentioned above.
As an additional bonus, this will be sensitive to existing files ...  

### Autotools issues

The autotools will not always adapt to version changes.
e.g. when using make you might get a message like
<pre>
"libtool: You should recreate aclocal.m4 with macros from libtool ..."
</pre>
in that case remove the offending intermediate file and
retry to build (in some cases it might help to use autoreconf).

The build-scripts do not adhere to the autotools convention
[wave hand] there is no default optimization.
As I expect some more user like usage,
and tried to make this work right out of the box with gcc/clang
(sorry maintainers, if you feel this is bad idea please make a suggestion).
If your don't like gcc/clang the Makefile.am export statements need some adaption.

## Logging

Now included is some basic logging support.
The default logging will be written to user home into the <code>log</code> directory.

If configured with:
<code>--with-sysdlog</code>
systemd journal will be used as default log.
Query with e.g.:
<code>journalctl SYSLOG_IDENTIFIER="glglobe"</code>
The log is created with extended fields to see these use
<code>-o verbose --output-fields=CODE_FILE,CODE_LINE,CODE_FUNC,MESSAGE</code>.

As a alternative configure option:
<code>--with-syslog</code>
is supported to use syslog  (as it seems out there are some distros that wave with their hand and say "you don't need systemd").

To change the log level the application config file e.g. <code>~/.config/glglobe.conf</code> may support in main section e.g.:
<pre>
logLevel=Info
</pre>
For the Levels see Log.hpp at the moment Severe, Alert, Crit, Error, Warn, Notice, Info, Debug.
But this is a work in progress so there might still be messages spilled on stdout...

### LogView

- if you want to use LogView with syslog, as there is no universal syslog format you may need to adapt the LogViewSyslog::parse method

## Structure

the structure of the libs used here is:
<pre>
genericImg (used almost everywhere i think)
+ genericGlm
| + monglmm
| + picnic
| + geodata
|   + glglobe
+ calcpp
+ nomad
</pre>

