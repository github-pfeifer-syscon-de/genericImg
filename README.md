# genericImg
Some basic functions (some related to imaging) used by my projects.

To build use any (lin)ux:
<pre>
autoreconf -fis
./configure --prefix=/usr...
make
</pre>
Using usr is a suggestion, as other locations may require some lib/pkg/-path tweaking
for later steps to find this lib, so use it,
unless you know what your are doing, as always ;)

For windows (get msys2 https://www.msys2.org/) the files shoud adapt use this modfied configure
(start "msys2 mingw64" window/shell see tooltip)
<pre>
./configure --prefix=/mingw64
</pre>
The lib requires to be installed before use so (run as root):
<pre>
cd .../genericImg
make install
</pre>

If you run into trouble with the used c++20 change configure.ac AX_CXX_COMPILE_STDCXX([20]... to ...[17] (this may not be an option anymore...)

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

the structure of the libs used here is:
<pre>
genericImg (used almost everywhere i think)
+ genericGlm
| + monglmm
| + picnic
| + geodata
|   + glglobe
</pre>