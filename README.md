# genericImg
Some basic functions (some related to imaging) used by my projects.

## Minimum requirements

- a compiler supporting C++20 (preferable Gcc >= 13, others may work as well, check source for "gcc","GNUC" to adapt some functions for some systems)
- Gnu autotools
- the project can be downloaded with
<pre>
git clone https://github.com/github-pfeifer-syscon-de/genericImg
</pre>
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
apt-get install libfmt-dev
</pre>

check configure.ac for "dnl uncomment for use libfmt e.g. with gcc < 13"
and uncomment the following lines (sry coul'd not find a viable switch for these)
the header "psc_format.hpp" provides a switch between format variants,
the adaption may not have been done for all dependent projects ...
(use above header and use any format with psc::fmt:: namespace).

## Any Linux

To build use from project dir:
<pre>
autoreconf -fis
./configure --prefix=/usr...
make
</pre>
Using usr is a suggestion, as other locations may require some lib/pkg/-path tweaking
for later steps to find this lib, so use it,
unless you know what your are doing, as always ;)

The build-scripts adhere to to the auto-make conventions,
this is: [wave hand] there is no default optimization.
So if you are interested in getting the best build for your local machine use:
<pre>
make  CXXFLAGS="-mtune=native -march=native -O3"
</pre>

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
./configure --prefix=${MINGW_PREFIX}
make CXXFLAGS="-mtune=native -march=native -O3"
</pre>
It may save you time when switching enviroments to do a fresh clone,
as some prefixes may be embedded into intermediate files.
(I tried to adaped the following readme to use the
the ${MINGW...} enviroment, but if i missed one, replace
"mingw-w64-x86_64" with ${MINGW_PACKAGE_PREFIX})

## Any system

The lib requires to be installed before use so (on linux run as root):
<pre>
make install
</pre>
If you dont like it (on linux run as root):
<pre>
make uninstall
</pre>

The autotools will not always adapt to version changes.
e.g. when using make you might get a message like
<pre>
"libtool: You should recreate aclocal.m4 with macros from libtool ..."
</pre>
in that case remove the offending intermediate file and
retry to build (in some cases it might help to use autoreconf).

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

- if you wan't to use LogView with syslog, as there is no universal syslog format you may need to adapt the LogViewSyslog::parse method

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

