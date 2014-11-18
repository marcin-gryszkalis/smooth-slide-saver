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

#include <assert.h>
#include <qdatetime.h>
#include "effect.h"
#include "smoothslidewidget.h"

// -------------------------------------------------------------------------

int Effect::numEffectRepeated = 0;

// -------------------------------------------------------------------------

Effect::Effect(SmoothSlideWidget *parent, bool needFadeIn) {

    this->slideWidget = parent;
    this->needFadeIn  = needFadeIn;
}


Effect::~Effect() {

}


void Effect::setupNewImage(int img) {

    slideWidget->setupNewImage(img);
}


void Effect::swapImages() {

    slideWidget->swapImages();
}


Image *Effect::image(int img) {

    assert(img >= 0 && img < 2);
    return slideWidget->image[img];
}


Effect::Type Effect::chooseEffect(Effect::Type oldType) {

    Effect::Type type;

    do {
        type = (rand() < RAND_MAX / 2) ? Effect::Fade : Effect::Blend;        
    } while (type == oldType && numEffectRepeated >= 1);
    
    if (type == oldType)
        numEffectRepeated++;
    else
        numEffectRepeated = 0;
        
   return type;
}



// -------------------------------------------------------------------------

FadeEffect::FadeEffect(SmoothSlideWidget *parent, bool needFadeIn):
    Effect(parent, needFadeIn)
{
    img[0] = image(0);
}


FadeEffect::~FadeEffect() {

}


bool FadeEffect::done() {

    if (img[0]->pos >= 1.0) {
        setupNewImage(0);
        return true;
    }
    
    return false;
}


void FadeEffect::advanceTime(float step) {

    img[0]->pos += step;
    if (img[0]->pos >= 1.0)
        img[0]->pos = 1.0;
        
    if (needFadeIn && img[0]->pos < 0.1)
        img[0]->opacity = img[0]->pos * 10;
    else if (img[0]->pos > 0.9)
        img[0]->opacity = (1.0 - img[0]->pos) * 10;
    else
        img[0]->opacity = 1.0;    
}

// -------------------------------------------------------------------------

BlendEffect::BlendEffect(SmoothSlideWidget *parent, bool needFadeIn):
    Effect(parent, needFadeIn)
{
    img[0] = image(0);
    img[1] = 0;
}


BlendEffect::~BlendEffect() {

}


bool BlendEffect::done() {

    if (img[0]->pos >= 1.0) {
        img[0]->paint = false;
        swapImages();
        return true;
    }
    
    return false;
}


void BlendEffect::advanceTime(float step) {

    img[0]->pos += step;
    if (img[0]->pos >= 1.0)
        img[0]->pos = 1.0;
        
    if (img[1])
        img[1]->pos += step;

    if (needFadeIn && img[0]->pos < 0.1)
        img[0]->opacity = img[0]->pos * 10;
    
    else if (img[0]->pos > 0.9) {
        
        img[0]->opacity = (1.0 - img[0]->pos) * 10;
        
        if (img[1] == 0) {
            setupNewImage(1);
            img[1] = image(1);
            img[1]->opacity = 1.0;
        }
    
    } else
        img[0]->opacity = 1.0;
}

