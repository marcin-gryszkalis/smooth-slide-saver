#! /bin/sh

SVN_DIR=smoothslidesaver
VERSION=`cat debian/changelog | ( read a; echo "$a" | cut -d\( -f2 | cut -d- -f1 )`
REL_DIR="$SVN_DIR-$VERSION"

if [ ! -f "debian/changelog" -o ! -d ".svn" ]
then
    echo "Current directory does not look like a working copy!?"
    exit 1
fi

if [ -d "../$REL_DIR" ]
then
    echo "A previous release seems to exists in '../$REL_DIR'. You need to remove it first."
    exit 1
fi

svn export . "../$REL_DIR"
cd "../$REL_DIR"
make -f Makefile.cvs
dpkg-buildpackage -rfakeroot
