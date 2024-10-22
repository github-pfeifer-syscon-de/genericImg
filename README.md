# genericImg
Some basic functions (some related to imaging) used by my projects.

To build use any (lin)ux:
<pre>
autoreconf -fis
./configure --prefix=/usr...
make
</pre>
Using usr is a suggestion, as other locations may require some path tweaking
for later steps to find this lib, so use it,
unless you know what your are doing, as always ;)

For Raspi e.g.:
<pre>
  ./configure --prefix=/usr
</pre>
For windows (get msys2 https://www.msys2.org/) the files shoud adapt use e.g.
(use "msys2 mingw64" window/shell see tooltip)<br>
<pre>
  ./configure --prefix=/mingw64
</pre>
Requires to be installed before use so (run as root):
<pre>
cd .../genericImg
make install
</pre>
If you run into trouble with the used c++20 change configure.ac AX_CXX_COMPILE_STDCXX([20]... to ...[17] (may need some adaptions)

Now included is some basic logging support.
The default logging will be written to user home into the <pre>log</pre> directory.

If configured with:
<pre>
--with-sysdlog
</pre>
systemd journal will be used as default log.
Query with e.g.:
<pre>
journalctl SYSLOG_IDENTIFIER="glglobe"
</pre>
The log is created with extended fields to see these use <pre>-o verbose --output-fields=CODE_FILE,CODE_LINE,CODE_FUNC</ptr>
(a extra constant CODE_LINE is added from where the log function is called).

As a alternative:
<pre>
--with-syslog
</pre>
is supported to use syslog.

To change the log level the application config file e.g. <pre>~/.config/glglobe.conf</pre> may support in main section e.g.:
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