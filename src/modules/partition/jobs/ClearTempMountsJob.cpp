/* === This file is part of Calamares - <http://github.com/calamares> ===
 *
 *   Copyright 2014-2015, Teo Mrnjavac <teo@kde.org>
 *
 *   Calamares is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Calamares is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Calamares. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ClearTempMountsJob.h"

#include <util/report.h>
#include <utils/Logger.h>

#include <QFile>
#include <QProcess>
#include <QStringList>


ClearTempMountsJob::ClearTempMountsJob()
    : Calamares::Job()
{
}


QString
ClearTempMountsJob::prettyName() const
{
    return tr( "Clear all temporary mounts." );
}


Calamares::JobResult
ClearTempMountsJob::exec()
{
    // Fetch a list of current mounts to Calamares temporary directories.
    QMap< QString, QString > lst;
    QFile mtab( "/etc/mtab" );
    if ( !mtab.open( QFile::ReadOnly | QFile::Text ) )
        return Calamares::JobResult::error( tr( "Cannot get list of temporary mounts." ) );

    while ( !mtab.atEnd() )
    {
        QStringList line = QString::fromLocal8Bit( mtab.readLine() )
                           .split( ' ', QString::SkipEmptyParts );
        QString device = line.at( 0 );
        QString mountPoint = line.at( 1 );
        if ( mountPoint.startsWith( "/tmp/calamares-" ) )
            lst.insert( device, mountPoint );
    }

    QStringList keys = lst.keys();
    keys.sort();

    QStringList goodNews;
    QProcess process;

    for ( int i = keys.length() - 1; i >= 0; --i )
    {
        QString partPath = lst.value( keys[ i ] );
        process.start( "umount", { "-lv", partPath } );
        process.waitForFinished();
        if ( process.exitCode() == 0 )
            goodNews.append( QString( "Successfully unmounted %1." ).arg( partPath ) );

    }

    Calamares::JobResult ok = Calamares::JobResult::ok();
    ok.setMessage( tr( "Cleared all temporary mounts." ) );
    ok.setDetails( goodNews.join( "\n" ) );

    cDebug() << "ClearTempMountsJob finished. Here's what was done:\n" << goodNews.join( "\n" );

    return ok;
}
