#ifndef TILESETVIEW_H
#define TILESETVIEW_H

#include <QWidget>
#include <QTimer>

class TilesetView : public QWidget
{
    Q_OBJECT
public:
    explicit TilesetView(QWidget *parent, const QPixmap *tiles, uint speed);
    QSize sizeHint() const;

public slots:
    void setAnimSpeed(int);

protected:
    void paintEvent(QPaintEvent*);

private:
    QTimer timer;
    const QPixmap* pixmap;

};

#endif // TILESETVIEW_H
