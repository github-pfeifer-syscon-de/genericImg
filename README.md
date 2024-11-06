# genericImg
Some basic functions (some related to imaging) used by my projects.

## Minimum requirements

- a compiler supporting C++20 (preferable Gcc >= 13, others may work as well, check source for "gcc","GNUC" to adapt some functions for some systems)
- Gnu autotools

## Any Linux

For some Debian specific hints see https://github.com/github-pfeifer-syscon-de/genericGlm/README.md .

To build use:
<pre>
autoreconf -fis
./configure --prefix=/usr...
make
</pre>
Using usr is a suggestion, as other locations may require some lib/pkg/-path tweaking
for later steps to find this lib, so use it,
unless you know what your are doing, as always ;)

## Windows

For windows (get msys2 https://www.msys2.org/ ).
Use "msys2 mingw64" window/shell (see tooltip)
First install the prerequisits:
<pre>
pacman -S git
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-autotools
pacman -S mingw-w64-x86_64-gtkmm3
pacman -S mingw-w64-x86_64-libexif
</pre>
Then it should be possible to build the project:
<pre>
autoreconf -fis
./configure --prefix=/mingw64
make
</pre>

## Any systems

The lib requires to be installed before use so (on linux run as root):
<pre>
make install
</pre>

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

## Structure

the structure of the libs used here is:
<pre>
genericImg (used almost everywhere i think)
+ genericGlm
| + monglmm
| + picnic
| + geodata
|   + glglobe
</pre>