#ifndef TILESETVIEW_H
#define TILESETVIEW_H

#include <QWidget>
#include <QTimer>

class TilesetView : public QWidget
{
    Q_OBJECT
public:
    explicit TilesetView(QWidget *parent, const QPixmap *tiles);
    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent*);

private:
    const QPixmap* pixmap;
    QTimer timer;

};

class SpritesView : public QWidget
{
    Q_OBJECT
public:
    explicit SpritesView(QWidget *parent);
    QSize sizeHint() const;

public slots:
    void setBank(int);
    void setPalette(int);
    void setColor(int);

protected:
    void paintEvent(QPaintEvent*);

private:
    uint bankNum, palNum, colorNum;
    QImage bank[2];

    void updateBank();

};

#endif // TILESETVIEW_H
