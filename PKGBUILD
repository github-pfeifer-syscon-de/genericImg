pkgname=genericimg
pkgver=r150.5a0c704
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
source=('configure.ac')
sha256sums=('SKIP')

pkgver() {
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

prepare() {
    cd "${startdir}"
    echo "prepare ${PWD} ---------------------------------------------"
    autoreconf -fis
    mkdir -p build
}


build() {
    cd "${startdir}"/build
    echo "build ${PWD} ---------------------------------------------"
    ../configure --prefix=/usr --with-sysdlog
    make 
}

check() {
    cd "${startdir}"/build
    echo "check ${PWD} ---------------------------------------------"
    make -k check
}

package() {
    cd "${startdir}"/build
    echo "package ${PWD} ---------------------------------------------"
    make DESTDIR="${pkgdir}/" install
}
