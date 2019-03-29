#include <QApplication>
#include <grid_map/Polygon.hpp>
#include <grid_map/iterators/PolygonIterator.hpp>
#include "grid_map/visualization/qt_display.hpp"

int main(int argc, char *argv[])
{
    grid_map::GridMap map({"layer"});
    map.setGeometry(grid_map::Length(8.0, 6.0), 0.01, grid_map::Position(0.0, 0.0)); // bufferSize(8, 5)

    grid_map::Matrix& image_matrix = map.get("layer");
    image_matrix.fill(255);

    grid_map::Polygon polygon;
    polygon.addVertex(grid_map::Position(-1.0, 1.5));
    polygon.addVertex(grid_map::Position(0.0, 2));
    polygon.addVertex(grid_map::Position(1.0, 1.5));
    polygon.addVertex(grid_map::Position(1.0, -1.5));
    polygon.addVertex(grid_map::Position(-1.0, -1.5));

    for (grid_map::PolygonIterator iterator(map, polygon);
         !iterator.isPastEnd();
         ++iterator)
    {
        image_matrix( (*iterator)(0), (*iterator)(1) ) = 0;
    }

    //-----------------
    // Show in Qt
    QApplication app(argc, argv);
    QGuiApplication::setApplicationDisplayName(ImageViewer::tr("Image Viewer"));

    QMainWindow window;
    auto image_view = new ImageViewer();
    image_view->load( image_matrix );
    window.setCentralWidget( image_view );
    window.resize( image_view->size() );

    window.show();
    return app.exec();
}
