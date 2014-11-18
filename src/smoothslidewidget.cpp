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

#include <assert.h>
#include <math.h>

#include <qimage.h>
#include <qdatetime.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>

#include "smoothslidesaver.h"
#include "smoothslidewidget.h"
#include "imageloadthread.h"

#include "debug.h"


// -------------------------------------------------------------------------

ViewTrans::ViewTrans(bool zoomIn, float relAspect) {

    int i;

    // randomly select sizes of start end end viewport
    double s[2];
    i = 0;
    do {
        s[0]  = 0.3 * rnd() + 1.0;
        s[1]  = 0.3 * rnd() + 1.0;        
    } while (fabs(s[0] - s[1]) < 0.15 && ++i < 10);
   
    if (zoomIn xor s[0] > s[1]) {
        double tmp = s[0];
        s[0]       = s[1];
        s[1]       = tmp;
    }
    
    deltaScale = s[1] / s[0] - 1.0;
    baseScale  = s[0];
    
    // additional scale factors to ensure proper aspect of the displayed image
    double x[2], y[2], xMargin[2], yMargin[2], bestDist;
    double sx, sy;
    if (relAspect > 1.0) {
        sx = 1.0;
        sy = relAspect;
    } else {
        sx = 1.0 / relAspect;
        sy = 1.0;
    }
    xScale = sx;
    yScale = sy;
            
   
    // calculate path
    xMargin[0] = (s[0] * sx - 1.0) / 2.0;
    yMargin[0] = (s[0] * sy - 1.0) / 2.0;
    xMargin[1] = (s[1] * sx - 1.0) / 2.0;
    yMargin[1] = (s[1] * sy - 1.0) / 2.0;
    
    i = 0;
    bestDist = 0.0;
    do {
        double sign = rndSign();
        x[0] = xMargin[0] * (0.2 * rnd() + 0.8) *  sign;
        y[0] = yMargin[0] * (0.2 * rnd() + 0.8) * -sign;
        x[1] = xMargin[1] * (0.2 * rnd() + 0.8) * -sign;
        y[1] = yMargin[1] * (0.2 * rnd() + 0.8) *  sign;
        
        if (fabs(x[1] - x[0]) + fabs(y[1] - y[0]) > bestDist) {
            baseX  = x[0];
            baseY  = y[0];
            deltaX = x[1] - x[0];
            deltaY = y[1] - y[0];
            bestDist = fabs(deltaX) + fabs(deltaY);
            //qDebug("bestDistance=%f", bestDist);
        }
    
    } while (bestDist < 0.3 && ++i < 10);
}

// -------------------------------------------------------------------------

Image::Image(ViewTrans *viewTrans, float aspect) {

    this->viewTrans = viewTrans;
    this->aspect    = aspect;
    this->pos       = 0.0;
    this->opacity   = 0.0;
    this->paint     = (viewTrans) ? true : false;    
    this->texture   = 0;
}

Image::~Image() {

    delete viewTrans;
    if (glIsTexture(texture))
        glDeleteTextures(1, &texture);
}

// -------------------------------------------------------------------------

SmoothSlideWidget::SmoothSlideWidget(QWidget *parent, const char *name)
 : QGLWidget(parent, name, 0, 0)
{
    srand(QTime::currentTime().msec());
    readSettings();

    screen = new ScreenProperties(this);
    screen->enableVSync();

    unsigned frameRate;
    if (forceFrameRate == 0)
        frameRate = screen->suggestFrameRate();
    else
        frameRate = forceFrameRate;

    image[0]    = new Image(0);
    image[1]    = new Image(0);
    effect      = 0;
    step        = 1.0 / ((float) (delay * frameRate));
    zoomIn      = rand() < RAND_MAX / 2;
    initialized = false;
    haveImages  = true;
    
    imageLoadThread = new ImageLoadThread(imagePath, useSubdirs);
    timer           = new QTimer(this);

    connect(timer, SIGNAL(timeout()), this, SLOT(moveSlot()));

    imageLoadThread->start();
    timer->start(1000 / frameRate);

    DbgLog::log("SmoothSlideWidget constructed");
}


SmoothSlideWidget::~SmoothSlideWidget() {

    delete effect;
    delete image[0];
    delete image[1];
    
    imageLoadThread->quit();
    bool terminated = imageLoadThread->wait(10000);

    if ( !terminated) {
        imageLoadThread->terminate();
        terminated = imageLoadThread->wait(3000);
    }

    if (terminated)
        delete imageLoadThread;

    delete timer;
    delete screen;

    DbgLog::log("SmoothSlideWidget destructed");
}


void SmoothSlideWidget::setNewEffect() {

    Effect::Type type;
    bool needFadeIn = (effect == 0 || effect->type() == Effect::Fade);

    // we currently only have two effects
    if (disableFadeInOut)
        type = Effect::Blend;
    else if (disableCrossFade)
        type = Effect::Fade;
    else
        type = Effect::chooseEffect((effect) ? effect->type() : Effect::Fade);
    
    delete effect;
    switch (type) {
        case Effect::Fade:
            effect = new FadeEffect(this, needFadeIn);
            break;
        case Effect::Blend:
            effect = new BlendEffect(this, needFadeIn);
            break;
        default:
            qDebug("Unknown transition effect, falling back to crossfade");
            effect = new BlendEffect(this, needFadeIn);
    }
}


void SmoothSlideWidget::moveSlot() {

    if (initialized) {

        if (effect->done()) {            
            setNewEffect();
            imageLoadThread->requestNewImage();
        }        
        effect->advanceTime(step);
    }

    updateGL();    
}


bool SmoothSlideWidget::setupNewImage(int idx) {

    assert(idx >= 0 && idx < 2);

    if ( !haveImages)
        return false;
        
    bool ok = false;
    zoomIn  = !zoomIn;

    if (imageLoadThread->grabImage()) {

        delete image[idx];

        // we have the image lock and there is an image
        float imageAspect    = imageLoadThread->imageAspect();
        ViewTrans *viewTrans = new ViewTrans(zoomIn, aspect() / imageAspect);
        image[idx]           = new Image(viewTrans, imageAspect);
        
        applyTexture(image[idx], imageLoadThread->image());
        ok = true;

        DbgLog::log("got new image from ImageLoadThread");

    } else {        
        haveImages = false;
        DbgLog::log("ImageLoadThread was unable to provide an image; "
                    "won't try again");
    }

    // don't forget to release the lock on the copy of the image
    // owned by the image loader thread
    imageLoadThread->ungrabImage();            

    return ok;
}


void SmoothSlideWidget::startSlideShowOnce() {

    // when the image loader thread is ready, it will already have loaded
    // the first image
    if (initialized == false && imageLoadThread->ready()) {
    
        setupNewImage(0);                   // setup the first image and
        imageLoadThread->requestNewImage(); // load the next one in background
        setNewEffect();                     // set the initial effect
        
        initialized = true;
    }
}


void SmoothSlideWidget::swapImages() {

    Image *tmp = image[0];
    image[0] = image[1];
    image[1] = tmp;
}


void SmoothSlideWidget::initializeGL() {

    // Enable Texture Mapping
    glEnable(GL_TEXTURE_2D);

    // Clear The Background Color
    glClearColor(0.0, 0.0, 0.0, 1.0f);

    glEnable (GL_TEXTURE_2D);
    glShadeModel (GL_SMOOTH);
    
    // Turn Blending On
    glEnable(GL_BLEND);
    // Blending Function For Translucency Based On Source Alpha Value
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable perspective vision
    glClearDepth(1.0f);
}


void SmoothSlideWidget::paintGL() {

    startSlideShowOnce();

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // only clear the color buffer, if none of the active images is fully opaque
    if ( !((image[0]->paint && image[0]->opacity == 1.0) ||
           (image[1]->paint && image[1]->opacity == 1.0)) )
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
                
    if (image[1]->paint)
        paintTexture(image[1]);
    if (image[0]->paint)
        paintTexture(image[0]);
    
    if ( !haveImages) {
        qglColor(Qt::white);
        renderText(10, 20, i18n("No images found! Please specify a directory containing images."));
        renderText(10, 50, i18n("You can do that using the screensaver configuration module "
                                "of the KDE Control Center."));
    }

    glFlush();
}


void SmoothSlideWidget::resizeGL(int w, int h) {

    glViewport(0, 0, (GLint) w, (GLint) h);
}


void SmoothSlideWidget::applyTexture(Image *img, const QImage &texture) {

    /* create the texture */
    glGenTextures(1, &img->texture);
    glBindTexture(GL_TEXTURE_2D, img->texture);

    /* actually generate the texture */
    glTexImage2D(GL_TEXTURE_2D, 0, 3, texture.width(), texture.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());
    /* enable linear filtering  */
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
}


void SmoothSlideWidget::paintTexture(Image *img) {

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float sx = img->viewTrans->xScaleCorrect();
    float sy = img->viewTrans->yScaleCorrect();
    
    glTranslatef(img->viewTrans->transX(img->pos) * 2.0,
                 img->viewTrans->transY(img->pos) * 2.0, 0.0);
    glScalef(img->viewTrans->scale(img->pos),
             img->viewTrans->scale(img->pos), 0.0);
    
    GLuint& tex = img->texture;

    glBindTexture(GL_TEXTURE_2D, tex);
    glBegin(GL_QUADS);
    {
        glColor4f(1.0, 1.0, 1.0, img->opacity);
        glTexCoord2f(0, 0);
        glVertex3f(-sx, -sy, 0);

        glTexCoord2f(1, 0);
        glVertex3f(sx, -sy, 0);

        glTexCoord2f(1, 1);
        glVertex3f(sx, sy, 0);

        glTexCoord2f(0, 1);
        glVertex3f(-sx, sy, 0);
    }
    glEnd();
}


void SmoothSlideWidget::readSettings() {

    KConfig *config = KGlobal::config();
    
    config->setGroup("Settings");
    delay            = config->readUnsignedNumEntry("Delay", 15);
    imagePath        = config->readPathEntry("ImagePath", "$HOME");
    useSubdirs       = config->readBoolEntry("RecurseIntoSubdirectories", false);
    disableFadeInOut = config->readBoolEntry("DisableFadeInOut", false);
    disableCrossFade = config->readBoolEntry("DisableCrossFade", false);
    forceFrameRate   = config->readUnsignedNumEntry("ForceFrameRate", 0);
    
    DbgLog::setEnabled(config->readBoolEntry("DebugMode", false));
    
    if (delay < 5)  delay = 5;
    if (delay > 20) delay = 20;
    if (forceFrameRate > 120) forceFrameRate = 120;
}

#include "smoothslidewidget.moc"
