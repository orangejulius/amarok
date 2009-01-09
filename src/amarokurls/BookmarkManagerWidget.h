/***************************************************************************
 *   Copyright (c) 2008  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
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
 
#ifndef BOOKMARKMANAGERWIDGET_H
#define BOOKMARKMANAGERWIDGET_H

#include "amarok_export.h"
#include "BookmarkTreeView.h"

#include <KVBox>

#include <QLineEdit>
#include <QPushButton>


/**
A widget for managing amarok:// bookmark urls

	@author Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>  
*/
class AMAROK_EXPORT BookmarkManagerWidget : public KVBox
{
    Q_OBJECT
public:
    BookmarkManagerWidget( QWidget * parent );
    ~BookmarkManagerWidget();

private:
    
    QString getBookmarkUrl();
    
    QPushButton * m_getCurrentBookmarkButton;
    QPushButton * m_addBookmarkButton;
    QPushButton * m_gotoBookmarkButton;
    QLineEdit * m_currentBookmarkNameEdit;
    QLineEdit * m_currentBookmarkUrlEdit;

    BookmarkTreeView * m_bookmarkView;

private slots:

    void showCurrentUrl();
    void addBookmark();
    void gotoBookmark();
    void bookmarkCurrent();

    void slotBookmarkSelected( AmarokUrl bookmark );

};

#endif
