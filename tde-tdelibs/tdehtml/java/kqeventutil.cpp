/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Wynn Wilkes <wynnw@caldera.com>
 *               2002 Till Krech <till@snafu.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include <tqevent.h>
#include "kqeventutil.h"

TQString KQEventUtil::getQtEventName( TQEvent* e )
{
    TQString s;

    switch( e->type() )
    {
        case TQEvent::None:
            s = "None";
            break;
        case TQEvent::Timer:
            s = "Timer";
            break;
        case TQEvent::MouseButtonPress:
            s = "MouseButtonPress";
            break;
        case TQEvent::MouseButtonRelease:
            s = "MouseButtonRelease";
            break;
        case TQEvent::MouseButtonDblClick:
            s = "MouseButtonClick";
            break;
        case TQEvent::MouseMove:
            s = "MouseMove";
            break;
        case TQEvent::KeyPress:
            s = "KeyPress";
            break;
        case TQEvent::KeyRelease:
            s = "KeyRelease";
            break;
        case TQEvent::FocusIn:
            s = "FocusIn";
            break;
        case TQEvent::FocusOut:
            s = "FocusOut";
            break;
        case TQEvent::Enter:
            s = "Enter";
            break;
        case TQEvent::Leave:
            s = "Leave";
            break;
        case TQEvent::Paint:
            s = "Paint";
            break;
        case TQEvent::Move:
            s = "Move";
            break;
        case TQEvent::Resize:
            s = "Resize";
            break;
        case TQEvent::Create:
            s = "Create";
            break;
        case TQEvent::Destroy:
            s = "Destroy";
            break;
        case TQEvent::Show:
            s = "Show";
            break;
        case TQEvent::Hide:
            s = "Hide";
            break;
        case TQEvent::Close:
            s = "Close";
            break;
        case TQEvent::Quit:
            s = "Quit";
            break;
        case TQEvent::Reparent:
            s = "Reparent";
            break;
        case TQEvent::ShowMinimized:
            s = "ShowMinimized";
            break;
        case TQEvent::ShowNormal:
            s = "ShowNormal";
            break;
        case TQEvent::WindowActivate:
            s = "WindowActivate";
            break;
        case TQEvent::WindowDeactivate:
            s = "WindowDeactivate";
            break;
        case TQEvent::ShowToParent:
            s = "ShowToParent";
            break;
        case TQEvent::HideToParent:
            s = "HideToParent";
            break;
        case TQEvent::ShowMaximized:
            s = "ShowMaximized";
            break;
        case TQEvent::Accel:
            s = "Accel";
            break;
        case TQEvent::Wheel:
            s = "Wheel";
            break;
        case TQEvent::AccelAvailable:
            s = "AccelAvailable";
            break;
        case TQEvent::CaptionChange:
            s = "CaptionChange";
            break;
        case TQEvent::IconChange:
            s = "IconChange";
            break;
        case TQEvent::ParentFontChange:
            s = "ParentFontChange";
            break;
        case TQEvent::ApplicationFontChange:
            s = "ApplicationFontChange";
            break;
        case TQEvent::ParentPaletteChange:
            s = "ParentPaletteChange";
            break;
        case TQEvent::ApplicationPaletteChange:
            s = "ApplicationPaletteChange";
            break;
        case TQEvent::Clipboard:
            s = "Clipboard";
            break;
        case TQEvent::Speech:
            s = "Speech";
            break;
        case TQEvent::SockAct:
            s = "SockAct";
            break;
        case TQEvent::AccelOverride:
            s = "AccelOverride";
            break;
        case TQEvent::DragEnter:
            s = "DragEnter";
            break;
        case TQEvent::DragMove:
            s = "DragMove";
            break;
        case TQEvent::DragLeave:
            s = "DragLeave";
            break;
        case TQEvent::Drop:
            s = "Drop";
            break;
        case TQEvent::DragResponse:
            s = "DragResponse";
            break;
        case TQEvent::ChildInserted:
            s = "ChildInserted";
            break;
        case TQEvent::ChildRemoved:
            s = "ChildRemoved";
            break;
        case TQEvent::LayoutHint:
            s = "LayoutHint";
            break;
        case TQEvent::ShowWindowRequest:
            s = "ShowWindowRequest";
            break;
        case TQEvent::ActivateControl:
            s = "ActivateControl";
            break;
        case TQEvent::DeactivateControl:
            s = "DeactivateControl";
            break;
        case TQEvent::User:
            s = "User Event";
            break;

        default:
            s = "Undefined Event, value = " + TQString::number( e->type() );
            break;
    }

    return s;
}
