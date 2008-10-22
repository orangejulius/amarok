/******************************************************************************
 * Copyright (C) 2008 Teo Mrnjavac <teo.mrnjavac@gmail.com>                   *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License as             *
 * published by the Free Software Foundation; either version 2 of             *
 * the License, or (at your option) any later version.                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/
#ifndef TOKEN_H
#define TOKEN_H

#include <QLabel>
#include <QHBoxLayout>


#include "FilenameLayoutWidget.h"

//Defines a part of a filename, drag&droppable in the FilenameLayoutWidget bar from the TokenListWidget list.
class Token
    : public QFrame
{
    Q_OBJECT
    public:
//         explicit Token( QWidget *parent = 0 );
        explicit Token( const QString &string, QWidget *parent = 0 );
        QString getString();
        void setString(const QString &string );
        
    private:
        QHBoxLayout *hlayout;
        unsigned int m_myCount;
        QString m_tokenString;
        QPixmap *m_icon;
        QLabel *m_label;
    protected:
        void resizeEvent( QResizeEvent *event );
};

#endif //TOKEN_H

