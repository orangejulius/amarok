/***************************************************************************
 *   Copyright (c) 2008  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
 *             (c) 2008  Jeff Mitchell <kde-dev@emailgoeshere.com>         *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
 
#include "SvgHandler.h"

#include "debug.h"
#include "SvgTinter.h"
#include "TheInstances.h"

#include <KStandardDirs>

#include <QHash>
#include <QPainter>
#include <QPixmapCache>

class SvgHandler::Private
{
    public:
        QHash<QString,QSvgRenderer*> renderers;

        bool loadSvg( const QString& name );
};

class SvgHandlerSingleton
{
    public:
        SvgHandler instance;
};

K_GLOBAL_STATIC( SvgHandlerSingleton, privateInstance )

SvgHandler * SvgHandler::instance()
{
    return &privateInstance->instance;
}


SvgHandler::SvgHandler()
    : d( new Private() )
{
}

SvgHandler::~SvgHandler()
{
    delete d;
}

bool SvgHandler::Private::loadSvg( const QString& name )
{
    QString svgFilename = KStandardDirs::locate( "data", name );
    
    QSvgRenderer *renderer = new QSvgRenderer( The::svgTinter()->tint( svgFilename ).toAscii() );

    if ( ! renderer->isValid() )
    {
        debug() << "Bluddy 'ell guvna, aye canna' load ya Ess Vee Gee at " << svgFilename;
        return false;
    }

    if( renderers[name] )
        delete renderers[name];

    renderers[name] = renderer;
    return true;
}

QSvgRenderer* SvgHandler::getRenderer( const QString& name )
{
    if( ! d->renderers[name] )
        if( ! d->loadSvg( name ) )
            d->renderers[name] = new QSvgRenderer();
    return d->renderers[name];
}

QPixmap SvgHandler::renderSvg( const QString &name, const QString& keyname, int width, int height, const QString& element )
{
    QPixmap pixmap( width, height );
    pixmap.fill( Qt::transparent );

    if( ! d->renderers[name] )
        if( ! d->loadSvg( name ) )
            return pixmap;

    const QString key = QString("%1:%2x%3")
        .arg( keyname )
        .arg( width )
        .arg( height );


    if ( !QPixmapCache::find( key, pixmap ) ) {
//         debug() << QString("svg %1 not in cache...").arg( key );

        QPainter pt( &pixmap );
        if ( element.isEmpty() )
            d->renderers[name]->render( &pt, QRectF( 0, 0, width, height ) );
        else
            d->renderers[name]->render( &pt, element, QRectF( 0, 0, width, height ) );
  
        QPixmapCache::insert( key, pixmap );
    }

    return pixmap;
}

void SvgHandler::reTint( const QString &name )
{
    The::svgTinter()->init();
    d->loadSvg( name );
}

namespace The {
    AMAROK_EXPORT SvgHandler* svgHandler() { return SvgHandler::instance(); }
}

