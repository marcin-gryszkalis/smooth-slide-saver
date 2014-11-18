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

#include <stdlib.h>
#include <kapplication.h>
#include <klocale.h>
#include <kconfig.h>
#include <qcheckbox.h>
#include <qcolor.h>
#include <kglobal.h>
#include <kurlrequester.h>
#include <knuminput.h>
#include <kurllabel.h>

#include "smoothslidesaver.h"
#include "smoothslidesaverui.h"

//! libkscreensaver interface
extern "C"
{
    const char *kss_applicationName = "smoothslidesaver.kss";
    const char *kss_description = i18n("A KDE Screensaver displaying images in a slideshow with the famous Ken Burns effect");
    const char *kss_version = "0.4.2";

    SmoothSlideSaver *kss_create( WId id )
    {
        KGlobal::locale()->insertCatalogue("smoothslidesaver");
        return new SmoothSlideSaver( id );
    }

    QDialog *kss_setup()
    {
        KGlobal::locale()->insertCatalogue("smoothslidesaver");
        return new SmoothSlideSaverSetup();
    }
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//! dialog to setup screen saver parameters
SmoothSlideSaverSetup::SmoothSlideSaverSetup( QWidget *parent, const char *name )
        : SmoothSlideSaverUI( parent, name, TRUE ), aboutDialog(0)
{
    imagePathRequester->setMode(KFile::Directory);
    
    connect(okayButton, SIGNAL(clicked()), SLOT(slotOkPressed()));
    connect(cancelButton, SIGNAL(clicked()), SLOT(slotCancelPressed()));
    connect(aboutButton, SIGNAL(clicked()), SLOT(slotAboutPressed()));
    
    readSettings();
}


//! read settings from config file
void SmoothSlideSaverSetup::readSettings()
{
    KConfig *config = KGlobal::config();
    config->setGroup("Settings");
    delayInput->setValue(config->readUnsignedNumEntry("Delay", 15));
    imagePathRequester->setURL(config->readPathEntry("ImagePath", "$HOME"));
    useSubDirCheckBox->setChecked(config->readBoolEntry("RecurseIntoSubdirectories", false));
}


//! Ok pressed - save settings and exit
void SmoothSlideSaverSetup::slotOkPressed()
{
    KConfig *config = KGlobal::config();
    
    config->setGroup("Settings");
    config->writeEntry("Delay", delayInput->value());
    config->writeEntry("ImagePath", imagePathRequester->url());
    config->writeEntry("RecurseIntoSubdirectories", useSubDirCheckBox->isChecked());
    config->sync();
    
    accept();
}

void SmoothSlideSaverSetup::slotCancelPressed()
{
    reject();
}

void SmoothSlideSaverSetup::slotAboutPressed()
{
    if (aboutDialog == 0) {
        aboutDialog = new AboutDialog(this);
        connect(aboutDialog->mailtoLink, SIGNAL(leftClickedURL(const QString &)),
                SLOT(slotSendMail(const QString &)));
    }
    aboutDialog->show();
}

void SmoothSlideSaverSetup::slotSendMail(const QString &url) {

    KApplication::kApplication()->invokeMailer(url, QString::null);
}

//-----------------------------------------------------------------------------


SmoothSlideSaver::SmoothSlideSaver( WId id ) : KScreenSaver( id )
{
    ssw = 0;    
    blank();
}

SmoothSlideSaver::~SmoothSlideSaver()
{
    delete ssw;
}


void SmoothSlideSaver::blank()
{
    ssw = new SmoothSlideWidget(0, 0);
    embed(ssw);
    ssw->show();
}
