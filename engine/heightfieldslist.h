#ifndef HEIGHTFIELDSLIST_H
#define HEIGHTFIELDSLIST_H

#include "igl/gui/drawableiglmesh.h"
#include "viewer/interfaces/drawable_object.h"

class HeightfieldsList : public DrawableObject, public SerializableObject{
    public:
        HeightfieldsList();
        // DrawableObject interface
        void draw() const;
        Pointd sceneCenter() const;
        double sceneRadius() const;
        bool isVisible() const;
        void setVisible(bool b);
        void setVisibleHeightfield(int i);
        void resize(int n);
        unsigned int getNumberVerticesHeightfield(int i) const;
        Pointd getVertexOfHeightfield(int he, int v) const;
        void setWireframe(bool b);
        void setPointShading();
        void setFlatShading();
        void setSmoothShading();

        void addHeightfield(const IGLInterface::DrawableIGLMesh &m, const Vec3 &target, int i = -1);
        unsigned int getNumHeightfields() const;

        void removeHeightfield(unsigned int i);
        const IGLInterface::IGLMesh& getHeightfield(unsigned int i) const;
        //IGLInterface::IGLMesh getHeightfield(unsigned int i) const;
        void setHeightfield(const IGLInterface::IGLMesh& he, unsigned int i);

        void explode(double dist);

        // SerializableObject interface
        void serialize(std::ofstream& binaryFile) const;
        void deserialize(std::ifstream& binaryFile);

    private:
        std::vector<IGLInterface::DrawableIGLMesh> heightfields;
        std::vector<Vec3> targets;
        bool visible;
        int nVisible;
};

#endif // HEIGHTFIELDSLIST_H
