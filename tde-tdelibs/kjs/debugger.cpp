/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "debugger.h"
#include "value.h"
#include "object.h"
#include "types.h"
#include "interpreter.h"
#include "internal.h"
#include "ustring.h"

using namespace KJS;

// ------------------------------ Debugger -------------------------------------

namespace KJS {
  struct AttachedInterpreter
  {
  public:
    AttachedInterpreter(Interpreter *i) : interp(i), next(0L) {}
    Interpreter *interp;
    AttachedInterpreter *next;
  };

}

Debugger::Debugger()
{
  rep = new DebuggerImp();
}

Debugger::~Debugger()
{
  // detach from all interpreters
  while (rep->interps)
    detach(rep->interps->interp);

  delete rep;
}

void Debugger::attach(Interpreter *interp)
{
  if (interp->imp()->debugger() != this)
    interp->imp()->setDebugger(this);

  // add to the list of attached interpreters
  if (!rep->interps)
    rep->interps = new AttachedInterpreter(interp);
  else {
    AttachedInterpreter *ai = rep->interps;
    while (ai->next) {
      if (ai->interp == interp)
          return; // already in list
      ai = ai->next;
    }
    ai->next = new AttachedInterpreter(interp);
  }
}

void Debugger::detach(Interpreter *interp)
{
  if (interp->imp()->debugger() == this)
    interp->imp()->setDebugger(0L);

  if (!rep->interps)
    return;
  // remove from the list of attached interpreters
  if (rep->interps->interp == interp) {
    AttachedInterpreter *old = rep->interps;
    rep->interps = rep->interps->next;
    delete old;
  }

  AttachedInterpreter *ai = rep->interps;
  if (!ai)
    return;
  while (ai->next && ai->next->interp != interp)
    ai = ai->next;
  if (ai->next) {
    AttachedInterpreter *old = ai->next;
    ai->next = ai->next->next;
    delete old;
  }
}

bool Debugger::sourceParsed(ExecState * /*exec*/, int /*sourceId*/,
                            const UString &/*source*/, int /*errorLine*/)
{
  return true;
}

bool Debugger::sourceUnused(ExecState * /*exec*/, int /*sourceId*/)
{
  return true;
}

bool Debugger::exception(ExecState * /*exec*/, const Value &/*value*/,
			 bool /*inTryCatch*/)
{
  return true;
}

bool Debugger::atStatement(ExecState * /*exec*/)
{
  return true;
}

bool Debugger::enterContext(ExecState * /*exec*/)
{
  return true;
}

bool Debugger::exitContext(ExecState * /*exec*/, const Completion &/*completion*/)
{
  return true;
}
