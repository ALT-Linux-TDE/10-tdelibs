/* This file is part of the KDE libraries
   Copyright (C) 2003 Hamish Rodda <rodda@kde.org>

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

#include "katearbitraryhighlight.h"
#include "katearbitraryhighlight.moc"

#include "katesupercursor.h"
#include "katedocument.h"

#include <kdebug.h>

#include <tqfont.h>

KateArbitraryHighlightRange::KateArbitraryHighlightRange(KateSuperCursor* start,
KateSuperCursor* end, TQObject* parent, const char* name)   :
KateSuperRange(start, end, parent, name) {
}

KateArbitraryHighlightRange::KateArbitraryHighlightRange(KateDocument* doc, const KateRange& range, TQObject* parent, const char* name)
  : KateSuperRange(doc, range, parent, name)
{
}

KateArbitraryHighlightRange::KateArbitraryHighlightRange(KateDocument* doc, const KateTextCursor& start, const KateTextCursor& end, TQObject* parent, const char* name)
  : KateSuperRange(doc, start, end, parent, name)
{
}

KateArbitraryHighlightRange::~KateArbitraryHighlightRange()
{
}

KateArbitraryHighlight::KateArbitraryHighlight(KateDocument* parent, const char* name)
  : TQObject(parent, name)
{
}

KateAttribute KateArbitraryHighlightRange::merge(TQPtrList<KateSuperRange> ranges)
{
  ranges.sort();

  KateAttribute ret;

  if (ranges.first() && ranges.current()->inherits("KateArbitraryHighlightRange"))
    ret = *(static_cast<KateArbitraryHighlightRange*>(ranges.current()));

  KateSuperRange* r;
  while ((r = ranges.next())) {
    if (r->inherits("KateArbitraryHighlightRange")) {
      KateArbitraryHighlightRange* hl = static_cast<KateArbitraryHighlightRange*>(r);
      ret += *hl;
    }
  }

  return ret;
}

void KateArbitraryHighlight::addHighlightToDocument(KateSuperRangeList* list)
{
  m_docHLs.append(list);
  connect(list, TQ_SIGNAL(rangeEliminated(KateSuperRange*)), TQ_SLOT(slotRangeEliminated(KateSuperRange*)));
  connect(list, TQ_SIGNAL(destroyed(TQObject*)),TQ_SLOT(slotRangeListDeleted(TQObject*)));
}

void KateArbitraryHighlight::addHighlightToView(KateSuperRangeList* list, KateView* view)
{
  if (!m_viewHLs[view])
    m_viewHLs.insert(view, new TQPtrList<KateSuperRangeList>());

  m_viewHLs[view]->append(list);

  connect(list, TQ_SIGNAL(rangeEliminated(KateSuperRange*)), TQ_SLOT(slotTagRange(KateSuperRange*)));
  connect(list, TQ_SIGNAL(tagRange(KateSuperRange*)), TQ_SLOT(slotTagRange(KateSuperRange*)));
  connect(list, TQ_SIGNAL(destroyed(TQObject*)),TQ_SLOT(slotRangeListDeleted(TQObject*)));
}

void KateArbitraryHighlight::slotRangeListDeleted(TQObject* obj) {
   int id=m_docHLs.findRef(static_cast<KateSuperRangeList*>(obj));
   if (id>=0) m_docHLs.take(id);
   
   for (TQMap<KateView*, TQPtrList<KateSuperRangeList>* >::Iterator it = m_viewHLs.begin(); it != m_viewHLs.end(); ++it)
    for (KateSuperRangeList* l = (*it)->first(); l; l = (*it)->next())
      if (l==obj) {
        l->take();
        break; //should we check if one list is stored more than once for a view ?? I don't think adding the same list 2 or more times is sane, but who knows (jowenn)
      }
}

KateSuperRangeList& KateArbitraryHighlight::rangesIncluding(uint line, KateView* view)
{
  // OPTIMISE make return value persistent

  static KateSuperRangeList s_return(false);

  Q_ASSERT(!s_return.autoDelete());
  s_return.clear();

  //--- TEMPORARY OPTIMISATION: return the actual range when there are none or one. ---
  if (m_docHLs.count() + m_viewHLs.count() == 0)
    return s_return;
  else if (m_docHLs.count() + m_viewHLs.count() == 1)
    if (m_docHLs.count())
      return *(m_docHLs.first());
    else
      if (m_viewHLs.values().first() && m_viewHLs.values().first()->count() == 1)
        if (m_viewHLs.keys().first() == view && m_viewHLs.values().first())
          return *(m_viewHLs.values().first()->first());
  //--- END Temporary optimisation ---

  if (view) {
    TQPtrList<KateSuperRangeList>* list = m_viewHLs[view];
    if (list)
      for (KateSuperRangeList* l = list->first(); l; l = list->next())
        if (l->count())
          s_return.appendList(l->rangesIncluding(line));

  } else {
    for (TQMap<KateView*, TQPtrList<KateSuperRangeList>* >::Iterator it = m_viewHLs.begin(); it != m_viewHLs.end(); ++it)
      for (KateSuperRangeList* l = (*it)->first(); l; l = (*it)->next())
        if (l->count())
          s_return.appendList(l->rangesIncluding(line));
  }

  for (KateSuperRangeList* l = m_docHLs.first(); l; l = m_docHLs.next())
    if (l->count())
      s_return.appendList(l->rangesIncluding(line));

  return s_return;
}

void KateArbitraryHighlight::slotTagRange(KateSuperRange* range)
{
  emit tagLines(viewForRange(range), range);
}

KateView* KateArbitraryHighlight::viewForRange(KateSuperRange* range)
{
  for (TQMap<KateView*, TQPtrList<KateSuperRangeList>* >::Iterator it = m_viewHLs.begin(); it != m_viewHLs.end(); ++it)
    for (KateSuperRangeList* l = (*it)->first(); l; l = (*it)->next())
      if (l->contains(range))
        return it.key();

  // This must belong to a document-global highlight
  return 0L;
}
