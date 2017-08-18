#include "boxlist.h"
#include "eigenmesh/gui/drawableeigenmesh.h"

BoxList::BoxList() : visible(true), visibleBox(-1), cylinder(true), eigenMesh(false){
}

BoxList::BoxList(bool cylinders) : visible(true), visibleBox(-1), cylinder(cylinders), eigenMesh(false){
}

void BoxList::addBox(const Box3D& b, int i) {
    if (i == -1)
        boxes.push_back(b);
    else {
        boxes.insert(boxes.begin()+i, b);
    }
}

void BoxList::clearBoxes() {
    boxes.clear();
}

unsigned int BoxList::getNumberBoxes() const {
    return boxes.size();
}

unsigned int BoxList::size() const {
    return boxes.size();
}

Box3D& BoxList::operator[](unsigned int i) {
    assert(i < boxes.size());
    return boxes[i];
}

const Box3D& BoxList::operator[](unsigned int i) const {
    assert(i < boxes.size());
    return boxes[i];
}

const Box3D &BoxList::getBox(unsigned int i) const {
    assert (i < boxes.size());
    return (boxes[i]);
}

const Box3D& BoxList::find(unsigned int id) const {
    //https://stackoverflow.com/questions/6679096/using-find-if-on-a-vector-of-object
    struct MyClassComp {
      explicit MyClassComp(int i) : n(i) { }
      inline bool operator()(const Box3D & m) const { return m.getId() == n; }
      int n;
    };

    std::vector<Box3D>::const_iterator it = std::find_if(boxes.begin(), boxes.end(), MyClassComp(id));
    return *it;
}

Box3D&BoxList::find(unsigned int id) {
    struct MyClassComp {
      explicit MyClassComp(int i) : n(i) { }
      inline bool operator()(const Box3D & m) const { return m.getId() == n; }
      int n;
    };

    std::vector<Box3D>::iterator it = std::find_if(boxes.begin(), boxes.end(), MyClassComp(id));
    return *it;
}

unsigned int BoxList::getIndexOf(unsigned int id) const {
    struct MyClassComp {
      explicit MyClassComp(int i) : n(i) { }
      inline bool operator()(const Box3D & m) const { return m.getId() == n; }
      int n;
    };

    std::vector<Box3D>::const_iterator it = std::find_if(boxes.begin(), boxes.end(), MyClassComp(id));
    return it - boxes.begin();
}

void BoxList::setBox(unsigned int i, const Box3D& b) {
    assert (i < boxes.size());
    boxes[i] = b;
}

void BoxList::insert(const BoxList& o) {
    boxes.insert(boxes.end(), o.boxes.begin(), o.boxes.end());
}

void BoxList::insert(const Box3D& b, int i) {
    if (i < 0){
        boxes.push_back(b);
        //boxes[boxes.size()-1].setId(boxes.size()-1);
    }
    else {
        assert ((unsigned int)i < boxes.size()+1);
        boxes.insert(boxes.begin()+i, b);
        /*for (; (unsigned int)i < boxes.size(); i++){
            boxes[i].setId(i);
        }*/
    }
}

void BoxList::removeBox(unsigned int i) {
    assert (i < boxes.size());
    boxes.erase(boxes.begin()+i);
}

void BoxList::getSubBoxLists(std::vector<BoxList>& v, int nPerBoxList) {
    int nBoxLists = getNumberBoxes() / nPerBoxList;
    if (getNumberBoxes() % nPerBoxList != 0) v.reserve(nBoxLists+1);
    else v.reserve(nBoxLists);
    for (unsigned int i = 0; i < getNumberBoxes(); i++){
        if (i % nPerBoxList == 0) v.push_back(BoxList());
        unsigned int j = i / nPerBoxList;
        v[j].addBox(getBox(i));
    }
}

void BoxList::setIds() {
    for (unsigned int i = 0; i < boxes.size(); i++){
        boxes[i].setId(i);
    }
}

void BoxList::sort(const Array2D<int>& ordering) {
    struct cmp {
        Array2D<int> order;
        cmp(const Array2D<int>& ord){
            order = ord;
        }
        bool operator()(const Box3D &a, const Box3D &b) const {
            bool val = order(a.getId(),b.getId());
            return val;
        }
    };

    std::sort(boxes.begin(), boxes.end(), cmp(ordering));

}

void BoxList::sortByTrianglesCovered() {
    struct cmp {
        bool operator()(const Box3D &a, const Box3D &b) const {
            if (a.getNumberTrianglesCovered() == b.getNumberTrianglesCovered())
                return a.getVolume() < b.getVolume();
            return a.getNumberTrianglesCovered() < b.getNumberTrianglesCovered();
        }
    };
    std::sort(boxes.begin(), boxes.end(), cmp());

}

void BoxList::sortByHeight() {
    struct cmp {
        bool operator()(const Box3D &a, const Box3D &b) const {
            double ah, bh;
            if (a.getTarget() == Vec3(1,0,0) || a.getTarget() == Vec3(-1,0,0))
                ah = a.getLengthX();
            else  if (a.getTarget() == Vec3(0,1,0) || a.getTarget() == Vec3(0,-1,0))
                ah = a.getLengthY();
            else
                ah = a.getLengthZ();

            if (b.getTarget() == Vec3(1,0,0) || b.getTarget() == Vec3(-1,0,0))
                bh = b.getLengthX();
            else  if (b.getTarget() == Vec3(0,1,0) || b.getTarget() == Vec3(0,-1,0))
                bh = b.getLengthY();
            else
                bh = b.getLengthZ();
            return (ah > bh);
        }
    };
    std::sort(boxes.begin(), boxes.end(), cmp());
}

void BoxList::generatePieces(double minimumDistance) {
    for (unsigned int i= 0; i < boxes.size(); i++){
        boxes[i].generateEigenMesh(minimumDistance);
    }
}

void BoxList::calculateTrianglesCovered(const CGALInterface::AABBTree& tree) {
    for (unsigned int i = 0; i < boxes.size(); i++){
        std::list<unsigned int> ids;
        tree.getCompletelyContainedDcelFaces(ids, boxes[i]);
        boxes[i].setTrianglesCovered(std::set<unsigned int>(ids.begin(), ids.end()));
    }
}

void BoxList::changeBoxLimits(const BoundingBox &newLimits, unsigned int i) {
    assert(i < boxes.size());
    boxes[i].min() = newLimits.min();
    boxes[i].max() = newLimits.max();
}

std::vector<Box3D>::const_iterator BoxList::begin() const {
    return boxes.begin();
}

std::vector<Box3D>::const_iterator BoxList::end() const {
    return boxes.end();
}

std::vector<Box3D>::iterator BoxList::begin() {
    return boxes.begin();
}

std::vector<Box3D>::iterator BoxList::end() {
    return boxes.end();
}

void BoxList::serialize(std::ofstream& binaryFile) const {
    Serializer::serialize(boxes, binaryFile);
}

bool BoxList::deserialize(std::ifstream& binaryFile) {
    if (Serializer::deserialize(boxes, binaryFile)){
        visibleBox = 0;
        cylinder = false;
        return true;
    }
    else
        return false;
}

void BoxList::setVisibleBox(int i) {
    if (i>=-1 && i < (int)boxes.size())
        visibleBox = i;
}

void BoxList::setCylinders(bool b) {
    cylinder = b;
}

void BoxList::visualizeEigenMeshBox(bool b) {
    eigenMesh = b;
}

void BoxList::draw() const {
    #ifdef VIEWER_DEFINED
    if (visible){
        if (visibleBox < 0){
            if (!cylinder){
                for (unsigned int i = 0; i < boxes.size(); i++)
                    boxes[i].draw();
            }
            else {
                for (unsigned int i = 0; i < boxes.size(); i++){
                    drawCube(boxes[i], boxes[i].getColor());
                }
            }
        }
        else {

            boxes[visibleBox].draw();
            if (eigenMesh){
                DrawableEigenMesh dm(boxes[visibleBox].getEigenMesh());
                //dm.setWireframe(true);
                //dm.setPointsShading();
                dm.draw();
            }
        }
    }
    #endif
}

Pointd BoxList::sceneCenter() const {
    BoundingBox bb(Pointd(-1,-1,-1), Pointd(1,1,1));
    for (unsigned int i = 0; i < boxes.size(); i++){
        bb.min() = bb.min().min(boxes[i].min());
        bb.max() = bb.max().max(boxes[i].max());
    }
    return bb.center();
}

double BoxList::sceneRadius() const {
    BoundingBox bb(Pointd(-1,-1,-1), Pointd(1,1,1));
    for (unsigned int i = 0; i < boxes.size(); i++){
        bb.min() = bb.min().min(boxes[i].min());
        bb.max() = bb.max().max(boxes[i].max());
    }
    return bb.diag() / 2;
}

bool BoxList::isVisible() const {
    return visible;
}

void BoxList::setVisible(bool b) {
    visible = b;
}

#ifdef VIEWER_DEFINED
void BoxList::drawLine(const Pointd &a, const Pointd &b, const Color& c) const {
    glBegin(GL_LINES);
    glColor3f(c.redF(), c.greenF(), c.blueF());
    glLineWidth(3);
    glVertex3f(a.x(), a.y(), a.z());
    glVertex3f(b.x(), b.y(), b.z());
    glEnd();
}

void BoxList::drawCube(const Box3D& b, const Color &c) const {
    std::vector<Pointd> p;
    b.getRotatedExtremes(p);
    drawLine(p[0], p[1], c);
    drawLine(p[1], p[2], c);
    drawLine(p[2], p[3], c);
    drawLine(p[0], p[3], c);

    drawLine(p[4], p[5], c);
    drawLine(p[5], p[6], c);
    drawLine(p[6], p[7], c);
    drawLine(p[4], p[7], c);

    drawLine(p[0], p[4], c);
    drawLine(p[1], p[5], c);
    drawLine(p[2], p[6], c);
    drawLine(p[3], p[7], c);
}
#endif
