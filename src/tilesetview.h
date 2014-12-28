#ifndef TILESETVIEW_H
#define TILESETVIEW_H

#include <QWidget>
#include <QTimer>

class TilesetView : public QWidget
{
    Q_OBJECT
public:
    explicit TilesetView(QWidget *parent, const QPixmap *tiles, int singleTile = -1);
    QSize sizeHint() const;
    void setMouseEnabled(bool);

public slots:
    void setSingleTile(int tile) { singleTile = tile; }

protected:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

    static const QColor infoColor;

    QTimer timer;
    bool mouseEnabled;
    int currTile, singleTile;

private:
    const QPixmap* pixmap;

signals:
    void tileHovered(int);
    void tileSelected(int);

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

protected:
    void paintEvent(QPaintEvent*);

private:
    uint bankNum, palNum;
    QImage bank[2];

    void updateBank();

};

#endif // TILESETVIEW_H
