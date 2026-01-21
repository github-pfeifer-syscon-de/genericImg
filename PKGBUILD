pkgname=genericimg
pkgver=r148.8e67d5d
pkgrel=1
pkgdesc="helping lib"
arch=("x86_64")
url="https://github.com/github-pfeifer-syscon-de/genericImg"
license=('GPL3')
depends=('gtkmm3' )
makedepends=('automake')
provides=()
conflicts=()
replaces=()
options=()
source=('config.status')
sha256sums=('SKIP')

pkgver() {
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

prepare() {
    cd "${startdir}"
#    PKGDEST="$startdir/build/pkg" 
#    autoreconf -fis
}

check() {
    cd "${startdir}"
    make -k check
}


build() {
    cd "${startdir}"
    ../configure --prefix=/usr --with-sysdlog
    make 
}

package() {
    cd "${startdir}"
    make DESTDIR="${pkgdir}/" install
}
