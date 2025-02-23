#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>

#include <VCard.h>

int main(int argc,char **argv)
{
  TDEAboutData aboutData("testwrite",I18N_NOOP("TestWritevCard"),"0.1");
  TDECmdLineArgs::init(argc,argv,&aboutData);

  TDEApplication app;
  
  kdDebug() << "Test Write VCard" << endl;

  using namespace VCARD;
  
  VCard v;

  ContentLine cl1;
  cl1.setName(EntityTypeToParamName(EntityName));
  cl1.setValue(new TextValue("Hans Wurst"));
  v.add(cl1);

  ContentLine cl2;
  cl2.setName(EntityTypeToParamName(EntityTelephone));
  cl2.setValue(new TelValue("12345"));
  ParamList p;
  p.append( new TelParam("home") );
  p.append( new TelParam("fax") );
  cl2.setParamList( p );
  v.add(cl2);

  TQCString str = v.asString();

  kdDebug() << "--- VCard begin ---" << endl
            << str
            << "--- VCard  end  ---" << endl;
}
