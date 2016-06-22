/*
 * @author    Alessandro Muntoni (muntoni.alessandro@gmail.com)
 * @copyright Alessandro Muntoni 2016.
 */

#ifndef PICKABLEDCEL_H
#define PICKABLEDCEL_H

#include "drawable_dcel.h"
#include "lib/common/pickable_object.h"

/**
 * \~Italian
 * @class PickableDcel
 */
class PickableDcel : public DrawableDcel, public PickableObject
{
    public:
        PickableDcel();
        PickableDcel(const Dcel &d);
        void drawWithNames() const;

    protected:
        #ifdef CGAL_DEFINED
        void drawFace(const Face* f) const;
        std::vector<int> obtainFaceTriangles(const Face* f) const;
        #endif
};

#endif // PICKABLEDCEL_H