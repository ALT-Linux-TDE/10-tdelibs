/***************************************************************************
 *   Markdown Viewer part                                                  *
 *   Copyright (c) 2022 Mavridis Philippe <mavridisf@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/


#include <tqbuffer.h>
#include <tqfile.h>

#include <tdeparts/genericfactory.h>
#include <kstandarddirs.h>

#include <tdehtmlview.h>

/* MD4C-HTML */
#include <md4c-html.h>

#include "markdown_part.h"


typedef KParts::GenericFactory<MarkdownPart> Factory;
K_EXPORT_COMPONENT_FACTORY(libtdemarkdown, Factory)

MarkdownPart::MarkdownPart(TQWidget* parentWidget, const char* widgetName,
                           TQObject* parent, const char* name, const TQStringList& args)
    : TDEHTMLPart(parentWidget, name = "TDEMarkdown")
{
    setInstance(Factory::instance());

    /* Features */
    setJScriptEnabled(false);
    setJavaEnabled(false);
    setMetaRefreshEnabled(false);
    setPluginsEnabled(false);
    setAutoloadImages(true);
    setXMLFile( locate("data", "tdemarkdown/markdown_part.rc") );
}

MarkdownPart::~MarkdownPart()
{
}

TDEAboutData* MarkdownPart::createAboutData()
{
    TDEAboutData* aboutData = new TDEAboutData(
            "tdemarkdown", I18N_NOOP("TDE Markdown Viewer"), "1.0",
            I18N_NOOP("TDEMarkdown is an embeddable viewer for Markdown documents."),
            TDEAboutData::License_GPL_V2, "© 2022 Mavridis Philippe"
    );
    aboutData->addAuthor("Mavridis Philippe (blu.256)", I18N_NOOP("Developer"), "mavridisf@gmail.com");
    return aboutData;
}

bool MarkdownPart::openURL(const KURL& u)
{
    if(u.isLocalFile())
    {
      TQFile local(u.path());

      if(!local.open(IO_ReadOnly))
      {
          return false;
      }

      TQByteArray data = local.readAll();

      local.close();

      if(!data.isNull())
      {
          if (data[data.size()-1] != '\0')
          {
              data.resize(data.size()+1);
              data[data.size()-1] = '\0';
          }
          begin(u);
          TQString parsed(parse((MD_CHAR*) data.data(), u.fileName().utf8().data()));
          write(parsed);
          end();
      }
    }

    emit started(0L);
    return true;
}

TQString& MarkdownPart::parse(MD_CHAR* document, MD_CHAR* title)
{
    m_buffer  = "<!DOCTYPE html>\n";
    m_buffer += "<html>\n";
    m_buffer += "  <head>\n";
    m_buffer += "    <meta charset='utf-8'>\n";
    m_buffer += "    <title>" + (title ? title : i18n("Markdown document")) + "</title>\n";
    m_buffer += "  </head>\n";
    m_buffer += "  <body>\n";

    TQByteArray data;
    int success = md_html(document,
                          MD_SIZE(strlen(document)),
                          &MarkdownPart::processHTML,
                          &data,
                          MD_DIALECT_GITHUB | MD_FLAG_PERMISSIVEURLAUTOLINKS | MD_FLAG_PERMISSIVEEMAILAUTOLINKS | MD_FLAG_PERMISSIVEWWWAUTOLINKS
                              | MD_FLAG_LATEXMATHSPANS | MD_FLAG_PERMISSIVEATXHEADERS | MD_FLAG_UNDERLINE | MD_FLAG_TASKLISTS,
                          0);

    if (success == -1)
    {
        m_buffer += TQString("<b>%1</b>").arg(i18n("Error: malformed document."));
    }
    else
    {
        if (data[data.size()-1] != '\0')
        {
            data.resize(data.size()+1);
            data[data.size()-1] = '\0';
        }
        m_buffer += TQString::fromLocal8Bit(data);
    }

    m_buffer += "  </body>\n";
    m_buffer += "</html>\n";
    return m_buffer;
}

void MarkdownPart::processHTML(const MD_CHAR* data, MD_SIZE data_size, void* user_data)
{
    TQByteArray   *ud = static_cast<TQByteArray*>(user_data);
    TQBuffer buff(*ud);

    if (data_size > 0)
    {
        buff.open(IO_WriteOnly | IO_Append);
        buff.writeBlock(data, (int)data_size);
        buff.close();
    }
}

#include "markdown_part.moc"
