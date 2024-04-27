# genericImg
Some basic functions (primarily related to imaging) used by my projects.

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
If you run into trouble with the used c++20 change configure.ac AX_CXX_COMPILE_STDCXX([20]... to ...[17]...

Now included is some basic logging support.
The default logging will be written to user home into the log directory.

If configured with:
<pre>
--with-sysdlog
</pre>
systemd journal will be used as default log.
Query with e.g.:
<pre>
journalctl SYSLOG_IDENTIFIER="glglobe"
</pre>
To change the log level the application config file e.g. ~/.config/glglobe.conf may support in main section e.g.:
<pre>
logLevel=Info
</pre>
For the Levels see Log.hpp at the moment Error,Warn,Info,Debug,Trace .
But this is a work in progress so there might still be messages spilled on stdout...

the structure of the libs used here is:
<pre>
genericImg (used alomost everywhere i think)
+ genericGlm
| + monglmm
| + picnic
| + geodata
|   + glglobe
</pre>