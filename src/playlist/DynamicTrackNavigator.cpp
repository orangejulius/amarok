/***************************************************************************
 * copyright            : (C) 2008 Daniel Jones <danielcjones@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#include "DynamicTrackNavigator.h"

#include "Debug.h"
#include "DynamicPlaylist.h"
#include "EngineController.h"
#include "Meta.h"
#include "PlaylistModel.h"


Playlist::DynamicTrackNavigator::DynamicTrackNavigator( Model* m, Dynamic::DynamicPlaylistPtr p )
    : TrackNavigator(m), m_playlist(p)
{
    QObject::connect( m_playlistModel, SIGNAL(activeRowChanged(int,int)),
            this, SLOT(activeRowChanged(int,int)));
    QObject::connect( m_playlistModel, SIGNAL(activeRowExplicitlyChanged(int,int)),
            this, SLOT(activeRowExplicitlyChanged(int,int)));
    QObject::connect( m_playlistModel, SIGNAL(repopulate()),
            this, SLOT(repopulate()) );

    markPlayed();
}


Playlist::DynamicTrackNavigator::~DynamicTrackNavigator()
{
    int activeRow = m_playlistModel->activeRow();
    int lim = qMin( m_playlist->previousCount() + 1, m_playlistModel->rowCount() );
    if( activeRow >= 0 )
        lim = qMin( lim, activeRow );

    for( int i = 0; i < lim; ++i )
    {
        setAsUpcoming( i );
    }
}



int
Playlist::DynamicTrackNavigator::nextRow()
{
    appendUpcoming();

    int activeRow = m_playlistModel->activeRow();
    int updateRow = activeRow + 1;

    setAsPlayed( activeRow );

    if( m_playlistModel->stopAfterMode() == StopAfterCurrent )
        return -1;
    else if( m_playlistModel->rowExists( updateRow ) )
        return updateRow;
    else
    {
        warning() << "DynamicPlaylist is not delivering.";
        return -1;
    }
}

int
Playlist::DynamicTrackNavigator::lastRow()
{
    setAsUpcoming( m_playlistModel->activeRow() );
    int updateRow = m_playlistModel->activeRow() - 1;
    return m_playlistModel->rowExists( updateRow ) ? updateRow : -1;
}

void
Playlist::DynamicTrackNavigator::appendUpcoming()
{
    int updateRow = m_playlistModel->activeRow() + 1;
    int rowCount = m_playlistModel->rowCount();
    int upcomingCountLag = m_playlist->upcomingCount() - (rowCount - updateRow);

    if( upcomingCountLag > 0 )
    {
        Meta::TrackList newUpcoming = m_playlist->getTracks( upcomingCountLag );
        m_playlistModel->insertOptioned( newUpcoming, Append );
    }
}

void
Playlist::DynamicTrackNavigator::removePlayed()
{
    int activeRow = m_playlistModel->activeRow();
    if( activeRow > m_playlist->previousCount() )
    {
        m_playlistModel->removeRows( 0, activeRow - m_playlist->previousCount() );
    }
}

void
Playlist::DynamicTrackNavigator::activeRowChanged( int, int )
{
    removePlayed();
}


void
Playlist::DynamicTrackNavigator::activeRowExplicitlyChanged( int from, int to )
{
    DEBUG_BLOCK
    debug() << "row changed: f,t = " << from << ", " << to;


    while( from > to )
        setAsUpcoming( from-- );
    while( from < to )
        setAsPlayed( from++ );

    removePlayed();
    appendUpcoming();
}

void
Playlist::DynamicTrackNavigator::repopulate()
{
    int start = m_playlistModel->activeRow() + 1;
    if( start < 0 )
        start = 0;
    int span = m_playlistModel->rowCount() - start;

    if( span > 0 )
        m_playlistModel->removeRows( start, span );

    m_playlist->recalculate();
    appendUpcoming();
}

void
Playlist::DynamicTrackNavigator::markPlayed()
{
    int activeRow = m_playlistModel->activeRow();
    if( activeRow < 0 )
        return;

    int lim = qMin( m_playlist->previousCount() + 1, m_playlistModel->rowCount() );
    lim = qMin( lim, activeRow );

    for( int i = 0; i < lim; ++i )
    {
        setAsPlayed(i);
    }
}

void
Playlist::DynamicTrackNavigator::setAsUpcoming( int row )
{
    if( !m_playlistModel->rowExists( row ) )
        return;
    QModelIndex i = m_playlistModel->index( row, 0 );
    m_playlistModel->setData( i, Item::Normal, StateRole );
}

void
Playlist::DynamicTrackNavigator::setAsPlayed( int row )
{
    if( !m_playlistModel->rowExists( row ) )
        return;
    QModelIndex i = m_playlistModel->index( row, 0 );
    m_playlistModel->setData( i, Item::DynamicPlayed, StateRole );
}



