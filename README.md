# genericImg
Some basic functions (primarily related to imaging) used by my projects.

To build use any (lin)ux:
<pre>
autoreconf -fis
./configure ...
make
</pre>
For Raspi e.g.:
<pre>
  ./configure --prefix=/usr --with-gles
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
