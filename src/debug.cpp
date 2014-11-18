/***************************************************************************
 *   Copyright (C) 2006 by Carsten Weinhold                                *
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
#include <sys/types.h>
#include <unistd.h>

#include <kapplication.h>

#include "debug.h"

DbgLog *DbgLog::instance = 0;


DbgLog::DbgLog() {

    logPrefix = QString().setNum(getpid()) + ": ";
    
    logFile.setName(QString(getenv("HOME")) + "/smoothslidesaver.log");
    logFile.open(IO_WriteOnly | IO_Append);

    logMessage(QString("\n\n====== started new log ======\n"));
}


DbgLog::~DbgLog() {

    logFile.close();
}


void DbgLog::setEnabled(bool enable) {

    if (enable && instance == 0)
        instance = new DbgLog();

    else if (instance) {
        delete instance;
        instance = 0;
    }
}


void DbgLog::logMessage(const QString &msg) {

    logFile.writeBlock(logPrefix, logPrefix.length());
    logFile.writeBlock(msg.latin1(), msg.length());
    logFile.writeBlock("\n", 1);
    logFile.flush();
    
    qDebug("%s", msg.latin1());
}


