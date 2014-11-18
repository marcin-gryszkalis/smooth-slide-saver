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

#ifndef IMAGELOADTHREAD_H
#define IMAGELOADTHREAD_H

#include <qimage.h>
#include <qthread.h>
#include <qwaitcondition.h>

/**
@author Carsten Weinhold
*/
class ImageLoadThread : public QThread
{
public:
    ImageLoadThread(const QString &imageDir, bool useSubdirs);
    
    void          quit();
    void          requestNewImage();
    bool          grabImage()   { imageLock.lock(); return haveImages; };
    void          ungrabImage() { imageLock.unlock(); };
    bool          ready()       { return initialized; };
    const QImage &image()       { return texture; };
    float         imageAspect() { return textureAspect; };

protected:
    void run();

    bool loadImage(const QString &path);
    
    QStringList collectImages(const QString &path, bool useSubdirs = true);
    void        shuffleImages(QStringList &files);
    QString     currentImageName();
    void        invalidateCurrentImageName();
    
private:
    QWaitCondition imageRequest;
    QMutex         condLock, imageLock;
    bool           initialized, needImage, haveImages, quitRequested, scanSubdirectories;
    
    float          textureAspect;
    QImage         texture;
    
    QString               imageDirectory;
    QStringList           imageNames;
    QStringList::iterator imageNameIterator;
};

#endif
