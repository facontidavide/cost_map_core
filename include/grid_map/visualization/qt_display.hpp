#ifndef QT_DISPLAY_HPP
#define QT_DISPLAY_HPP

#include "grid_map/GridMap.hpp"
#include <QMainWindow>
#include <QImage>
#include <QScrollBar>
#include <QLabel>
#include <QScrollArea>
#include <QWheelEvent>
#include <QPainter>
#include <QStyle>

class NonAntiAliasImage : public QWidget{
    Q_OBJECT
    Q_DISABLE_COPY(NonAntiAliasImage)
public:
    explicit NonAntiAliasImage(QWidget* parent = Q_NULLPTR)
        : QWidget(parent)
    {}
    const QPixmap& pixmap() const;
    void setPixmap(const QPixmap& px);

protected:
    void paintEvent(QPaintEvent*);

private:
    QPixmap m_pixmap;
};

class ImageViewer : public QScrollArea
{
    Q_OBJECT

public:
    ImageViewer();
    bool load(const grid_map::Matrix& input_matrix);

private:

    void setImage(const QImage &newImage);
    void scaleImage(double factor);

    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    void wheelEvent(QWheelEvent *event) override;

    QImage image;
    NonAntiAliasImage *image_widget_;
    double scaleFactor;

    QAction *normalSizeAct;
    QAction *fitToWindowAct;
};

#endif // QT_DISPLAY_HPP
