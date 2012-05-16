#include <QtGui>

#include "Board.hpp"
#include "ihm/ihm.hpp"

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  Ihm ihm;
  
  ihm.show();
  return app.exec();
}
