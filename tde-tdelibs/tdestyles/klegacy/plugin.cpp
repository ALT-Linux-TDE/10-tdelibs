#include "klegacystyle.h"
#include <tdelocale.h>

extern "C" {
    TDEStyle* allocate();
    int minor_version();
    int major_version();
    const char *description();
}

TDEStyle* allocate()
{
    return(new KLegacyStyle());
}

int minor_version()
{
    return(0);
}

int major_version()
{
    return(1);
}

const char *description()
{
    return(i18n("TDE LegacyStyle plugin").utf8());
}
