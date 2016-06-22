/*
 * @author    Alessandro Muntoni (muntoni.alessandro@gmail.com)
 * @copyright Alessandro Muntoni 2016.
 */

#ifndef COMMON_H
#define COMMON_H

#include "lib/common/common.h"
#include "lib/dcel/dcel.h"
#include <set>
#include <Eigen/Core>
#include <memory>

#define ONE_ON_SQRT2 1/(sqrt(2))
#define ONE_ON_SQRT3 1/(sqrt(3))

#define RED_ID 0
#define GREEN_ID 1
#define BLUE_ID 2
#define YELLOW_ID 6
#define MAGENTA_ID 10
#define CYANO_ID 14
#define WHITE_ID 18

#define IS_RED(label) ((label == 0) || (label==3))
#define IS_GREEN(label) ((label == 1) || (label==4))
#define IS_BLUE(label) ((label == 2) || (label==5))
#define IS_YELLOW(label) ((label >= 6) && (label < 10))
#define IS_MAGENTA(label) ((label >= 10) && (label < 14))
#define IS_CYANO(label) ((label >= 14) && (label < 18))
#define IS_WHITE(label) ((label >= 18) && (label < 26))

extern int WINDOW_MANAGER_ID;
extern int DCEL_MANAGER_ID;
extern int ENGINE_MANAGER_ID;

static std::vector<Vec3> XYZ = {
    Vec3( 1.0f,  0.0f,  0.0f),                         //    +X : label  0
    Vec3( 0.0f,  1.0f,  0.0f),                         //    +Y : label  1
    Vec3( 0.0f,  0.0f,  1.0f),                         //    +Z : label  2
    Vec3(-1.0f,  0.0f,  0.0f),                         //    -X : label  3
    Vec3( 0.0f, -1.0f,  0.0f),                         //    -Y : label  4
    Vec3( 0.0f,  0.0f, -1.0f),                         //    -Z : label  5
    Vec3( ONE_ON_SQRT2,  ONE_ON_SQRT2,  0.0f),         //   +XY : label  6
    Vec3(-ONE_ON_SQRT2,  ONE_ON_SQRT2,  0.0f),         //  -X+Y : label  7
    Vec3(-ONE_ON_SQRT2, -ONE_ON_SQRT2,  0.0f),         //   -XY : label  8
    Vec3( ONE_ON_SQRT2, -ONE_ON_SQRT2,  0.0f),         //  +X-Y : label  9
    Vec3( ONE_ON_SQRT2,  0.0f,  ONE_ON_SQRT2),         //   +XZ : label 10
    Vec3(-ONE_ON_SQRT2,  0.0f,  ONE_ON_SQRT2),         //  -X+Z : label 11
    Vec3(-ONE_ON_SQRT2,  0.0f, -ONE_ON_SQRT2),         //   -XZ : label 12
    Vec3( ONE_ON_SQRT2,  0.0f, -ONE_ON_SQRT2),         //  +X-Z : label 13
    Vec3( 0.0f,  ONE_ON_SQRT2,  ONE_ON_SQRT2),         //   +YZ : label 14
    Vec3( 0.0f, -ONE_ON_SQRT2,  ONE_ON_SQRT2),         //  -Y+Z : label 15
    Vec3( 0.0f, -ONE_ON_SQRT2, -ONE_ON_SQRT2),         //   -YZ : label 16
    Vec3( 0.0f,  ONE_ON_SQRT2, -ONE_ON_SQRT2),         //  +Y-Z : label 17
    Vec3( ONE_ON_SQRT3,  ONE_ON_SQRT3,  ONE_ON_SQRT3), //  +XYZ : label 18
    Vec3(-ONE_ON_SQRT3, -ONE_ON_SQRT3, -ONE_ON_SQRT3), //  -XYZ : label 19
    Vec3( ONE_ON_SQRT3, -ONE_ON_SQRT3, -ONE_ON_SQRT3), // +X-YZ : label 20
    Vec3(-ONE_ON_SQRT3,  ONE_ON_SQRT3,  ONE_ON_SQRT3), // -X+YZ : label 21
    Vec3( ONE_ON_SQRT3,  ONE_ON_SQRT3, -ONE_ON_SQRT3), // +XY-Z : label 22
    Vec3(-ONE_ON_SQRT3, -ONE_ON_SQRT3,  ONE_ON_SQRT3), // -XY+Z : label 23
    Vec3( ONE_ON_SQRT3, -ONE_ON_SQRT3,  ONE_ON_SQRT3), //+X-Y+Z : label 24
    Vec3(-ONE_ON_SQRT3,  ONE_ON_SQRT3, -ONE_ON_SQRT3)  //-X+Y-Z : label 25
};

static std::vector<QColor> colors = {
    QColor(255, 0, 0),    //     Red: label 0
    QColor(0, 255, 0),    //   Green: label 1
    QColor(0, 0, 255),    //    Blue: label 2
    QColor(255, 0, 0),    //     Red: label 3
    QColor(0, 255, 0),    //   Green: label 4
    QColor(0, 0, 255),    //    Blue: label 5
    QColor(255, 255, 0),    //  Yellow: label 6
    QColor(255, 255, 0),    //  Yellow: label 7
    QColor(255, 255, 0),    //  Yellow: label 8
    QColor(255, 255, 0),    //  Yellow: label 9
    QColor(255, 0, 255),    // Magenta: label 10
    QColor(255, 0, 255),    // Magenta: label 11
    QColor(255, 0, 255),    // Magenta: label 12
    QColor(255, 0, 255),    // Magenta: label 13
    QColor(0, 255, 255),    //   Cyano: label 14
    QColor(0, 255, 255),    //   Cyano: label 15
    QColor(0, 255, 255),    //   Cyano: label 16
    QColor(0, 255, 255),    //   Cyano: label 17
    QColor(200, 200, 200),    //   White: label 18
    QColor(200, 200, 200),    //   White: label 19
    QColor(200, 200, 200),    //   White: label 20
    QColor(200, 200, 200),    //   White: label 21
    QColor(200, 200, 200),    //   White: label 22
    QColor(200, 200, 200),    //   White: label 23
    QColor(200, 200, 200),    //   White: label 24
    QColor(200, 200, 200)     //   White: label 25
};

QColor colorOfNormal(const Vec3 &normal);

std::string exec(const char* cmd);

void getEigenVerteicesFacesFromDcel(Eigen::MatrixXd &V, Eigen::MatrixXi &F, Eigen::MatrixXd& C, Eigen::MatrixXd& NV, Eigen::MatrixXd& NF, Eigen::MatrixXd& BB, const Dcel &d);

#endif // COMMON_H