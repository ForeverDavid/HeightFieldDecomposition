#ifndef BOX_H
#define BOX_H

#include "lib/common/drawable_object.h"
#include "GUI/objects/cylinder.h"

class Box3D : public DrawableObject{
    public:
        Box3D();
        Box3D(const Pointd &min, const Pointd &max, const Pointd &c1 = Pointd(), const Pointd &c2 = Pointd(), const Pointd &c3 = Pointd(), const QColor c = QColor(0,0,0));
        Box3D(const Pointd &min, const Pointd &max, const QColor c);

        void setColor(const QColor &c);
        Pointd getMin() const;
        void setMin(const Pointd& value);
        Pointd getMax() const;
        void setMax(const Pointd& value);

        void setW(double d);
        void setH(double d);
        void setD(double d);
        void moveX(double d);
        void moveY(double d);
        void moveZ(double d);

        // DrawableObject interface
        void draw() const;
        Pointd sceneCenter() const;
        double sceneRadius() const;
        bool isVisible() const;
        void setVisible(bool b);

    protected:
        Pointd min, max;
        Pointd c1, c2, c3;
        QColor color;
        bool visible;
};

inline void Box3D::setColor(const QColor& c) {
    color = c;
}

inline Pointd Box3D::getMin() const {
    return min;
}

inline void Box3D::setMin(const Pointd& value) {
    min = value;
}

inline Pointd Box3D::getMax() const {
    return max;
}

inline void Box3D::setMax(const Pointd& value) {
    max = value;
}

inline void Box3D::setW(double d) {
    max.setX(min.x()+d);
}

inline void Box3D::setH(double d) {
    max.setY(min.y()+d);
}

inline void Box3D::setD(double d) {
    max.setZ(min.z()+d);
}

inline void Box3D::moveX(double d) {
    min.setX(min.x() + d);
    max.setX(max.x() + d);
}

inline void Box3D::moveY(double d) {
    min.setY(min.y() + d);
    max.setY(max.y() + d);
}

inline void Box3D::moveZ(double d) {
    min.setZ(min.z() + d);
    max.setZ(max.z() + d);
}

#endif // BOX_H