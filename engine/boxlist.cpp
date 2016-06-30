#include "boxlist.h"

BoxList::BoxList() : visible(true), visibleBox(-1), cylinder(true){

}

BoxList::BoxList(bool cylinders) : visible(true), visibleBox(-1), cylinder(cylinders){
}

void BoxList::addBox(const Box3D& b) {
    boxes.push_back(b);
}

void BoxList::clearBoxes() {
    boxes.clear();
}

unsigned int BoxList::getNumberBoxes() const{
    return boxes.size();
}

Box3D BoxList::getBox(unsigned int i) const {
    if (i < boxes.size())
        return (boxes[i]);
    return Box3D();
}

void BoxList::setBox(unsigned int i, const Box3D& b) {
    assert (i < boxes.size());
    boxes[i] = b;
}

void BoxList::insert(const BoxList& o) {
    boxes.insert(boxes.end(), o.boxes.begin(), o.boxes.end());
}

void BoxList::removeBox(unsigned int i) {
    assert (i < boxes.size());
    boxes.erase(boxes.begin()+i);
}

void BoxList::serialize(std::ofstream& binaryFile) const {
    Serializer::serialize(boxes, binaryFile);
}

void BoxList::deserialize(std::ifstream& binaryFile) {
    Serializer::deserialize(boxes, binaryFile);
    visibleBox = 0;
    cylinder = false;
}

void BoxList::setVisibleBox(int i) {
    if (i>=-1 && i < (int)boxes.size())
        visibleBox = i;
}

void BoxList::setCylinders(bool b) {
    cylinder = b;
}

void BoxList::draw() const {
    if (visible){
        if (visibleBox < 0){
            if (!cylinder){
                for (unsigned int i = 0; i < boxes.size(); i++)
                    boxes[i].draw();
            }
            else {
                for (unsigned int i = 0; i < boxes.size(); i++)
                    drawCube(boxes[i]);
            }
        }
        else
            boxes[visibleBox].draw();
    }
}

Pointd BoxList::sceneCenter() const {
    return std::move(Pointd());
}

double BoxList::sceneRadius() const {
    return -1;
}

bool BoxList::isVisible() const {
    return visible;
}

void BoxList::setVisible(bool b) {
    visible = b;
}

void BoxList::drawLine(const Pointd &a, const Pointd &b) const {
    glBegin(GL_LINES);
    glColor3f(0.0, 0.0, 0.0);
    glVertex3f(a.x(), a.y(), a.z());
    glVertex3f(b.x(), b.y(), b.z());
    glEnd();
}

void BoxList::drawCube(const Box3D& b) const {
    Pointd to(b.getMin().x(), b.getMin().y(), b.getMax().z());
    drawLine(b.getMin(), to);
    to.set(b.getMin().x(), b.getMax().y(), b.getMin().z());
    drawLine(b.getMin(), to);
    to.set(b.getMax().x(), b.getMin().y(), b.getMin().z());
    drawLine(b.getMin(), to);

    to.set(b.getMax().x(), b.getMax().y(), b.getMin().z());
    drawLine(b.getMax(), to);
    to.set(b.getMax().x(), b.getMin().y(), b.getMax().z());
    drawLine(b.getMax(), to);
    to.set(b.getMin().x(), b.getMax().y(), b.getMax().z());
    drawLine(b.getMax(), to);

    Pointd from(b.getMin().x(), b.getMin().y(), b.getMax().z());
    to.set(b.getMin().x(), b.getMax().y(), b.getMax().z());
    drawLine(from, to);
    from.set(b.getMin().x(), b.getMin().y(), b.getMax().z());
    to.set(b.getMax().x(), b.getMin().y(), b.getMax().z());
    drawLine(from, to);

    from.set(b.getMin().x(), b.getMax().y(), b.getMin().z());
    to.set(b.getMin().x(), b.getMax().y(), b.getMax().z());
    drawLine(from, to);
    from.set(b.getMin().x(), b.getMax().y(), b.getMin().z());
    to.set(b.getMax().x(), b.getMax().y(), b.getMin().z());
    drawLine(from, to);

    from.set(b.getMax().x(), b.getMin().y(), b.getMin().z());
    to.set(b.getMax().x(), b.getMax().y(), b.getMin().z());
    drawLine(from, to);
    from.set(b.getMax().x(), b.getMin().y(), b.getMin().z());
    to.set(b.getMax().x(), b.getMin().y(), b.getMax().z());
    drawLine(from, to);
}
