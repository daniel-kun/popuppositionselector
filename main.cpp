//*****************************************************************************
// copyright  :    (c) 2009 Daniel Albuschat
//*****************************************************************************

#include "PopupPositionSelector.h"

#include <QApplication>

//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
   QApplication app(argc, argv);

   PopupPositionSelector selector;
   selector.show();

   return app.exec();
}
