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

#ifndef EFFECT_H
#define EFFECT_H

class Image;
class SmoothSlideWidget;

/**
@author Carsten Weinhold
*/
class Effect {
public:
    typedef enum { Fade, Blend } Type;
    
    Effect(SmoothSlideWidget *parent, bool needFadeIn = true);
    virtual ~Effect();

    virtual void advanceTime(float step) = 0;
    virtual Type type() = 0;
    virtual bool done() = 0;
    virtual bool fadeIn() const { return needFadeIn; };

    static Type chooseEffect(Type oldType);

protected:
    void   setupNewImage(int img);
    void   swapImages();
    Image *image(int img);
            
private:
    SmoothSlideWidget *slideWidget;

protected:
    static int numEffectRepeated;
    bool       needFadeIn;
    Image     *img[2];
};

// -------------------------------------------------------------------------

class FadeEffect: public Effect {
public:
    FadeEffect(SmoothSlideWidget *parent, bool needFadeIn = true);
    virtual ~FadeEffect();

    virtual void advanceTime(float step);
    virtual Type type() { return Fade; };
    virtual bool done();
};

// -------------------------------------------------------------------------

class BlendEffect: public Effect {
public:
    BlendEffect(SmoothSlideWidget *parent, bool needFadeIn = true);
    virtual ~BlendEffect();

    virtual void advanceTime(float step);
    virtual Type type() { return Blend; };
    virtual bool done();
};

#endif
