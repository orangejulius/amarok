/****************************************************************************************
 * Copyright (c) 2007-2009 Maximilian Kossick <maximilian.kossick@googlemail.com>       *
 * Copyright (c) 2008 Peter ZHOU <peterzhoulei@gmail.com>                               *
 * Copyright (c) 2008 Seb Ruiz <ruiz@kde.org>                                           *
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

#ifndef AMAROK_META_FILE_P_H
#define AMAROK_META_FILE_P_H

#include "amarokconfig.h"
#include "core/collections/Collection.h"
#include "core/support/Debug.h"
#include "core/meta/Meta.h"
#include "core/meta/support/MetaUtility.h"
#include "shared/MetaReplayGain.h"
#include "shared/MetaTagLib.h"
#include "core/statistics/StatisticsProvider.h"
#include "core-impl/collections/support/jobs/WriteTagsJob.h"
#include "core-impl/collections/support/ArtistHelper.h"
#include "core-impl/capabilities/AlbumActionsCapability.h"
#include "covermanager/CoverCache.h"

#include <ThreadWeaver/Weaver>

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QWeakPointer>
#include <QSet>
#include <QString>

namespace Capabilities
{
    class LastfmReadLabelCapability;
}

namespace MetaFile
{

//d-pointer implementation

struct MetaData
{
    MetaData()
        : created( 0 )
        , discNumber( 0 )
        , trackNumber( 0 )
        , length( 0 )
        , fileSize( 0 )
        , sampleRate( 0 )
        , bitRate( 0 )
        , year( 0 )
        , bpm( -1.0 )
        , trackGain( 0.0 )
        , trackPeak( 0.0 )
        , albumGain( 0.0 )
        , albumPeak( 0.0 )
        , embeddedImage( false )
    { }
    QString title;
    QString artist;
    QString album;
    QString albumArtist;
    QString comment;
    QString composer;
    QString genre;
    uint created;
    int discNumber;
    int trackNumber;
    qint64 length;
    int fileSize;
    int sampleRate;
    int bitRate;
    int year;
    qreal bpm;
    qreal trackGain;
    qreal trackPeak;
    qreal albumGain;
    qreal albumPeak;
    bool embeddedImage;
};

class Track::Private : public QObject
{
public:
    Private( Track *t )
        : QObject()
        , url()
        , batchUpdate( false )
        , album()
        , artist()
        , albumArtist()
        , provider( 0 )
        , track( t )
    {}

public:
    KUrl url;
    bool batchUpdate;
    Meta::AlbumPtr album;
    Meta::ArtistPtr artist;
    Meta::ArtistPtr albumArtist;
    Meta::GenrePtr genre;
    Meta::ComposerPtr composer;
    Meta::YearPtr year;
    Statistics::StatisticsProvider *provider;
    QWeakPointer<Capabilities::LastfmReadLabelCapability> readLabelCapability;
    QWeakPointer<Collections::Collection> collection;

    Meta::FieldHash changes;

    void writeMetaData()
    {
        DEBUG_BLOCK;
        Meta::Tag::writeTags( url.isLocalFile() ? url.toLocalFile() : url.path(), changes );
        changes.clear();
        readMetaData();
    }

    void notifyObservers()
    {
        track->notifyObservers();
    }

    MetaData m_data;

public slots:
    void readMetaData();

private:
    TagLib::FileRef getFileRef();
    Track *track;
};

void Track::Private::readMetaData()
{
    QFileInfo fi( url.isLocalFile() ? url.toLocalFile() : url.path() );
    m_data.created = fi.created().toTime_t();

    Meta::FieldHash values = Meta::Tag::readTags( fi.absoluteFilePath() );

    if( values.contains(Meta::valTitle) )
        m_data.title = values.value(Meta::valTitle).toString();
    if( values.contains(Meta::valArtist) )
        m_data.artist = values.value(Meta::valArtist).toString();
    if( values.contains(Meta::valAlbum) )
        m_data.album = values.value(Meta::valAlbum).toString();
    if( values.contains(Meta::valAlbumArtist) )
        m_data.albumArtist = values.value(Meta::valAlbumArtist).toString();
    if( values.contains(Meta::valHasCover) )
        m_data.embeddedImage = values.value(Meta::valHasCover).toBool();
    if( values.contains(Meta::valComment) )
        m_data.comment = values.value(Meta::valComment).toString();
    if( values.contains(Meta::valGenre) )
        m_data.genre = values.value(Meta::valGenre).toString();
    if( values.contains(Meta::valYear) )
        m_data.year = values.value(Meta::valYear).toInt();
    if( values.contains(Meta::valDiscNr) )
        m_data.discNumber = values.value(Meta::valDiscNr).toInt();
    if( values.contains(Meta::valTrackNr) )
        m_data.trackNumber = values.value(Meta::valTrackNr).toInt();
    if( values.contains(Meta::valBpm) )
        m_data.bpm = values.value(Meta::valBpm).toReal();
    if( values.contains(Meta::valBitrate) )
        m_data.bitRate = values.value(Meta::valBitrate).toInt();
    if( values.contains(Meta::valLength) )
        m_data.length = values.value(Meta::valLength).toLongLong();
    if( values.contains(Meta::valSamplerate) )
        m_data.sampleRate = values.value(Meta::valSamplerate).toInt();
    if( values.contains(Meta::valFilesize) )
        m_data.fileSize = values.value(Meta::valFilesize).toLongLong();

    if( values.contains(Meta::valTrackGain) )
        m_data.trackGain = values.value(Meta::valTrackGain).toReal();
    if( values.contains(Meta::valTrackGainPeak) )
        m_data.trackPeak= values.value(Meta::valTrackGainPeak).toReal();
    if( values.contains(Meta::valAlbumGain) )
        m_data.albumGain = values.value(Meta::valAlbumGain).toReal();
    if( values.contains(Meta::valAlbumGainPeak) )
        m_data.albumPeak= values.value(Meta::valAlbumGainPeak).toReal();

    if( values.contains(Meta::valComposer) )
        m_data.composer = values.value(Meta::valComposer).toString();

    if( provider )
    {
        if( values.contains(Meta::valRating) )
            provider->setRating( values.value(Meta::valRating).toReal() );
        if( values.contains(Meta::valScore) )
            provider->setScore( values.value(Meta::valScore).toReal() );
        if( values.contains(Meta::valPlaycount) )
            provider->setPlayCount( values.value(Meta::valPlaycount).toReal() );
    }

    if(url.isLocalFile())
    {
        m_data.fileSize = QFile( url.toLocalFile() ).size();
    }
    else
    {
        m_data.fileSize = QFile( url.path() ).size();
    }

    //as a last ditch effort, use the filename as the title if nothing else has been found
    if ( m_data.title.isEmpty() )
    {
        m_data.title = url.fileName();
    }

    // try to guess best album artist (even if non-empty, part of compilation detection)
    m_data.albumArtist = ArtistHelper::bestGuessAlbumArtist( m_data.albumArtist,
        m_data.artist, m_data.genre, m_data.composer );
}

// internal helper classes

class FileArtist : public Meta::Artist
{
public:
    FileArtist( MetaFile::Track::Private *dptr, bool isAlbumArtist = false )
        : Meta::Artist()
        , d( dptr )
        , m_isAlbumArtist( isAlbumArtist )
    {}

    Meta::TrackList tracks()
    {
        return Meta::TrackList();
    }

    QString name() const
    {
        const QString artist = m_isAlbumArtist ? d.data()->m_data.albumArtist
                                               : d.data()->m_data.artist;
        return artist;
    }

    bool operator==( const Meta::Artist &other ) const {
        return name() == other.name();
    }

    QWeakPointer<MetaFile::Track::Private> const d;
    const bool m_isAlbumArtist;
};

class FileAlbum : public Meta::Album
{
public:
    FileAlbum( MetaFile::Track::Private *dptr )
        : Meta::Album()
        , d( dptr )
    {}

    bool hasCapabilityInterface( Capabilities::Capability::Type type ) const
    {
        switch( type )
        {
            case Capabilities::Capability::Actions:
                return true;
            default:
                return false;
        }
    }

    Capabilities::Capability* createCapabilityInterface( Capabilities::Capability::Type type )
    {
        switch( type )
        {
            case Capabilities::Capability::Actions:
                return new Capabilities::AlbumActionsCapability( Meta::AlbumPtr( this ) );
            default:
                return 0;
        }
    }

    bool isCompilation() const
    {
        /* non-compilation albums with no album artists may be hidden in collection
         * browser if certain modes are used, so force compilation in this case */
        return !hasAlbumArtist();
    }

    bool hasAlbumArtist() const
    {
        return !d.data()->albumArtist->name().isEmpty();
    }

    Meta::ArtistPtr albumArtist() const
    {
        /* only return album artist if it would be non-empty, some Amarok parts do not
         * call hasAlbumArtist() prior to calling albumArtist() and it is better to be
         * consistent with other Meta::Track implementations */
        if( hasAlbumArtist() )
            return d.data()->albumArtist;
        return Meta::ArtistPtr();
    }

    Meta::TrackList tracks()
    {
        return Meta::TrackList();
    }

    QString name() const
    {
        if( d )
        {
            const QString albumName = d.data()->m_data.album;
            return albumName;
        }
        else
            return QString();
    }

    bool hasImage( int /* size */ = 0 ) const
    {
        if( d && d.data()->m_data.embeddedImage )
            return true;
        return false;
    }

    QImage image( int size = 0 ) const
    {
        QImage image;
        if( d && d.data()->m_data.embeddedImage )
        {
            image = Meta::Tag::embeddedCover( d.data()->url.toLocalFile() );
        }

        if( image.isNull() || size <= 0 /* do not scale */ )
            return image;
        return image.scaled( size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation );
    }

    bool canUpdateImage() const
    {
        return d; // true if underlying track is not null
    }

    void setImage( const QImage &image )
    {
        if( !d )
            return;

        Meta::FieldHash fields;
        fields.insert( Meta::valImage, image );
        WriteTagsJob *job = new WriteTagsJob( d.data()->url.toLocalFile(), fields );
        QObject::connect( job, SIGNAL(done(ThreadWeaver::Job*)), job, SLOT(deleteLater()) );
        ThreadWeaver::Weaver::instance()->enqueue( job );
        if( d.data()->m_data.embeddedImage == image.isNull() )
            // we need to toggle the embeddedImage switch in this case
            QObject::connect( job, SIGNAL(done(ThreadWeaver::Job*)), d.data(), SLOT(readMetaData()) );

        CoverCache::invalidateAlbum( this );
        notifyObservers();
        // following call calls Track's notifyObservers. This is needed because for example
        // UmsCollection justifiably listens only to Track's metadataChanged() to update
        // its MemoryCollection maps
        d.data()->notifyObservers();
    }

    void removeImage()
    {
        setImage( QImage() );
    }

    bool operator==( const Meta::Album &other ) const {
        return name() == other.name();
    }

    QWeakPointer<MetaFile::Track::Private> const d;
};

class FileGenre : public Meta::Genre
{
public:
    FileGenre( MetaFile::Track::Private *dptr )
        : Meta::Genre()
        , d( dptr )
    {}

    Meta::TrackList tracks()
    {
        return Meta::TrackList();
    }

    QString name() const
    {
        const QString genreName = d.data()->m_data.genre;
        return genreName;
    }

    bool operator==( const Meta::Genre &other ) const {
        return name() == other.name();
    }

    QWeakPointer<MetaFile::Track::Private> const d;
};

class FileComposer : public Meta::Composer
{
public:
    FileComposer( MetaFile::Track::Private *dptr )
        : Meta::Composer()
        , d( dptr )
    {}

    Meta::TrackList tracks()
    {
        return Meta::TrackList();
    }

    QString name() const
    {
        const QString composer = d.data()->m_data.composer;
        return composer;
     }

    bool operator==( const Meta::Composer &other ) const {
        return name() == other.name();
    }

    QWeakPointer<MetaFile::Track::Private> const d;
};

class FileYear : public Meta::Year
{
public:
    FileYear( MetaFile::Track::Private *dptr )
        : Meta::Year()
        , d( dptr )
    {}

    Meta::TrackList tracks()
    {
        return Meta::TrackList();
    }

    QString name() const
    {
        const QString year = QString::number( d.data()->m_data.year );
        return year;
    }

    bool operator==( const Meta::Year &other ) const {
        return name() == other.name();
    }

    QWeakPointer<MetaFile::Track::Private> const d;
};


}

#endif
