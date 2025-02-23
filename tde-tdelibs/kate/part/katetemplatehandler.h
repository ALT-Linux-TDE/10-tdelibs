/* This file is part of the KDE libraries
   Copyright (C) 2004 Joseph Wenninger <jowenn@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef _KATE_TEMPLATE_HANDLER_H_
#define _KATE_TEMPLATE_HANDLER_H_

#include "katesupercursor.h"
#include "katekeyinterceptorfunctor.h"
#include <tqobject.h>
#include <tqmap.h>
#include <tqdict.h>
#include <tqptrlist.h>
#include <tqstring.h>

class KateDocument;

class KateTemplateHandler: public TQObject, public KateKeyInterceptorFunctor {
		TQ_OBJECT
	public:
		KateTemplateHandler(KateDocument *doc,uint line,uint column, const TQString &templateString, const TQMap<TQString,TQString> &initialValues);
		virtual ~KateTemplateHandler();
		inline bool initOk() {return m_initOk;}
		virtual bool operator()(KKey key);
	private:
		struct KateTemplatePlaceHolder {
			KateSuperRangeList ranges;
			bool isCursor;
			bool isInitialValue;
		};
		class KateTemplateHandlerPlaceHolderInfo{
			public:
				KateTemplateHandlerPlaceHolderInfo():begin(0),len(0),placeholder(""){};
				KateTemplateHandlerPlaceHolderInfo(uint begin_,uint len_,const TQString& placeholder_):begin(begin_),len(len_),placeholder(placeholder_){}
				uint begin;
				uint len;
				TQString placeholder;
		};
		class KateSuperRangeList *m_ranges;
		class KateDocument *m_doc;
		TQPtrList<KateTemplatePlaceHolder> m_tabOrder;
		TQDict<KateTemplatePlaceHolder> m_dict;
		void generateRangeTable(uint insertLine,uint insertCol, const TQString& insertString, const TQValueList<KateTemplateHandlerPlaceHolderInfo> &buildList);
		int m_currentTabStop;
		KateSuperRange *m_currentRange;
		void locateRange(const KateTextCursor &cursor );
		bool m_initOk;
		bool m_recursion;
	private slots:
		void slotTextInserted(int,int);
		void slotDocumentDestroyed();
		void slotAboutToRemoveText(const KateTextRange &range);
		void slotTextRemoved();
};
#endif
