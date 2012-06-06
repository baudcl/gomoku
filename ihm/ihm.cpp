#include <QObject>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeView>
#include <QTextCodec>
#include <QMessageBox>

#include "ihm.hpp"
#include <sstream>

//#include <QDebug>
#include "../board/Board.hpp"
#include <iostream>

void cellsPropAppend(QDeclarativeListProperty<CellData>* prop, CellData* value)
{
  Q_UNUSED(prop);
  Q_UNUSED(value);
  return; //Append not supported
}

int cellsPropCount(QDeclarativeListProperty<CellData>* prop)
{
  return static_cast<QList<CellData*>*>(prop->data)->count();
}

CellData* cellsPropAt(QDeclarativeListProperty<CellData>* prop, int index)
{
  return static_cast<QList<CellData*>*>(prop->data)->at(index);
}


Ihm::Ihm(QWidget *parent)
  : QMainWindow(parent)
{

  _ui.setupUi(this);

  qmlRegisterType<CellData>();

  QMessageBox msgBox;
  QPushButton *iaButton = msgBox.addButton(tr("Player VS IA"), QMessageBox::AcceptRole);
  QPushButton *twoPlayersButton = msgBox.addButton(tr("Player VS Player"), QMessageBox::AcceptRole);

  msgBox.setText("Game type");

  msgBox.exec();

 QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
  if (msgBox.clickedButton() == (QAbstractButton*)iaButton) {
    _game = new Game(&_ref, 19, true);
  } else if (msgBox.clickedButton() == (QAbstractButton*)twoPlayersButton) {
    _game = new Game(&_ref, 19, false);
  }

  connect(_ui.actionDouble, SIGNAL(toggled(bool)), &_ref, SLOT(setDoubleThreeRule(bool)));
  connect(_ui.actionNewGame, SIGNAL(triggered()), &_ref, SLOT(reset()));
  connect(_ui.actionNewGame, SIGNAL(triggered()), this, SLOT(newGame()));
  connect(_ui.action10ms, SIGNAL(triggered()), &_ref, SLOT(iaTime10()));
  connect(_ui.action20ms, SIGNAL(triggered()), &_ref, SLOT(iaTime20()));
  connect(_ui.action50ms, SIGNAL(triggered()), &_ref, SLOT(iaTime50()));
  connect(_ui.actionEating, SIGNAL(toggled(bool)), &_ref, SLOT(setEatingAtEndForWin(bool)));
  connect(&_ref, SIGNAL(s_notifyVictory(int)), this, SLOT(winGame(int)));
  connect(_ui.actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
  connect(&_ref, SIGNAL(s_sendHmiText(const QString &)), _ui.textEdit, SLOT(append(const QString &)));
  connect(_game, SIGNAL(s_sendHmiText(const QString &)), _ui.textEdit, SLOT(append(const QString &)));
  _ui.canvas->engine()->rootContext()->setContextObject(_game);

  _ui.canvas->setSource(QString("qrc:Gomoku.qml"));
}


void Ihm::winGame(int p)
{
    p = p;
    _game->setRunning(false);
    _ui.canvas->setEnabled(false);
}

void Ihm::newGame()
{
  _ui.canvas->setEnabled(true);
}

void Ihm::sendPicture(QString pic)
{
  emit emitColor(pic);
}

Ihm::~Ihm()
{
    if (_game)
        delete _game;
}

CellData::CellData(QString image)
{
  _source = image;
}

CellData::~CellData() {}

void CellData::source(QString s)
{
  _source = s;
  emit sourceChanged();
}

QString Game::MakePicture(int ii, int color)
{
  QString src;
  //../images/
  if (ii == 0)
    src = "TopLeft.png";
  else if (ii == _cols - 1)
    src = "TopRight.png";
  else if (ii > 0 && ii < _cols - 1)
    src = "Top.png";
  else if (ii == _numberBox - _cols)
    src = "BottomLeft.png";
  else if (ii == _cols * _cols - 1)
    src = "BottomRight.png";
  else if (ii > (_numberBox - _cols - 1) && ii < (_numberBox - 1))
    src = "Bottom.png";
  else if (ii % _cols == 0)
    src = "Left.png";
  else if (ii % _cols == _cols - 1)
    src = "Right.png";
  else
    src = ".png";

  if (color == WHITE)
    src = "../images/White" + src;
  else if (color == BLACK)
    src = "../images/Black" + src;
  else
    src = "../images/Empty" + src;

  return src;
}




Game::Game(Referee *ref, int column, bool ia) : _cols(column), _numberBox(_cols * _cols), _player(BLACK), _ref(ref), _ia(ia)
{
   std::ostringstream hmiText;

  for(int ii = 0; ii < _numberBox; ++ii)
    {
      if (ii == 0)
	_cells << new CellData("../images/EmptyTopLeft.png");
      else if (ii == _cols - 1)
	_cells << new CellData("../images/EmptyTopRight.png");
      else if (ii > 0 && ii < _cols - 1)
	_cells << new CellData("../images/EmptyTop.png");
      else if (ii == _numberBox - _cols)
	_cells << new CellData("../images/EmptyBottomLeft.png");
      else if (ii == _cols * _cols - 1)
	_cells << new CellData("../images/EmptyBottomRight.png");
      else if (ii > (_numberBox - _cols - 1) && ii < (_numberBox - 1))
	_cells << new CellData("../images/EmptyBottom.png");
      else if (ii % _cols == 0)
	_cells << new CellData("../images/EmptyLeft.png");
      else if (ii % _cols == _cols - 1)
	_cells << new CellData("../images/EmptyRight.png");
      else
	_cells << new CellData;

    }

    connect(this->_ref, SIGNAL(s_newPawl(int, int)), this, SLOT(addPawl(int, int)));
    connect(this->_ref, SIGNAL(s_deletePawl(int)), this, SLOT(deletePawl(int)));
    connect(this->_ref, SIGNAL(iaPlay()), this, SLOT(iaCanPlay()));
    //    connect(this->_ref, SIGNAL(s_sendHmiText(const std::string &)), this, SLOT(dispHmiText(const std::string &)));

    hmiText << "Au tours du joueur " << _player << " de jouer";
    QString s = QString::fromStdString(hmiText.str());
    emit s_sendHmiText(s);
  //this->dispHmiText(hmiText.str());
}

Game::~Game()
{
    for (unsigned int i = 0; i < _cells.size(); ++i)
    {
        delete _cells[i];
    }
}


 QDeclarativeListProperty<CellData> Game::cells(){
   return QDeclarativeListProperty<CellData>(this, &_cells, &cellsPropAppend,
 					    &cellsPropCount, &cellsPropAt, 0);
}

void Game::iaCanPlay()
{
  if (_ia)
    {
      _player = GET_OPPOSITE_COLOR(_player);
      _ref->iaCanPlay(_player);
    }
}

void Game::dispHmiText(const std::string &msg)
{
  //    std::cout << msg << std::endl;
}

void Game::deletePawl(int index)
{
    this->addPawl(index, 0);
}

//inutile ??
void Game::addPawl(int index)
{
    this->addPawl(index, 0);
}

void Game::addPawl(int index, int color)
{
  QString src;
  _cells[index]->source(this->MakePicture(index, color));
}

bool Game::clic(int index)
{
  std::ostringstream hmiText;

  if (_ref->setStone(index, _player) == true) {
    _player = GET_OPPOSITE_COLOR(_player); //on change de joueur
    if (_running)
    {
        hmiText << "Au tours du joueur " << GET_N_PLAYER(_player) + 1 << " de jouer";
        QString s = QString::fromStdString(hmiText.str());
        emit s_sendHmiText(s);
    }
  }
  return true;
}

bool Game::rclick(int index)
{
    _ref->getBoard()->DEBUG_METHOD_print_node(index, true);
    return (true);
}


