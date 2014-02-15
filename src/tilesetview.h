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
    void setMouseEnabled(bool);

protected:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

private:
    static const QColor infoColor;

    const QPixmap* pixmap;
    QTimer timer;
    bool mouseEnabled;
    int currTile;

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
