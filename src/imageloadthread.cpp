/***************************************************************************
 *   Copyright (C) 2005-2006 by Carsten Weinhold                           *
 *   carsten.weinhold@gmx.de                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/***************************************************************************
 *   Parts of this code are based on slideshowgl.{cpp|h} by Renchi Raju    *
 *   <renchi@pooh.tam.uiuc.edu> from the Digikam project                   *
 ***************************************************************************/

extern bool _debugMode;

#include <qdir.h>
#include <qvaluevector.h>
#include <qdeepcopy.h>

#include "smoothslidewidget.h"
#include "imageloadthread.h"

#include "debug.h"

ImageLoadThread::ImageLoadThread(const QString &imageDir, bool useSubdirs) {

    initialized   = false;
    needImage     = true;
    haveImages    = false;
    quitRequested = false;

    imageDirectory     = imageDir;
    scanSubdirectories = useSubdirs;

    DbgLog::log("ImageLoadThread constructed");
}


void ImageLoadThread::quit() {

    QMutexLocker locker(&condLock);
    
    quitRequested = true;
    imageRequest.wakeOne();
}


void ImageLoadThread::requestNewImage() {

    QMutexLocker locker(&condLock);
    
    if ( !needImage) {
        needImage = true;
        imageRequest.wakeOne();
    }
}


void ImageLoadThread::run() {

    DbgLog::log("entered ImageLoadThread::run()");

    // scan directories for images
    imageNames = collectImages(imageDirectory, scanSubdirectories);

    // shuffle images twice (makes it 'more random')
    shuffleImages(imageNames);
    shuffleImages(imageNames);

    imageNameIterator = imageNames.begin();

    QMutexLocker locker(&condLock);

    // we enter the loop with needImage==true, so we will immediatly
    // try to load an image

    while (true) {

        if (quitRequested)
            break;

        if (needImage) {

            needImage = false;
            condLock.unlock();

            bool ok;
            do {
                ok = loadImage(currentImageName());
                if ( !ok)
                    invalidateCurrentImageName();
            } while ( !ok && !imageNames.isEmpty());

            if ( !ok) {
                // generate a black dummy image
                texture = QImage(128, 128, 32);
                texture.fill(Qt::black.rgb());
                DbgLog::log("null image created");
            }

            condLock.lock();

            if ( !initialized) {
                haveImages  = ok;
                initialized = true;
                DbgLog::log(QString("ImageLoadThread initialized: haveImages=")
                            + ((haveImages) ? "true" : "false"));
            }

        } else {
            // wait for new requests from the consumer
            imageRequest.wait(&condLock);
        }
    }

     DbgLog::log("left ImageLoadThread::run()");
}


bool ImageLoadThread::loadImage(const QString &path) {

    QImage image(path);

    if (image.isNull()) {
        return false;
    }

    float aspect  = (float)image.width() / (float)image.height();
    image         = image.smoothScale(1024, 1024);
    
    imageLock.lock();
    
    // this is the critical moment, when we make the new texture and
    // aspect available to the consumer
    textureAspect = aspect;
    texture       = QGLWidget::convertToGLFormat(image);
    
    imageLock.unlock();

    DbgLog::log(QString("loaded image: ") + path);

    return true;
}


QStringList ImageLoadThread::collectImages(const QString &path, bool useSubdirs) {

    QDir        dir(path, 0, QDir::Unsorted);    
    QStringList subDirs = dir.entryList(QDir::Dirs, QDir::Unsorted);    
    
    dir.setNameFilter("*.[pP][nN][gG] *.[jJ][pP][gG] *.[jJ][pP][eE][gG] "
                      "*.[gG][iI][fF] *.[bB][mM][pP] *.[xX][bB][mM] "
                      "*.[pP][nN][mM] *.[mM][nN][gG]");
    QStringList dirFiles = dir.entryList(QDir::Files, QDir::Unsorted);
    QStringList files;
    
    // collect files in directory 'path'
    QStringList::const_iterator j = dirFiles.begin();    
    while (j != dirFiles.end()) {
        files.append(path + "/" + *j);
        ++j;
    }
    
    // collect files from subdirs of 'path'
    QStringList::const_iterator i = subDirs.begin();
    while (useSubdirs && i != subDirs.end()) {
        
        if (*i != "." && *i != "..") {            
            QStringList subDirFiles = collectImages(path + "/" + *i);
    
            j = subDirFiles.begin();    
            while (j != subDirFiles.end()) {
                files.append(*j);
                ++j;
            }
        }
        ++i;
    }

    DbgLog::log(QString("collected ") + QString().setNum(files.count())
                + " images in " + path);

    return files;
}


void ImageLoadThread::shuffleImages(QStringList &images) {

    QStringList           shuffled;
    QValueVector<QString> imageVector;
    
    imageVector.reserve(images.size());
    
    // copy all filenames into a vector for efficient random access
    QStringList::const_iterator i = images.constBegin();
    while (i != images.constEnd()) {
        imageVector.push_back(*i);
        ++i;
    }
    
    size_t lastPos = imageVector.size() - 1;
    while ( !imageVector.isEmpty()) {
        
        // append randomly selected filename to shuffled list
        size_t pos = rand() % imageVector.size();
        shuffled.append(imageVector.at(pos));
        
        // remove filename from list
        imageVector.at(pos) = imageVector.at(lastPos);
        imageVector.pop_back();        
        lastPos--;
    }

    images = shuffled;
}


QString ImageLoadThread::currentImageName() {

    if (imageNameIterator == imageNames.end()) {

        // shuffle image filenames; make sure, the first image after
        // shuffling is different from the last one before (if possible)
        QString oldLastImageName = imageNames.last();
        do
            shuffleImages(imageNames);
        while (imageNames.last() != oldLastImageName &&
               imageNames.first() != imageNames.last());
        
        imageNameIterator = imageNames.begin();
    }

    QString name = *imageNameIterator;
    
    if (imageNameIterator != imageNames.end()) {
        ++imageNameIterator;
        DbgLog::log(QString("image to load: ") + name);
        return name;
    }
    
    return QString::null;
}


void ImageLoadThread::invalidateCurrentImageName() {

    if (imageNameIterator != imageNames.end()) {
        QStringList::iterator del = imageNameIterator;

        DbgLog::log(QString("could not load image ") + *del);
        
        ++imageNameIterator;
        imageNames.remove(del);
    }
}
