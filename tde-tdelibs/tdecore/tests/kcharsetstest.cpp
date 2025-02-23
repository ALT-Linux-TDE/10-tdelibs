
#include <kcharsets.h>

#include <assert.h>

int main()
{
    TQString input( "&lt;Hello &amp;World&gt;" );
    TQString output = KCharsets::resolveEntities( input );
    assert( output == "<Hello &World>" );
    return 0;
}
