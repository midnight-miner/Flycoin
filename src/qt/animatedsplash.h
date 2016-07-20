#ifndef ANIMATEDSPLASH
#define ANIMATEDSPLASH
#include <QtWidgets>

class AnimatedSplash:public QSplashScreen
{
  Q_OBJECT
  public:
    AnimatedSplash():movie(":/images/splash")
    {
      setPixmap(QPixmap::fromImage(QImage(":/images/splash")));
      movie.start();
      connect(&movie,SIGNAL(updated(QRect)),this,SLOT(frameUpdate()));
	  	  
	  QTimer *timer = new QTimer(this);
      connect(timer, SIGNAL(timeout()), this, SLOT(updateEvent()));
      timer->start(10);
    }

    ~AnimatedSplash()
    {}

	QString startUpText;
	
	void updateEvent()
	{
		QApplication::instance()->processEvents();
	}
	
    void paintEvent(QPaintEvent* event)
    {
      QPainter painter(this);
	  	  
      painter.drawPixmap(movie.frameRect(), movie.currentPixmap());
	  
	  QPen pen(Qt::white);
	  painter.setPen(pen);
	  
	  QFont font = painter.font();
	  font.setPointSize(14);
	  painter.setFont(font);
	  painter.drawText(15, movie.frameRect().height()-fontMetrics().height(), startUpText);
    }
	
	void showMessage(QString text)
	{
		startUpText = text;
	}

  private slots:
    void frameUpdate()
    {
      setPixmap(movie.currentPixmap());
      update();
    }

  private:
    QMovie movie;

};
#endif // ANIMATEDSPLASH