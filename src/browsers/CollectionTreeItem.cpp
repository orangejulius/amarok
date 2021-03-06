/****************************************************************************************
 * Copyright (c) 2007 Alexandre Pereira de Oliveira <aleprj@gmail.com>                  *
 * Copyright (c) 2007-2009 Maximilian Kossick <maximilian.kossick@googlemail.com>       *
 * Copyright (c) 2009 Seb Ruiz <ruiz@kde.org>                                           *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#include "CollectionTreeItem.h"
#include "CollectionTreeItemModelBase.h"
#include "core/support/Debug.h"
#include "amarokconfig.h"

#include "core/capabilities/ActionsCapability.h"

#include <KLocale>
#include <KIcon>

Q_DECLARE_METATYPE( QAction* )
Q_DECLARE_METATYPE( QList<QAction*> )

CollectionTreeItem::CollectionTreeItem( CollectionTreeItemModelBase *model )
: m_data( 0 )
    , m_parent( 0 )
    , m_model( model )
    , m_parentCollection( 0 )
    , m_updateRequired( false )
    , m_trackCount( -1 )
    , m_type( Root )
    //, m_name( "Root" )
    , m_isCounting( false )
{
}

CollectionTreeItem::CollectionTreeItem( Meta::DataPtr data, CollectionTreeItem *parent, CollectionTreeItemModelBase *model  )
    : m_data( data )
    , m_parent( parent )
    , m_model( model )
    , m_parentCollection( 0 )
    , m_updateRequired( true )
    , m_trackCount( -1 )
    , m_type( Data )
    //, m_name( data ? data->name() : "NullData" )
    , m_isCounting( false )
{
    if ( m_parent )
        m_parent->appendChild( this );
}

CollectionTreeItem::CollectionTreeItem( Collections::Collection *parentCollection, CollectionTreeItem *parent, CollectionTreeItemModelBase *model  )
    : m_data( 0 )
    , m_parent( parent )
    , m_model( model )
    , m_parentCollection( parentCollection )
    , m_updateRequired( true )
    , m_trackCount( -1 )
    , m_type( Collection )
    //, m_name( parentCollection ? parentCollection->collectionId() : "NullColl" )
    , m_isCounting( false )
{
    if ( m_parent )
        m_parent->appendChild( this );

    connect( parentCollection, SIGNAL( updated() ), SLOT( collectionUpdated() ) );
}

CollectionTreeItem::CollectionTreeItem( Type type, const Meta::DataList &data, CollectionTreeItem *parent, CollectionTreeItemModelBase *model  )
    : m_data( 0 )
    , m_parent( parent )
    , m_model( model )
    , m_parentCollection( 0 )
    , m_updateRequired( false )  //the node already has all children
    , m_trackCount( -1 )
    , m_type( type )
    , m_isCounting( false )
{
    DEBUG_BLOCK
    if( m_parent )
        m_parent->m_childItems.insert( 0, this );

    foreach( Meta::DataPtr datap, data )
        new CollectionTreeItem( datap, this, m_model );
}

CollectionTreeItem::~CollectionTreeItem()
{
    qDeleteAll(m_childItems);
}

void
CollectionTreeItem::appendChild(CollectionTreeItem *child)
{
    m_childItems.append(child);
}

void
CollectionTreeItem::removeChild( int index )
{
    CollectionTreeItem *child = m_childItems[index];
    m_childItems.removeAt( index );
    child->prepareForRemoval();
    child->deleteLater();
}

void
CollectionTreeItem::prepareForRemoval()
{
    m_parent = 0;
    m_model->itemAboutToBeDeleted( this );
    foreach( CollectionTreeItem *item, m_childItems )
    {
        item->prepareForRemoval();
    }
}

CollectionTreeItem*
CollectionTreeItem::child( int row )
{
    return m_childItems.value( row );
}

QVariant
CollectionTreeItem::data( int role ) const
{
    if( isNoLabelItem() )
    {
        switch( role )
        {
        case Qt::DisplayRole:
            return i18nc( "No labels are assigned to the given item are any of its subitems", "No Labels" );
        case Qt::DecorationRole:
            return KIcon( "label-amarok" );
        }
        return QVariant();
    }
    else if( m_parentCollection )
    {
        static const QString counting = i18n( "Counting..." );
        switch( role )
        {
        case Qt::DisplayRole:
        case CustomRoles::FilterRole:
        case CustomRoles::SortRole:
            return m_parentCollection->prettyName();
        case Qt::DecorationRole:
            return m_parentCollection->icon();
        case CustomRoles::ByLineRole:
            if( m_isCounting )
                return counting;
            if( m_trackCount < 0 )
            {
                m_isCounting = true;

                Collections::QueryMaker *qm = m_parentCollection->queryMaker();
                connect( qm, SIGNAL( newResultReady(QStringList) ),
                         SLOT( tracksCounted(QStringList) ) );

                qm->setAutoDelete( true )
                  ->setQueryType( Collections::QueryMaker::Custom )
                  ->addReturnFunction( Collections::QueryMaker::Count, Meta::valUrl )
                  ->run();

                return counting;
            }
            return i18np( "1 track", "%1 tracks", m_trackCount );
        case CustomRoles::HasCapacityRole:
            return m_parentCollection->hasCapacity();
        case CustomRoles::UsedCapacityRole:
            return m_parentCollection->usedCapacity();
        case CustomRoles::TotalCapacityRole:
            return m_parentCollection->totalCapacity();
        case CustomRoles::DecoratorRoleCount:
            return decoratorActions().size();
        case CustomRoles::DecoratorRole:
            QVariant v;
            v.setValue( decoratorActions() );
            return v;
        }
    }
    return QVariant();
}

QList<QAction*>
CollectionTreeItem::decoratorActions() const
{
    QList<QAction*> decoratorActions;
    QScopedPointer<Capabilities::ActionsCapability> dc( m_parentCollection->create<Capabilities::ActionsCapability>() );
    if( dc )
        decoratorActions = dc->actions();
    return decoratorActions;
}

void
CollectionTreeItem::tracksCounted( QStringList res )
{
    if( !res.isEmpty() )
        m_trackCount = res.first().toInt();
    else
        m_trackCount = 0;
    m_isCounting = false;
    emit dataUpdated();
}

void
CollectionTreeItem::collectionUpdated()
{
    m_trackCount = -1;
}

int
CollectionTreeItem::row() const
{
    if( m_parent )
    {
        const QList<CollectionTreeItem*> &children = m_parent->m_childItems;
        if( !children.isEmpty() && children.contains( const_cast<CollectionTreeItem*>(this) ) )
            return children.indexOf( const_cast<CollectionTreeItem*>(this) );
        else
            return -1;
    }
    return 0;
}

int
CollectionTreeItem::level() const
{
    if( m_parent )
        return m_parent->level() + 1;
    return -1;
}

bool
CollectionTreeItem::isDataItem() const
{
    return m_type == Data;
}

bool
CollectionTreeItem::isVariousArtistItem() const
{
    return m_type == CollectionTreeItem::VariousArtist;
}

bool
CollectionTreeItem::isNoLabelItem() const
{
    return m_type == CollectionTreeItem::NoLabel;
}

bool
CollectionTreeItem::isAlbumItem() const
{
    return m_type == Data && !Meta::AlbumPtr::dynamicCast( m_data ).isNull();
}

bool
CollectionTreeItem::isTrackItem() const
{
    return m_type == Data && !Meta::TrackPtr::dynamicCast( m_data ).isNull();
}

Collections::QueryMaker*
CollectionTreeItem::queryMaker() const
{
    Collections::Collection* coll = parentCollection();
    if( coll )
        return coll->queryMaker();
    return 0;
}

void
CollectionTreeItem::addMatch( Collections::QueryMaker *qm, CategoryId::CatMenuId levelCategory ) const
{
    if( !qm )
        return;

    if( isVariousArtistItem() )
        qm->setAlbumQueryMode( Collections::QueryMaker::OnlyCompilations );
    if( isNoLabelItem() )
        qm->setLabelQueryMode( Collections::QueryMaker::OnlyWithoutLabels );
    else if( Meta::TrackPtr track = Meta::TrackPtr::dynamicCast( m_data ) )
        qm->addMatch( track );
    else if( Meta::ArtistPtr artist = Meta::ArtistPtr::dynamicCast( m_data ) )
    {
        qm->addMatch( artist );
        if( levelCategory == CategoryId::AlbumArtist )
            qm->setArtistQueryMode( Collections::QueryMaker::AlbumArtists );
    }
    else if( Meta::AlbumPtr album = Meta::AlbumPtr::dynamicCast( m_data ) )
        qm->addMatch( album );
    else if( Meta::ComposerPtr composer = Meta::ComposerPtr::dynamicCast( m_data ) )
        qm->addMatch( composer );
    else if( Meta::GenrePtr genre = Meta::GenrePtr::dynamicCast( m_data ) )
        qm->addMatch( genre );
    else if( Meta::YearPtr year = Meta::YearPtr::dynamicCast( m_data ) )
        qm->addMatch( year );
    else if( Meta::LabelPtr label = Meta::LabelPtr::dynamicCast( m_data ) )
        qm->addMatch( label );
}


KUrl::List
CollectionTreeItem::urls() const
{
    /*QueryBuilder qb = queryBuilder();
    qb.addReturnValue( QueryBuilder::tabSong, QueryBuilder::valURL );
    QStringList values = qb.run();
    KUrl::List list;
    foreach( QString s, values ) {
        list += KUrl( s );
    }
    return list;*/
    KUrl::List list;
    return list;
}

bool
CollectionTreeItem::operator<( const CollectionTreeItem& other ) const
{
    if( isVariousArtistItem() )
        return true;
    return m_data->sortableName() < other.m_data->sortableName();
}

QList<Meta::TrackPtr>
CollectionTreeItem::descendentTracks()
{
    QList<Meta::TrackPtr> descendentTracks;
    Meta::TrackPtr track;
    if( isDataItem() )
        track = Meta::TrackPtr::dynamicCast( m_data );

    if( !track.isNull() )
        descendentTracks << track;
    else
    {
        foreach( CollectionTreeItem *child, m_childItems )
            descendentTracks << child->descendentTracks();
    }
    return descendentTracks;
}

bool
CollectionTreeItem::allDescendentTracksLoaded() const
{
    if( isTrackItem() )
        return true;

    if( requiresUpdate() )
        return false;

    foreach( CollectionTreeItem *item, m_childItems )
    {
        if( !item->allDescendentTracksLoaded() )
            return false;
    }
    return true;
}

void
CollectionTreeItem::setRequiresUpdate( bool updateRequired )
{
    m_updateRequired = updateRequired;
}

bool
CollectionTreeItem::requiresUpdate() const
{
    return m_updateRequired;
}

CollectionTreeItem::Type
CollectionTreeItem::type() const
{
    return m_type;
}

QList<CollectionTreeItem*>
CollectionTreeItem::children() const
{
    return m_childItems;
}

#include "CollectionTreeItem.moc"

