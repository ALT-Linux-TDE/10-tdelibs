/***************************************************************************
 *   Markdown Viewer part                                                  *
 *   Copyright (c) 2022 Mavridis Philippe <mavridisf@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/


#ifndef __MARKDOWN_PART_H
#define __MARKDOWN_PART_H

#include <tqwidget.h>

#include <tdehtml_part.h>

class TDEHTMLPart;

class MarkdownPart : public TDEHTMLPart
{
    TQ_OBJECT

    public:
        MarkdownPart(TQWidget* parentWidget, const char* widgetName,
                     TQObject* parent, const char* name, const TQStringList& args);
        ~MarkdownPart();

        /* Create and return About data */
        static TDEAboutData* createAboutData();

        /* Implemented virtual from TDEHTMLPart */
        bool openURL(const KURL& u);

        /* Parser */
        TQString& parse(MD_CHAR* document, MD_CHAR* title = nullptr);

    private:
        TQString m_buffer;

        static void processHTML(const MD_CHAR* data, MD_SIZE data_size, void* userData);

};


#endif // __MARKDOWN_PART_H
