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

#ifndef __SmoothSlideSaver_H__
#define __SmoothSlideSaver_H__

#include <kaboutdialog.h>

#include "kscreensaver.h"
#include "smoothslidesaverui.h"
#include "aboutdialog.h"
#include "smoothslidewidget.h"

class SmoothSlideSaver : public KScreenSaver
{
    Q_OBJECT
public:
    SmoothSlideSaver( WId drawable );
    virtual ~SmoothSlideSaver();

private:
    void readSettings();
    void blank();
    
    SmoothSlideWidget *ssw;
};

class SmoothSlideSaverSetup : public SmoothSlideSaverUI
{
    Q_OBJECT
public:
    SmoothSlideSaverSetup( QWidget *parent = NULL, const char *name = NULL );

private slots:
    void slotOkPressed();
    void slotCancelPressed();
    void slotAboutPressed();
    void slotSendMail(const QString &url);

private:
    void readSettings();
    SmoothSlideSaver *saver;
    AboutDialog      *aboutDialog;
};

#endif
