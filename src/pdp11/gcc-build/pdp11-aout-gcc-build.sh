#!/bin/bash

#
# Configure the directories here
#

# where the build process is going to happen
BUILDROOT=~/tmpsrc/pdp11gcc

# destination prefix
PREFIX=/usr/local/pdp11

# path to extra libs, needed for correct linking with iconv on some systems
EXTRALIBS=-L/usr/lib

# prefix for gmp, mpfr and mpc
GMP=/opt/local

# prefix for system iconv 
ICONV=/usr

#NEWLIBURL=ftp://sources.redhat.com/pub/newlib/newlib-1.18.0.tar.gz
NEWLIBURL=http://nativeclient.googlecode.com/svn-history/r1508/trunk/src/third_party/newlib/newlib-1.18.0.tar.gz

# The following may be useful if you have to resume after a failure at some point
# Mind the stuff that accumulates in $BUILDROOT/build directory,
# the script never deletes anything by itself.

# Uses port install, see text below
INSTALL_GMP=YES

BUILD_BINUTILS=YES
DOWNLOAD_NEWLIB=YES
BUILD_PHASE1=YES
BUILD_NEWLIB=YES

#
# Configurable stuff ends here
#

function echo3() {
    echo
    echo $1
    echo
}

function gruu() {
    echo
    echo "ERROR in \`$1'"
    echo 
    exit 127
}

cd $BUILDROOT && BUILDROOT=`pwd`

echo "This script will try to build GCC for pdp11-aout target"
echo "Directory for source and intermediate files: $BUILDROOT"
echo "Installation prefix: $PREFIX"
echo 
echo "It may be required that you enter your password for sudo"
echo "between build phases"
echo
echo "Press Enter when ready"
read

mkdir -p $BUILDROOT
cd $BUILDROOT
if [ -n "$(ls)" ] ; then
    echo "The build directory is not empty. It is recommended that you"
    echo "start in a clean location."
    echo
    echo "Press Enter to continue..."
    read 
fi

if [ "$INSTALL_GMP" == "YES" ] ; then
    echo3 "Installing GCC building prerequisites"
    sudo port install -b gmp mpfr libmpc
fi

if [ "$DOWNLOAD_NEWLIB" == "YES" ] ; then
    echo3 "Downloading newlib and patches"
    wget -c $NEWLIBURL || gruu "newlib"
    wget -c http://sensi.org/~svo/pdp11/newlib-pdp11.tar.gz || gruu "newlib-pdp11"
    wget -c http://sensi.org/~svo/pdp11/newlib-pdp11.diff || gruu "newlib-pdp11.diff"
    wget -c http://sensi.org/~svo/pdp11/gcc-configure-pdp11.diff || gruu "gcc-configure-pdp11.diff"
fi

if [ "$BUILD_BINUTILS" == "YES" ] ; then
    echo3 "Logging in to sourceware.org CVS"
    #cvs -z 9 -d :pserver:anoncvs@sourceware.org:/cvs/src login
    echo '/1 :pserver:anoncvs@sourceware.org:2401/cvs/src Ay=0=h<Z' >> ~/.cvspass
    echo3 "Checking out binutils"
    cvs -z 9 -d :pserver:anoncvs@sourceware.org:/cvs/src co binutils || gruu "cvs co binutils"

    mkdir -p build/binutils && cd build/binutils
    ../../src/configure --target=pdp11-aout --prefix=$PREFIX --disable-nls --disable-werror || gruu "configure binutils"
    make && sudo make install || gruu "build binutils"

    echo3 "Successfully built and installed binutils"
fi

PATH=$PATH:$PREFIX/bin

cd $BUILDROOT

if [ -d gcc ] ; then
    echo3 "gcc appears to be checked out already, delete $BUILDROOT/gcc dir if not"
else
    echo3 "Checking out gcc source"
    svn checkout svn://gcc.gnu.org/svn/gcc/trunk gcc || gruu "checkout gcc"
fi

echo3 "Applying configure patch"
cd gcc && svn revert configure  || gruu "svn revert gcc-configure"
cd ..
patch -p0 < gcc-configure-pdp11.diff || gruu "patch gcc-configure"
mkdir -p build/gcc

if [ -d newlib-1.18.0 ] ; then
    echo3 "newlib appears to be prepared already"
else
    echo3 "Unpacking and patching newlib-1.18.0"
    tar zxvf newlib-1.18.0.tar.gz || gruu "untar newlib"
    patch -p0 < newlib-pdp11.diff || gruu "patch newlib"
    tar zxvf newlib-pdp11.tar.gz  || gruu "untar newlib-pdp11"
fi


# PHASE 1

if [ "$BUILD_PHASE1" == "YES" ] ; then
    echo3 "Configuring gcc first stage"
    cd $BUILDROOT/gcc/
    LDFLAGS=$EXTRALIBS ../../gcc/configure --target=pdp11-aout --prefix=$PREFIX --disable-nls --disable-werror --with-gmp=$GMP --with-iconv=$ICONV --enable-languages=c --disable-libssp --disable-multilib --disable-shared || gruu "phase 1 configure"
    echo3 "Building gcc first stage"
    make all-gcc && sudo make install-gcc || gruu "phase 1 make or install"
fi 

if [ "$BUILD_NEWLIB" == "YES" ] ; then
    echo3 "Configuring newlib"
    # NEWLIB
    cd $BUILDROOT/build && mkdir -p newlib && cd newlib
    ../../newlib-1.18.0/configure --target=pdp11-aout --prefix=$PREFIX --disable-multilib --disable-nls || gruu "configure newlib"
    make && sudo make install || gruu "build newlib"
fi

echo3 "Conifiguring gcc second stage"
cd $BUILDROOT/build && mkdir -p gcc-s2 && cd gcc-s2
LDFLAGS=-L/usr/lib  ../../gcc/configure --target=pdp11-aout --prefix=$PREFIX --disable-nls --disable-werror --with-newlib --with-gmp=$GMP --with-iconv=$ICONV --enable-languages=c --disable-libssp --disable-multilib --disable-shared || gruu "phase 2 configure"
make && sudo make install || gruu "phase 2 make or install"

echo3 "Hoorj!"

