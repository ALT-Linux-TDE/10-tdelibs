#include <tdeapplication.h>
#include <klineeditdlg.h>

#include <tqstring.h>
#include <tqtextview.h>

int main(int argc, char** argv)
{
  TDEApplication app(argc, argv, "klineedittest");
  KLineEditDlg dialog( "_text", "_value", 0L );
  if(dialog.exec())
    {
      tqDebug("Accepted.");
    } else {
      tqDebug("Rejected.");
    }
  return 0;
}

