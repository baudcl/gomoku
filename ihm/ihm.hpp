#ifndef _IHM_HPP_
#define _IHM_HPP_

#include <QObject>
#include <QDeclarativeListProperty>

//#include <qdeclarative.h>

#include "ui_ihm.h"
#include "Referee.hpp"

class Game;

class Ihm: public QMainWindow
{
  Q_OBJECT

public:
  Ihm(QWidget *parent = 0);
  virtual ~Ihm();


private:
  void sendPicture(QString pic);
  Referee _ref;
  Ui::Ihm _ui;
  Game*   _game;

signals:
  void emitColor(QString);

public slots:
  void winGame(int);
  void newGame();
};

class CellData : public QObject
{
  Q_OBJECT

  public:
  CellData(QString image = "../images/Empty.png");
  ~CellData();
  Q_PROPERTY(QString source READ source NOTIFY sourceChanged)
  QString source() const { return _source; }
  void source(QString s);

private:
  QString _source;

signals:
  void sourceChanged();
};

class Game : public QObject
{
  Q_OBJECT

  public:
  Game(Referee *ref, int column, bool ia);
  ~Game();

  Q_PROPERTY(QDeclarativeListProperty<CellData> cells READ cells CONSTANT)
  QDeclarativeListProperty<CellData> cells();

  Q_PROPERTY(int cols READ cols NOTIFY colsChanged)
  int cols() const { return _cols; }
  void setRunning(bool b) {_running = b;}
  QString MakePicture(int, int);

signals:
  void  colsChanged();
  void  s_sendHmiText(const QString &msg);

public slots:
  Q_INVOKABLE bool clic(int);
  void addPawl(int);
  void addPawl(int, int);
  void deletePawl(int);
  void dispHmiText(const std::string &);

private:
  bool onBoard( int r, int c ) const { return r >= 0 && r < _cols && c >= 0 && c < _cols; }
  CellData *cell( int row, int col ) { return onBoard(row, col) ? _cells[col + _cols * row] : 0; }
  QList<CellData *> _cells;
  int _cols;
  int _numberBox;
  int _player;
  Referee *_ref;
  bool _running;
  bool _ia;
};

#endif /* _IHM_HPP_ */
