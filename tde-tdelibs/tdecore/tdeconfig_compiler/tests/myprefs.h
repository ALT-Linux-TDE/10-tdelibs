#include <tdeconfigskeleton.h>

class MyPrefs : public TDEConfigSkeleton
{
  public:
    MyPrefs( const TQString &a ) : TDEConfigSkeleton( a ) {}
};
