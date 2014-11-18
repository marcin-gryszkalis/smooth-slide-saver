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

#ifndef SMOOTHSLIDEWIDGET_H
#define SMOOTHSLIDEWIDGET_H

#include <qgl.h>
#include <qtimer.h>

#include "effect.h"
#include "screenproperties.h"

class SmoothSlideSaver;
class ImageLoadThread;

// -------------------------------------------------------------------------

class ViewTrans
{
public:
    ViewTrans(bool zoomIn, float relAspect);
    
    float transX(float pos) const { return baseX + deltaX * pos; };
    float transY(float pos) const { return baseY + deltaY * pos; };    
    float scale (float pos) const { return baseScale * (1.0 + deltaScale * pos); };
    float xScaleCorrect() { return xScale; };
    float yScaleCorrect() { return yScale; };
    
private:
    double rnd() const { return (double)rand() / (double)RAND_MAX; };
    double rndSign() const { return (rand() < RAND_MAX / 2) ? 1.0 : -1.0; };

    // delta and scale values (begin to end) and the needed offsets
    double deltaX, deltaY, deltaScale;
    double baseScale, baseX, baseY;
    float  xScale, yScale;
};

// -------------------------------------------------------------------------

class Image {
public:
    Image(ViewTrans *viewTrans, float aspect = 1.0);
    ~Image();

    ViewTrans *viewTrans;
    float      aspect;
    float      pos;
    float      opacity;
    bool       paint;
    GLuint     texture;    
};

// -------------------------------------------------------------------------

/**
@author Carsten Weinhold
*/
class SmoothSlideWidget : public QGLWidget
{
Q_OBJECT
public:
    SmoothSlideWidget(QWidget *parent = 0, const char *name = 0);

    ~SmoothSlideWidget();

private:
    float aspect() { return (float)width() / (float)height(); };
    bool  setupNewImage(int imageIndex);
    void  startSlideShowOnce();
    void  swapImages();
    void  setNewEffect();
    
    void     initializeGL();
    void     paintGL();
    void     resizeGL(int w, int h);
    void     applyTexture(Image *img, const QImage &image);
    void     paintTexture(Image *img);
    unsigned suggestFrameRate(unsigned forceRate);
        
    void readSettings();
        
private slots:
    void moveSlot();

private:
    SmoothSlideSaver *smoothSlideSaver;    
    ScreenProperties *screen;        
    QTimer           *timer;
    ImageLoadThread  *imageLoadThread;
    bool              haveImages;

    Image  *image[2];
    Effect *effect;
    int     numEffectRepeated;
    bool    zoomIn, initialized;
    float   step;

    // settings from config file
    int      delay;
    QString  imagePath;
    bool     useSubdirs;
    bool     disableFadeInOut;
    bool     disableCrossFade;
    unsigned forceFrameRate;
    
    friend class Effect;
};

#endif
