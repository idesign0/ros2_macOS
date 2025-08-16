from python_qt_binding.QtCore import QRectF
from python_qt_binding.QtWidgets import QAbstractGraphicsShapeItem


class QGraphicsBox3dItem(QAbstractGraphicsShapeItem):

    def __init__(self, bounding_box):
        super(QGraphicsBox3dItem, self).__init__()
        self._bounding_box = bounding_box

    def boundingRect(self):
        return self._bounding_box

    def paint(self, painter, option, widget):
        # Main rectangle
        rectangle = QRectF(self._bounding_box.topLeft().x(),
                           self._bounding_box.topLeft().y() + self._bounding_box.height() * 0.1,
                           self._bounding_box.width() - self._bounding_box.height() * 0.1,
                           self._bounding_box.height() - self._bounding_box.height() * 0.1)
        painter.drawRect(rectangle)
        # Top line
        painter.drawLine(int(rectangle.topLeft().x() + self._bounding_box.height() * 0.1),
                         int(self._bounding_box.topLeft().y()),
                         int(self._bounding_box.topRight().x()),
                         int(self._bounding_box.topRight().y()))
        # Top left corner
        painter.drawLine(int(rectangle.topLeft().x() + self._bounding_box.height() * 0.1),
                         int(self._bounding_box.topLeft().y()),
                         int(self._bounding_box.topLeft().x() + 1),
                         int(rectangle.topLeft().y()))
        # Top right corner
        painter.drawLine(int(self._bounding_box.topRight().x()),
                         int(self._bounding_box.topRight().y()),
                         int(rectangle.topRight().x()),
                         int(rectangle.topRight().y()))
        # Bottom right corner
        painter.drawLine(int(rectangle.bottomRight().x() + 1),
                         int(rectangle.bottomRight().y() - 1),
                         int(self._bounding_box.bottomRight().x()),
                         int(rectangle.bottomRight().y() - self._bounding_box.height() * 0.1))
        # Right line
        painter.drawLine(int(self._bounding_box.topRight().x()),
                         int(self._bounding_box.topRight().y()),
                         int(self._bounding_box.topRight().x()),
                         int(self._bounding_box.bottomRight().y()
                             - self._bounding_box.height() * 0.1))
