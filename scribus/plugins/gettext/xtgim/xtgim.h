/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
 /***************************************************************************
  *   Copyright (C) 2009 by Jain Basil Aliyas                               *
  *   mail@jainbasil.net                                                    *
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
  *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
  ***************************************************************************/

#ifndef XTGIM_H
#define XTGIM_H

#include "pluginapi.h"
#include "pageitem.h"

#include <QByteArray>
#include <QString>

class StyleManager;
class XtgScanner;

extern "C" PLUGIN_API void GetText2(const QString& filename, const QString& encoding, bool textOnly, bool prefix, bool append, PageItem *textItem);
extern "C" PLUGIN_API QString FileFormatName();
extern "C" PLUGIN_API QStringList FileExtensions();

class XtgIm
{
public:
	XtgIm(PageItem *textItem, bool textOnly, bool prefix, bool append);
	~XtgIm();

	bool importFile(const QString& fileName);

protected:
	XtgScanner* m_scanner { nullptr };
};
#endif			/* XTGIM_H */
