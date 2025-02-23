/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "cupsdcomment.h"

#include <tqfile.h>
#include <tqregexp.h>
#include <tdelocale.h>
#include <kstandarddirs.h>

TQString Comment::comment()
{
        TQString str = comment_;
        str.replace(TQRegExp("<[^>]*>"), "");
        str += ("#\n" + example_);
        return str;
}

TQString Comment::toolTip()
{
        TQString str = comment_;
        str.replace(TQRegExp("^#[\\s]*"), "").replace(TQRegExp("\n#[\\s]*"), "\n");
        return i18n("Do not translate the keyword between brackets (e.g. ServerName, ServerAdmin, etc.)", str.utf8());
}

TQString Comment::key()
{
	return key_;
}

bool Comment::load(TQFile *f)
{
        comment_ = "";
        example_ = "";
	key_ = "";
        TQString line, *current = &comment_;
        while (!f->atEnd())
        {
                f->readLine(line, 1024);
                if (line.left(2) == "$$")
                {
                        current = &example_;
                }
		else if (line.left(2) == "%%")
		{
			key_ = line.mid(2).stripWhiteSpace();
		}
                else if (line.left(2) == "@@")
                {
                        return true;
                }
                else if (line.stripWhiteSpace().isEmpty())
                {
                        ; // do nothing
                }
                else
                {
                        if (line[0] != '#') break;
                        else
                        {
                                current->append(line);
                        }
                }
        }
        return false;
}

//------------------------------------------------------------------------------------------------------------

TQString CupsdComment::operator[] (const TQString& key)
{
        return comment(key);
}

TQString CupsdComment::comment(const TQString& key)
{
        if (comments_.count() != 0 || loadComments())
	{
		Comment *comm = comments_.find(key);
		if (comm)
			return comm->comment();
	}
        return TQString::null;
}

TQString CupsdComment::toolTip(const TQString& key)
{
        if (comments_.count() != 0 || loadComments())
	{
		Comment *comm = comments_.find(key);
		if (comm)
			return comm->toolTip();
	}
        return TQString::null;
}

bool CupsdComment::loadComments()
{
        comments_.setAutoDelete(true);
        comments_.clear();
        TQFile	f(locate("data", "tdeprint/cupsd.conf.template"));
	if (f.exists() && f.open(IO_ReadOnly))
	{
                Comment         *comm;
                while (!f.atEnd())
                {
                        comm = new Comment();
                        if (!comm->load(&f))
                                break;
                        else
                        {
				if (comm->key().isEmpty())
					delete comm;
				else
					comments_.insert(comm->key(), comm);
                        }
                }
	}
        return true;
}
