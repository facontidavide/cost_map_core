#include "grid_map/visualization/qt_display.hpp"
#include <QGuiApplication>
#include <QAction>
#include <QScreen>
#include <QDebug>

QImage GridMapToImage(const grid_map::Matrix &matrix )
{
    QImage image( matrix.cols(), matrix.rows(), QImage::Format_Mono );
    grid_map::Matrix transposed = matrix.transpose();
    memcpy( image.bits(), transposed.data(), matrix.size() );
    return image;
}


ImageViewer::ImageViewer()
   : image_widget_(new NonAntiAliasImage(this) )
   , scaleFactor(1.0)
{
    image_widget_->setBackgroundRole(QPalette::Base);
    image_widget_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    this->setBackgroundRole(QPalette::Dark);
    this->setWidget(image_widget_);
}


bool ImageViewer::load(const grid_map::Matrix &matrix )
{
    QImage newImage( QSize(matrix.rows(), matrix.cols()), QImage::Format_Grayscale8 );
    memcpy( newImage.bits(), matrix.data(), matrix.size() );
    setImage(newImage);
    return true;
}

void ImageViewer::setImage(const QImage &newImage)
{
    image = newImage;
    image_widget_->setPixmap(QPixmap::fromImage(image));
    scaleFactor = 1.0;
    scaleImage(1.0);
    resize(image_widget_->size());
}


void ImageViewer::scaleImage(double factor)
{
    Q_ASSERT(image_widget_->pixmap());
    scaleFactor *= factor;
    image_widget_->resize(scaleFactor * image_widget_->pixmap().size());

    adjustScrollBar(horizontalScrollBar(), factor);
    adjustScrollBar(verticalScrollBar(), factor);
}

void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

void ImageViewer::wheelEvent(QWheelEvent *event)
{
    if( event->delta() > 0)
    {
        scaleImage(0.9);
    }
    else{
        scaleImage(1.1);
    }
}

const QPixmap &NonAntiAliasImage::pixmap() const
{
    return m_pixmap;
}

void NonAntiAliasImage::setPixmap(const QPixmap &px)
{
    m_pixmap = px;
    update();
}

void NonAntiAliasImage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    style()->drawItemPixmap(&painter, rect(), Qt::AlignCenter,
                            m_pixmap.scaled(rect().size()));
}
