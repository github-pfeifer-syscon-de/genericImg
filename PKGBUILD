# use this from build (use "meson setup build" to prepare directory)
pkgname=genericimg
pkgver=r156.1707437
pkgrel=1
pkgdesc="helping lib"
arch=("x86_64")
url="https://github.com/github-pfeifer-syscon-de/genericImg"
license=('GPL3')
depends=('gtkmm3' )
makedepends=('meson')
provides=()
conflicts=()
replaces=()
options=()
source=()   # beware this creates links in src
sha256sums=()

pkgver() {
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

prepare() {
    cd "${startdir}/.."
#    echo "prepare ${PWD} ---------------------------------------------"
    meson setup build -Dprefix=/usr -Dlog=user
}


build() {
    cd "${startdir}"
    echo "build ${PWD} ---------------------------------------------"
    meson compile 
}

check() {
    cd "${startdir}"
    echo "check ${PWD} ---------------------------------------------"
    meson test --print-errorlogs 
}

package() {
    cd "${startdir}"
    echo "package ${PWD} ---------------------------------------------"
    meson install --destdir "$pkgdir"
}
