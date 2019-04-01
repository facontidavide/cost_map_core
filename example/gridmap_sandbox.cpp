#include <QApplication>
#include <QHBoxLayout>
#include <grid_map/operators/Inflation.hpp>
#include <grid_map/Polygon.hpp>
#include <grid_map/iterators/PolygonIterator.hpp>
#include "grid_map/visualization/qt_display.hpp"

int main(int argc, char *argv[])
{
    grid_map::GridMap map({"layer", "inflated"});
    map.setGeometry(grid_map::Length(5.0, 3.0), 0.01, grid_map::Position(0.0, 0.0)); // bufferSize(8, 5)

    grid_map::Matrix& image_matrix = map.get("layer");
    image_matrix.fill( grid_map::FREE_SPACE );

    grid_map::Polygon polygon;
    polygon.addVertex(grid_map::Position(0.0, 0.0));
    polygon.addVertex(grid_map::Position(2.0, 0.0));
    polygon.addVertex(grid_map::Position(0.0, 1.0));

    for (grid_map::PolygonIterator iterator(map, polygon);
         !iterator.isPastEnd();
         ++iterator)
    {
        image_matrix( (*iterator)(0), (*iterator)(1) ) = grid_map::LETHAL_OBSTACLE;
    }

    grid_map::Inflate inflator;
    grid_map::ROSInflationComputer computer(0.0, 3.0);

    inflator("layer", "inflated", 0.4, computer, map);

    //-----------------
    // Show in Qt
    QApplication app(argc, argv);
    QGuiApplication::setApplicationDisplayName(ImageViewer::tr("Image Viewer"));

    QMainWindow win;
    auto layout = new QHBoxLayout();

    auto image1 = new ImageViewer();
    image1->load( map.get("layer") );
    auto image2 = new ImageViewer();
    image2->load( map.get("inflated") );

    layout->addWidget(image1);
    layout->addWidget(image2);

    image1->setSizePolicy( QSizePolicy ::Expanding , QSizePolicy ::Expanding );
    image2->setSizePolicy( QSizePolicy ::Expanding , QSizePolicy ::Expanding );

    QWidget *main_widget = new QWidget();
    main_widget->setLayout( layout );
    win.setCentralWidget( main_widget );
    win.resize( QSize( main_widget->width()+50, main_widget->height()+100) );

    win.show();
    return app.exec();
}
