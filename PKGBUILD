# $Id: pkgbuild-mode.el,v 1.23 2007/10/20 16:02:14 juergen Exp $
# Maintainer:  <opkdx@myhost>
pkgname=trivialfs  
pkgver=0.1
pkgrel=1 
pkgdesc="Filesystem which provides limited support for tagging"
url=""
arch=('i686 x86_64')
license=('GPL')
depends=(fuse)
makedepends=(fuse pkgconfig)
conflicts=()
replaces=()
backup=()
install=
source=(trivialfs.tar.gz)
md5sums=('84625a1c1b453a32e94fa77594a04147')

build() {
  cd $srcdir/trivialfs
  make || return 1
  mkdir -p "$pkgdir/usr/bin"
  make DESTDIR=$pkgdir install || return 1
}
