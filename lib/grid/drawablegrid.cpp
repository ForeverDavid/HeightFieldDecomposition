#include "drawablegrid.h"
#include <omp.h>

DrawableGrid::DrawableGrid() : visible(true), drawMode(DRAW_KERNEL), slice(NO_SLICE), sliceValue(0) {
}

DrawableGrid::DrawableGrid(const Eigen::RowVector3i& resolution, const Eigen::MatrixXd& gridCoordinates, const Eigen::VectorXd& signedDistances, const Eigen::RowVector3i& gMin, const Eigen::RowVector3i& gMax) :
    Grid(resolution, gridCoordinates, signedDistances, gMin, gMax), visible(true), drawMode(DRAW_KERNEL), slice(NO_SLICE), sliceValue(0) {
}

DrawableGrid::~DrawableGrid(){
}

double DrawableGrid::getKernelDistance() const {
    return kernelDistance;
}

void DrawableGrid::setKernelDistance(double value) {
    kernelDistance = value;
}

void DrawableGrid::setDrawKernel() {
    drawMode = DRAW_KERNEL;
}

void DrawableGrid::setDrawBorders() {
    drawMode = DRAW_BORDERS;
}

void DrawableGrid::setSlice(int value) {
    slice = value;
    sliceValue = 0;
}

void DrawableGrid::setSliceValue(int value) {
    switch(slice){
        case X_SLICE:
            assert (value < (int)resX);
            break;
        case Y_SLICE:
            assert (value < (int)resY);
            break;
        case Z_SLICE:
            assert (value < (int)resZ);
            break;
    }
    sliceValue = value;
}

double DrawableGrid::getHsvFactor(double w) const {
    return 1-((w - MIN_PAY)/(MAX_PAY-MIN_PAY));
}

void DrawableGrid::draw() const {
    if (visible){
        //double xi, yi, zi;
        switch (drawMode){
            case DRAW_KERNEL:
                switch (slice){
                    case NO_SLICE:
                        for (unsigned int i = 0; i < getResX(); ++i){
                            for (unsigned int j = 0; j < getResY(); ++j){
                                for (unsigned int k = 0; k < getResZ(); ++k){
                                    if (getSignedDistance(i,j,k) < -kernelDistance){
                                        sphere(getPoint(i,j,k), 0.3, QColor(255,0,0));
                                    }
                                }
                            }
                        }
                        break;
                    case X_SLICE:
                        for (unsigned int j = 0; j < getResY(); ++j){
                            for (unsigned int k = 0; k < getResZ(); ++k){
                                if (getSignedDistance(sliceValue,j,k) < -kernelDistance){
                                    sphere(getPoint(sliceValue,j,k), 0.3, QColor(255,0,0));
                                }
                            }
                        }
                        break;
                    case Y_SLICE:
                        for (unsigned int i = 0; i < getResX(); ++i){
                            for (unsigned int k = 0; k < getResZ(); ++k){
                                if (getSignedDistance(i,sliceValue,k) < -kernelDistance){
                                    sphere(getPoint(i,sliceValue,k), 0.3, QColor(255,0,0));
                                }
                            }
                        }
                        break;
                    case Z_SLICE:
                        for (unsigned int i = 0; i < getResX(); ++i){
                            for (unsigned int j = 0; j < getResY(); ++j){
                                if (getSignedDistance(i,j,sliceValue) < -kernelDistance){
                                    sphere(getPoint(i,j,sliceValue), 0.3, QColor(255,0,0));
                                }
                            }
                        }
                        break;
                }
                break;
            case DRAW_BORDERS:
                switch (slice){
                    case NO_SLICE:
                        for (unsigned int i = 0; i < getResX(); ++i){
                            for (unsigned int j = 0; j < getResY(); ++j){
                                for (unsigned int k = 0; k < getResZ(); ++k){
                                    double w = getWeight(i,j,k);
                                    QColor c; c.setHsv(getHsvFactor(w)*240,255,255);
                                    sphere(getPoint(i,j,k), 0.3, c);
                                }
                            }
                        }
                        break;
                    case X_SLICE:
                        for (unsigned int j = 0; j < getResY(); ++j){
                            for (unsigned int k = 0; k < getResZ(); ++k){
                                double w = getWeight(sliceValue,j,k);
                                QColor c; c.setHsv(getHsvFactor(w)*240,255,255);
                                sphere(getPoint(sliceValue,j,k), 0.4, c);
                            }
                        }
                        /*xi = gridCoordinates(getIndex(sliceValue,0,0), 0);
                        for (yi = bb.getMinY(); yi <= bb.getMaxY(); yi+=0.5){
                            for (zi = bb.getMinZ(); zi <= bb.getMaxZ(); zi+=0.5){
                                double w = getValue(Pointd(xi,yi,zi));
                                QColor c; c.setHsv(getHsvFactor(w)*240,255,255);
                                sphere(Pointd(xi,yi,zi), 0.2, c);
                            }
                        }*/
                        break;
                    case Y_SLICE:
                        for (unsigned int i = 0; i < getResX(); ++i){
                            for (unsigned int k = 0; k < getResZ(); ++k){
                                double w = getWeight(i,sliceValue,k);
                                QColor c; c.setHsv(getHsvFactor(w)*240,255,255);
                                sphere(getPoint(i,sliceValue,k), 0.4, c);
                            }
                        }
                        /*yi = gridCoordinates(getIndex(0,sliceValue,0), 1);
                        for (xi = bb.getMinX(); xi <= bb.getMaxX(); xi+=0.5){
                            for (zi = bb.getMinZ(); zi <= bb.getMaxZ(); zi+=0.5){
                                double w = getValue(Pointd(xi,yi,zi));
                                QColor c; c.setHsv(getHsvFactor(w)*240,255,255);
                                sphere(Pointd(xi,yi,zi), 0.2, c);
                            }
                        }*/
                        break;
                    case Z_SLICE:
                        for (unsigned int i = 0; i < getResX(); ++i){
                            for (unsigned int j = 0; j < getResY(); ++j){
                                double w = getWeight(i,j,sliceValue);
                                QColor c; c.setHsv(getHsvFactor(w)*240,255,255);
                                sphere(getPoint(i,j,sliceValue), 0.4, c);
                            }
                        }
                        /*zi = gridCoordinates(getIndex(0,0,sliceValue), 2);
                        for (xi = bb.getMinX(); xi <= bb.getMaxX(); xi+=0.5){
                            for (yi = bb.getMinY(); yi <= bb.getMaxY(); yi+=0.5){
                                double w = getValue(Pointd(xi,yi,zi));
                                QColor c; c.setHsv(getHsvFactor(w)*240,255,255);
                                sphere(Pointd(xi,yi,zi), 0.2, c);
                            }
                        }*/
                        break;
                }
                break;
            default:
                assert(0);
        }

        for (unsigned int i = 0; i < cubes.size(); ++i){
            drawCube(cubes[i]);
        }
    }
}

Pointd DrawableGrid::sceneCenter() const {
    return bb.center();
}

double DrawableGrid::sceneRadius() const {
   return bb.diag();
}

bool DrawableGrid::isVisible() const {
    return visible;
}

void DrawableGrid::setVisible(bool b) {
    visible = b;
}

void DrawableGrid::addCube(const BoundingBox& bb) {
    cubes.push_back(bb);
}

void DrawableGrid::drawLine(const Pointd &a, const Pointd &b) const
{
    glBegin(GL_LINES);
    glColor3f(0.0, 0.0, 0.0);
    glVertex3f(a.x(), a.y(), a.z());
    glVertex3f(b.x(), b.y(), b.z());
    glEnd();
}

void DrawableGrid::drawCube(const BoundingBox &b) const
{
    Pointd to(b.getMinX(), b.getMinY(), b.getMaxZ());
    drawLine(b.getMin(), to);
    to.set(b.getMinX(), b.getMaxY(), b.getMinZ());
    drawLine(b.getMin(), to);
    to.set(b.getMaxX(), b.getMinY(), b.getMinZ());
    drawLine(b.getMin(), to);

    to.set(b.getMaxX(), b.getMaxY(), b.getMinZ());
    drawLine(b.getMax(), to);
    to.set(b.getMaxX(), b.getMinY(), b.getMaxZ());
    drawLine(b.getMax(), to);
    to.set(b.getMinX(), b.getMaxY(), b.getMaxZ());
    drawLine(b.getMax(), to);

    Pointd from(b.getMinX(), b.getMinY(), b.getMaxZ());
    to.set(b.getMinX(), b.getMaxY(), b.getMaxZ());
    drawLine(from, to);
    from.set(b.getMinX(), b.getMinY(), b.getMaxZ());
    to.set(b.getMaxX(), b.getMinY(), b.getMaxZ());
    drawLine(from, to);

    from.set(b.getMinX(), b.getMaxY(), b.getMinZ());
    to.set(b.getMinX(), b.getMaxY(), b.getMaxZ());
    drawLine(from, to);
    from.set(b.getMinX(), b.getMaxY(), b.getMinZ());
    to.set(b.getMaxX(), b.getMaxY(), b.getMinZ());
    drawLine(from, to);

    from.set(b.getMaxX(), b.getMinY(), b.getMinZ());
    to.set(b.getMaxX(), b.getMaxY(), b.getMinZ());
    drawLine(from, to);
    from.set(b.getMaxX(), b.getMinY(), b.getMinZ());
    to.set(b.getMaxX(), b.getMinY(), b.getMaxZ());
    drawLine(from, to);
}