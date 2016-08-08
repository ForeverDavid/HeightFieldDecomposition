#include "enginemanager.h"
#include "ui_enginemanager.h"
#include "common.h"
#include <cstdio>
#include <QFileDialog>
#include <omp.h>
#include "cgal/aabbtree.h"

EngineManager::EngineManager(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::EngineManager),
    mainWindow((MainWindow*)parent),
    g(nullptr),
    d(nullptr),
    b(nullptr),
    iterations(nullptr),
    solutions(nullptr),
    baseComplex(nullptr),
    he(nullptr) {
    ui->setupUi(this);
    ui->iterationsSlider->setMaximum(0);

    //ui->frame_2->setVisible(false);
    //ui->frame_5->setVisible(false);
}

void EngineManager::deleteDrawableObject(DrawableObject* d) {
    if (d != nullptr){
        d->setVisible(false);
        mainWindow->deleteObj(d);
        delete d;
        d = nullptr;
    }
}

EngineManager::~EngineManager() {
    delete ui;
    deleteDrawableObject(g);
    deleteDrawableObject(d);
    deleteDrawableObject(b);
    deleteDrawableObject(iterations);
    deleteDrawableObject(solutions);
    deleteDrawableObject(baseComplex);
}

void EngineManager::updateLabel(double value, QLabel* label) {
    std::stringstream ss;
    ss << std::setprecision(std::numeric_limits<double>::digits10+1);
    ss << value;
    label->setText(QString::fromStdString(ss.str()));
}

void EngineManager::updateBoxValues() {
    if (b != nullptr){
        ui->minXSpinBox->setValue(b->getMinX());
        ui->minYSpinBox->setValue(b->getMinY());
        ui->minZSpinBox->setValue(b->getMinZ());
        ui->maxXSpinBox->setValue(b->getMaxX());
        ui->maxYSpinBox->setValue(b->getMaxY());
        ui->maxZSpinBox->setValue(b->getMaxZ());
        ui->wSpinBox->setValue(b->getMaxX() - b->getMinX());
        ui->hSpinBox->setValue(b->getMaxY() - b->getMinY());
        ui->dSpinBox->setValue(b->getMaxZ() - b->getMinZ());
    }
}

void EngineManager::updateColors(double angleThreshold, double areaThreshold) {
    d->setColor(QColor(128,128,128));
    std::set<const Dcel::Face*> flippedFaces, savedFaces;
    Engine::getFlippedFaces(flippedFaces, savedFaces, *d, XYZ[ui->targetComboBox->currentIndex()], angleThreshold/100, areaThreshold);

    for (const Dcel::Face* cf : flippedFaces){
        Dcel::Face* f = d->getFace(cf->getId());
        f->setColor(QColor(255,0,0));
    }
    for (const Dcel::Face* cf : savedFaces){
        Dcel::Face* f = d->getFace(cf->getId());
        f->setColor(QColor(0,0,255));
    }

    d->update();
    mainWindow->updateGlCanvas();
}

void EngineManager::serialize(std::ofstream& binaryFile) const {
    g->serialize(binaryFile);
    d->serialize(binaryFile);
    bool bb = false;

    bb = false;
    if (solutions!=nullptr){
        bb = true;
        Serializer::serialize(bb, binaryFile);
        solutions->serialize(binaryFile);
    }
    else
        Serializer::serialize(bb, binaryFile);
}

void EngineManager::deserialize(std::ifstream& binaryFile) {
    deleteDrawableObject(g);
    deleteDrawableObject(d);
    g = new DrawableGrid();
    d = new DrawableDcel();
    g->deserialize(binaryFile);
    d->deserialize(binaryFile);
    bool bb = false;
    for (Dcel::FaceIterator fit = d->faceBegin(); fit != d->faceEnd(); ++fit)
        (*fit)->setColor(QColor(128,128,128));
    d->setWireframe(true);
    d->setPointsShading();
    updateColors(ui->toleranceSlider->value(), ui->areaToleranceSpinBox->value());
    d->update();
    mainWindow->pushObj(d, "Scaled Mesh");
    mainWindow->pushObj(g, "Grid");
    e = Energy(*g);
    ui->weigthsRadioButton->setChecked(true);
    ui->sliceCheckBox->setChecked(true);
    g->setDrawBorders();
    g->setSlice(1);


    Serializer::deserialize(bb, binaryFile);
    if (bb){
        deleteDrawableObject(solutions);
        solutions = new BoxList();
        solutions->deserialize(binaryFile);
        solutions->setVisibleBox(0);
        solutions->setCylinders(false);
        mainWindow->pushObj(solutions, "Solutions");
        ui->showAllSolutionsCheckBox->setEnabled(true);
        ui->solutionsSlider->setEnabled(true);
        ui->solutionsSlider->setMaximum(solutions->getNumberBoxes()-1);
        ui->setFromSolutionSpinBox->setValue(0);
        ui->setFromSolutionSpinBox->setMaximum(solutions->getNumberBoxes()-1);
    }

    mainWindow->updateGlCanvas();
}

void EngineManager::on_generateGridPushButton_clicked() {
    DcelManager* dm = (DcelManager*)mainWindow->getManager(DCEL_MANAGER_ID);
    DrawableDcel* dd = dm->getDcel();
    if (dd != nullptr){
        deleteDrawableObject(d);
        d = new DrawableDcel(*dd);
        mainWindow->pushObj(d, "Scaled Mesh");
        deleteDrawableObject(g);
        g = new DrawableGrid();

        Engine::scaleAndRotateDcel(*d, 0, ui->factorSpinBox->value());
        std::set<const Dcel::Face*> flippedFaces, savedFaces;
        Engine::getFlippedFaces(flippedFaces, savedFaces, *d, XYZ[ui->targetComboBox->currentIndex()], (double)ui->toleranceSlider->value()/100, ui->areaToleranceSpinBox->value());
        Engine::generateGrid(*g, *d, ui->distanceSpinBox->value(), ui->heightfieldsCheckBox->isChecked(), XYZ[ui->targetComboBox->currentIndex()], savedFaces);
        updateColors(ui->toleranceSlider->value(), ui->areaToleranceSpinBox->value());
        d->update();
        g->setKernelDistance(ui->distanceSpinBox->value());
        e = Energy(*g);
        mainWindow->pushObj(g, "Grid");
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_distanceSpinBox_valueChanged(double arg1) {
    if (g!=nullptr){
        g->setKernelDistance(arg1);
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_targetComboBox_currentIndexChanged(int index) {
    if (d!= nullptr && g!= nullptr){
        g->setTarget(XYZ[index]);
        updateColors(ui->toleranceSlider->value(), ui->areaToleranceSpinBox->value());
        d->update();
        std::set<const Dcel::Face*> flippedFaces, savedFaces;
        Engine::getFlippedFaces(flippedFaces, savedFaces, *d, XYZ[ui->targetComboBox->currentIndex()], (double)ui->toleranceSlider->value()/100, ui->areaToleranceSpinBox->value());
        g->calculateBorderWeights(*d, ui->heightfieldsCheckBox->isChecked(), savedFaces);
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_kernelRadioButton_toggled(bool checked) {
    if (checked && g!=nullptr){
        g->setDrawKernel();
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_weigthsRadioButton_toggled(bool checked) {
    if (checked && g!=nullptr){
        g->setDrawBorders();
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_freezeKernelPushButton_clicked() {
    if (g!=nullptr && d!=nullptr){
        double value = ui->distanceSpinBox->value();
        g->setTarget(XYZ[ui->targetComboBox->currentIndex()]);
        std::set<const Dcel::Face*> flippedFaces, savedFaces;
        Engine::getFlippedFaces(flippedFaces, savedFaces, *d, XYZ[ui->targetComboBox->currentIndex()], (double)ui->toleranceSlider->value()/100, ui->areaToleranceSpinBox->value());
        g->calculateWeightsAndFreezeKernel(*d, value, ui->heightfieldsCheckBox->isChecked(), savedFaces);
        e = Energy(*g);
        e.calculateFullBoxValues(*g);
        mainWindow->updateGlCanvas();
    }

}

void EngineManager::on_sliceCheckBox_stateChanged(int arg1) {
    if (g!=nullptr){
        if (arg1 == Qt::Checked){
            ui->sliceComboBox->setEnabled(true);
            ui->sliceSlider->setEnabled(true);
            int s = ui->sliceComboBox->currentIndex();
            g->setSliceValue(0);
            g->setSlice(s+1);
            if (s == 0) ui->sliceSlider->setMaximum(g->getResX() -1);
            if (s == 1) ui->sliceSlider->setMaximum(g->getResY() -1);
            if (s == 2) ui->sliceSlider->setMaximum(g->getResZ() -1);
        }
        else {
            ui->sliceComboBox->setEnabled(false);
            ui->sliceSlider->setEnabled(false);
            g->setSlice(0);
        }
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_sliceSlider_valueChanged(int value) {
    if (g!=nullptr){
        g->setSliceValue(value);
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_sliceComboBox_currentIndexChanged(int index) {
    if (g!=nullptr){
        g->setSliceValue(0);
        g->setSlice(index+1);
        ui->sliceSlider->setValue(0);
        if (index == 0) ui->sliceSlider->setMaximum(g->getResX() -1);
        if (index == 1) ui->sliceSlider->setMaximum(g->getResY() -1);
        if (index == 2) ui->sliceSlider->setMaximum(g->getResZ() -1);
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_serializePushButton_clicked() {
    QString filename = QFileDialog::getSaveFileName(nullptr,
                       "Serialize",
                       ".",
                       "BIN(*.bin)");
    if (!filename.isEmpty()) {
        std::ofstream myfile;
        myfile.open (filename.toStdString(), std::ios::out | std::ios::binary);
        serialize(myfile);
        myfile.close();
    }
}

void EngineManager::on_deserializePushButton_clicked() {
    QString filename = QFileDialog::getOpenFileName(nullptr,
                       "Deserialize",
                       ".",
                       "BIN(*.bin)");

    if (!filename.isEmpty()) {
        std::ifstream myfile;
        myfile.open (filename.toStdString(), std::ios::in | std::ios::binary);
        deserialize(myfile);
        myfile.close();
    }
}

void EngineManager::on_wSpinBox_valueChanged(double arg1) {
    if (b!=nullptr){
        b->setW(arg1);
        updateBoxValues();
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_hSpinBox_valueChanged(double arg1) {
    if (b!=nullptr){
        b->setH(arg1);
        updateBoxValues();
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_dSpinBox_valueChanged(double arg1) {
    if (b!=nullptr){
        b->setD(arg1);
        updateBoxValues();
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_plusXButton_clicked() {
    if (b!=nullptr){
        if (ui->boxRadioButton->isChecked()){
            b->moveX(ui->stepSpinBox->value());
            updateBoxValues();
        }
        else {
            Pointd c;
            if (ui->c1RadioButton->isChecked()){
                c = b->getConstraint1();
                b->setConstraint1(Pointd(c.x()+ui->stepSpinBox->value(), c.y(), c.z()));
            }
            if (ui->c2RadioButton->isChecked()){
                c = b->getConstraint2();
                b->setConstraint2(Pointd(c.x()+ui->stepSpinBox->value(), c.y(), c.z()));
            }
            if (ui->c3RadioButton->isChecked()){
                c = b->getConstraint3();
                b->setConstraint3(Pointd(c.x()+ui->stepSpinBox->value(), c.y(), c.z()));
            }
            if (ui->allRadioButton->isChecked()){
                b->moveX(ui->stepSpinBox->value());
                c = b->getConstraint1();
                b->setConstraint1(Pointd(c.x()+ui->stepSpinBox->value(), c.y(), c.z()));
                c = b->getConstraint2();
                b->setConstraint2(Pointd(c.x()+ui->stepSpinBox->value(), c.y(), c.z()));
                c = b->getConstraint3();
                b->setConstraint3(Pointd(c.x()+ui->stepSpinBox->value(), c.y(), c.z()));
            }
        }

        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_minusXButton_clicked() {
    if (b!=nullptr){
        if (ui->boxRadioButton->isChecked()){
            b->moveX(- ui->stepSpinBox->value());
            updateBoxValues();
        }
        else {
            Pointd c;
            if (ui->c1RadioButton->isChecked()){
                c = b->getConstraint1();
                b->setConstraint1(Pointd(c.x()-ui->stepSpinBox->value(), c.y(), c.z()));
            }
            if (ui->c2RadioButton->isChecked()){
                c = b->getConstraint2();
                b->setConstraint2(Pointd(c.x()-ui->stepSpinBox->value(), c.y(), c.z()));
            }
            if (ui->c3RadioButton->isChecked()){
                c = b->getConstraint3();
                b->setConstraint3(Pointd(c.x()-ui->stepSpinBox->value(), c.y(), c.z()));
            }
            if (ui->allRadioButton->isChecked()){
                b->moveX(- ui->stepSpinBox->value());
                c = b->getConstraint1();
                b->setConstraint1(Pointd(c.x()-ui->stepSpinBox->value(), c.y(), c.z()));
                c = b->getConstraint2();
                b->setConstraint2(Pointd(c.x()-ui->stepSpinBox->value(), c.y(), c.z()));
                c = b->getConstraint3();
                b->setConstraint3(Pointd(c.x()-ui->stepSpinBox->value(), c.y(), c.z()));
            }
        }
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_plusYButton_clicked() {
    if (b!=nullptr){
        if (ui->boxRadioButton->isChecked()){
            b->moveY(ui->stepSpinBox->value());
            updateBoxValues();
        }
        else {
            Pointd c;
            if (ui->c1RadioButton->isChecked()){
                c = b->getConstraint1();
                b->setConstraint1(Pointd(c.x(), c.y()+ui->stepSpinBox->value(), c.z()));
            }
            if (ui->c2RadioButton->isChecked()){
                c = b->getConstraint2();
                b->setConstraint2(Pointd(c.x(), c.y()+ui->stepSpinBox->value(), c.z()));
            }
            if (ui->c3RadioButton->isChecked()){
                c = b->getConstraint3();
                b->setConstraint3(Pointd(c.x(), c.y()+ui->stepSpinBox->value(), c.z()));
            }
            if (ui->allRadioButton->isChecked()){
                b->moveY(ui->stepSpinBox->value());
                c = b->getConstraint1();
                b->setConstraint1(Pointd(c.x(), c.y()+ui->stepSpinBox->value(), c.z()));
                c = b->getConstraint2();
                b->setConstraint2(Pointd(c.x(), c.y()+ui->stepSpinBox->value(), c.z()));
                c = b->getConstraint3();
                b->setConstraint3(Pointd(c.x(), c.y()+ui->stepSpinBox->value(), c.z()));
            }

        }
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_minusYButton_clicked() {
    if (b!=nullptr){
        if (ui->boxRadioButton->isChecked()){
            b->moveY(- ui->stepSpinBox->value());
            updateBoxValues();
        }
        else {
            Pointd c;
            if (ui->c1RadioButton->isChecked()){
                c = b->getConstraint1();
                b->setConstraint1(Pointd(c.x(), c.y()-ui->stepSpinBox->value(), c.z()));
            }
            if (ui->c2RadioButton->isChecked()){
                c = b->getConstraint2();
                b->setConstraint2(Pointd(c.x(), c.y()-ui->stepSpinBox->value(), c.z()));
            }
            if (ui->c3RadioButton->isChecked()){
                c = b->getConstraint3();
                b->setConstraint3(Pointd(c.x(), c.y()-ui->stepSpinBox->value(), c.z()));
            }
            if (ui->allRadioButton->isChecked()){
                b->moveY(- ui->stepSpinBox->value());
                c = b->getConstraint1();
                b->setConstraint1(Pointd(c.x(), c.y()-ui->stepSpinBox->value(), c.z()));
                c = b->getConstraint2();
                b->setConstraint2(Pointd(c.x(), c.y()-ui->stepSpinBox->value(), c.z()));
                c = b->getConstraint3();
                b->setConstraint3(Pointd(c.x(), c.y()-ui->stepSpinBox->value(), c.z()));
            }

        }
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_plusZButton_clicked() {
    if (b!=nullptr){
        if (ui->boxRadioButton->isChecked()){
            b->moveZ(ui->stepSpinBox->value());
            updateBoxValues();
        }
        else {
            Pointd c;
            if (ui->c1RadioButton->isChecked()){
                c = b->getConstraint1();
                b->setConstraint1(Pointd(c.x(), c.y(), c.z()+ui->stepSpinBox->value()));
            }
            if (ui->c2RadioButton->isChecked()){
                c = b->getConstraint2();
                b->setConstraint2(Pointd(c.x(), c.y(), c.z()+ui->stepSpinBox->value()));
            }
            if (ui->c3RadioButton->isChecked()){
                c = b->getConstraint3();
                b->setConstraint3(Pointd(c.x(), c.y(), c.z()+ui->stepSpinBox->value()));
            }
            if (ui->allRadioButton->isChecked()){
                b->moveZ(ui->stepSpinBox->value());
                c = b->getConstraint1();
                b->setConstraint1(Pointd(c.x(), c.y(), c.z()+ui->stepSpinBox->value()));
                c = b->getConstraint2();
                b->setConstraint2(Pointd(c.x(), c.y(), c.z()+ui->stepSpinBox->value()));
                c = b->getConstraint3();
                b->setConstraint3(Pointd(c.x(), c.y(), c.z()+ui->stepSpinBox->value()));
            }

        }
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_minusZButton_clicked() {
    if (b!=nullptr){
        if (ui->boxRadioButton->isChecked()){
            b->moveZ(- ui->stepSpinBox->value());
            updateBoxValues();
        }
        else {
            Pointd c;
            if (ui->c1RadioButton->isChecked()){
                c = b->getConstraint1();
                b->setConstraint1(Pointd(c.x(), c.y(), c.z()-ui->stepSpinBox->value()));
            }
            if (ui->c2RadioButton->isChecked()){
                c = b->getConstraint2();
                b->setConstraint2(Pointd(c.x(), c.y(), c.z()-ui->stepSpinBox->value()));
            }
            if (ui->c3RadioButton->isChecked()){
                c = b->getConstraint3();
                b->setConstraint3(Pointd(c.x(), c.y(), c.z()-ui->stepSpinBox->value()));
            }
            if (ui->allRadioButton->isChecked()){
                b->moveZ(- ui->stepSpinBox->value());
                c = b->getConstraint1();
                b->setConstraint1(Pointd(c.x(), c.y(), c.z()-ui->stepSpinBox->value()));
                c = b->getConstraint2();
                b->setConstraint2(Pointd(c.x(), c.y(), c.z()-ui->stepSpinBox->value()));
                c = b->getConstraint3();
                b->setConstraint3(Pointd(c.x(), c.y(), c.z()-ui->stepSpinBox->value()));
            }

        }
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_energyBoxPushButton_clicked() {
    if (b!=nullptr){
        double energy = e.energy(*b);
        Eigen::VectorXd gradient(6);
        Eigen::VectorXd solution(6);
        solution << b->getMinX(), b->getMinY(), b->getMinZ(), b->getMaxX(), b->getMaxY(), b->getMaxZ();
        e.gradientEnergy(gradient, solution, b->getConstraint1(), b->getConstraint2(), b->getConstraint3());
        updateLabel(gradient(0), ui->gminx);
        updateLabel(gradient(1), ui->gminy);
        updateLabel(gradient(2), ui->gminz);
        updateLabel(gradient(3), ui->gmaxx);
        updateLabel(gradient(4), ui->gmaxy);
        updateLabel(gradient(5), ui->gmaxz);
        std::cerr << "\nGradient: \n" << gradient << "\n";
        updateLabel(energy, ui->energyLabel);
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_minimizePushButton_clicked() {
    if (b!=nullptr){
        double it;
        if (ui->saveIterationsCheckBox->isChecked()){
            deleteDrawableObject(iterations);
            iterations = new BoxList();
            Timer t("Graidient Discent");
            it = e.gradientDiscend(*b, *iterations);
            t.stopAndPrint();
            iterations->setVisibleBox(0);
            ui->iterationsSlider->setMaximum(iterations->getNumberBoxes()-1);
            mainWindow->pushObj(iterations, "Iterations");

            double energy = e.energy(iterations->getBox(0));
            updateLabel(energy, ui->energyIterationLabel);
        }
        else {
            Timer t("Gradient Discent");
            it = e.gradientDiscend(*b);
            t.stopAndPrint();
        }
        updateLabel(it, ui->minimizedEnergyLabel);
        double energy = e.energy(*b);
        updateLabel(energy, ui->energyLabel);
        updateBoxValues();
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_BFGSButton_clicked() {
    if (b!=nullptr){
        double it;
        if (ui->saveIterationsCheckBox->isChecked()){
            deleteDrawableObject(iterations);
            iterations = new BoxList();
            Timer t("BFGS");
            it = e.BFGS(*b, *iterations);
            t.stopAndPrint();
            iterations->setVisibleBox(0);
            ui->iterationsSlider->setMaximum(iterations->getNumberBoxes()-1);
            mainWindow->pushObj(iterations, "Iterations");

            double energy = e.energy(iterations->getBox(0));
            updateLabel(energy, ui->energyIterationLabel);
        }
        else {
            Timer t("BFGS");
            it = e.BFGS(*b);
            t.stopAndPrint();
        }
        updateLabel(it, ui->minimizedEnergyLabel);
        double energy = e.energy(*b);
        updateLabel(energy, ui->energyLabel);
        updateBoxValues();
        mainWindow->updateGlCanvas();
    }
}


void EngineManager::on_serializeBoxPushButton_clicked() {
    if (b!=nullptr){
        std::ofstream myfile;
        myfile.open ("box.bin", std::ios::out | std::ios::binary);
        b->serialize(myfile);
        myfile.close();
    }
}

void EngineManager::on_deserializeBoxPushButton_clicked() {
    if (b == nullptr){
        b = new Box3D();
        mainWindow->pushObj(b, "Box", false);
    }

    std::ifstream myfile;
    myfile.open ("box.bin", std::ios::in | std::ios::binary);
    b->deserialize(myfile);
    myfile.close();
    updateBoxValues();
    mainWindow->updateGlCanvas();
}

void EngineManager::on_iterationsSlider_sliderMoved(int position) {
    if (iterations != nullptr){
        iterations->setVisibleBox(position);
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_energyIterationsButton_clicked() {
    if (iterations != nullptr){
        Box3D b = iterations->getBox(ui->iterationsSlider->value());
        double energy = e.energy(b);
        Eigen::VectorXd gradient(6);
        e.gradientTricubicInterpolationEnergy(gradient, b.getMin(), b.getMax());
        std::cerr << "Gradient: \n" << gradient << "\n";
        updateLabel(energy, ui->energyIterationLabel);
    }
}

void EngineManager::on_createBoxesPushButton_clicked() {
    if (d!=nullptr){
        deleteDrawableObject(solutions);
        solutions = new BoxList();
        //Engine::calculateInitialBoxes(*solutions, *d, Eigen::Matrix3d::Identity(), true, XYZ[ui->targetComboBox->currentIndex()]);
        Dcel copy = *d;
        Eigen::Matrix3d m = Eigen::Matrix3d::Identity();

        Engine::calculateInitialBoxes(*solutions, copy, m);
        ui->showAllSolutionsCheckBox->setEnabled(true);
        solutions->setVisibleBox(0);
        ui->solutionsSlider->setEnabled(true);
        ui->solutionsSlider->setMaximum(solutions->getNumberBoxes()-1);
        ui->setFromSolutionSpinBox->setValue(0);
        ui->setFromSolutionSpinBox->setMaximum(solutions->getNumberBoxes()-1);
        mainWindow->pushObj(solutions, "Solutions");
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_showAllSolutionsCheckBox_stateChanged(int arg1) {
    if (arg1 == Qt::Checked){
        ui->solutionsSlider->setEnabled(false);
        ui->solutionsSlider->setValue(0);
        solutions->setCylinders(true);
        solutions->setVisibleBox(-1);
    }
    else {
        ui->solutionsSlider->setEnabled(true);
        ui->solutionsSlider->setValue(0);
        solutions->setCylinders(false);
        solutions->setVisibleBox(0);
    }
    ui->solutionNumberLabel->setNum((int)solutions->getNumberBoxes());
    mainWindow->updateGlCanvas();
}

void EngineManager::on_solutionsSlider_valueChanged(int value) {
    if (ui->solutionsSlider->isEnabled()){
        solutions->setVisibleBox(value);
        ui->setFromSolutionSpinBox->setValue(value);
        ui->solutionNumberLabel->setText(QString::number(value));
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_minimizeAllPushButton_clicked() {
    if (solutions != nullptr){
        Engine::expandBoxes(*solutions, *g);
    }
}

void EngineManager::on_setFromSolutionButton_clicked() {
    if (solutions != nullptr){
        unsigned int value = ui->setFromSolutionSpinBox->value();
        if (value < solutions->getNumberBoxes()){
            if (b == nullptr){
                b = new Box3D(solutions->getBox(value));
                mainWindow->pushObj(b, "Box");
            }
            else {
                b->setMin(solutions->getBox(value).getMin());
                b->setMax(solutions->getBox(value).getMax());
                b->setConstraint1(solutions->getBox(value).getConstraint1());
                b->setConstraint2(solutions->getBox(value).getConstraint2());
                b->setConstraint3(solutions->getBox(value).getConstraint3());
                b->setColor(solutions->getBox(value).getColor());
                b->setRotationMatrix(solutions->getBox(value).getRotationMatrix());
                b->setTarget(solutions->getBox(value).getTarget());
            }
            //deleteDrawableObject(b);
            //solutions->getBox(value);
            //b = new Box3D(solutions->getBox(value));
            //mainWindow->pushObj(b, "Box");
            mainWindow->updateGlCanvas();
        }

    }
}

void EngineManager::on_wireframeDcelCheckBox_stateChanged(int arg1) {
    if (d!=nullptr && ui->inputMeshRadioButton->isChecked()){
        d->setWireframe(arg1 == Qt::Checked);
        mainWindow->updateGlCanvas();
    }
    if (baseComplex!=nullptr && ui->baseComplexRadioButton->isChecked()) {
        baseComplex->setWireframe(arg1 == Qt::Checked);
        mainWindow->updateGlCanvas();
    }
    if (he != nullptr && ui->heightfieldsRadioButton->isChecked()){
        he->setWireframe(arg1 == Qt::Checked);
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_pointsDcelRadioButton_toggled(bool checked) {
    if (d!=nullptr && ui->inputMeshRadioButton->isChecked()){
        if (checked){
            d->setPointsShading();
            mainWindow->updateGlCanvas();
        }
    }
    if (baseComplex!=nullptr && ui->baseComplexRadioButton->isChecked()) {
        if (checked){
            baseComplex->setPointShading();
            mainWindow->updateGlCanvas();
        }
    }
    if (he != nullptr && ui->heightfieldsRadioButton->isChecked()){
        if (checked){
            he->setPointShading();
            mainWindow->updateGlCanvas();
        }
    }
}

void EngineManager::on_flatDcelRadioButton_toggled(bool checked) {
    if (d!=nullptr && ui->inputMeshRadioButton->isChecked()){
        if (checked){
            d->setFlatShading();
            mainWindow->updateGlCanvas();
        }
    }
    if (baseComplex!=nullptr && ui->baseComplexRadioButton->isChecked()) {
        if (checked){
            baseComplex->setFlatShading();
            mainWindow->updateGlCanvas();
        }
    }
    if (he != nullptr && ui->heightfieldsRadioButton->isChecked()){
        if (checked){
            he->setFlatShading();
            mainWindow->updateGlCanvas();
        }
    }
}

void EngineManager::on_smoothDcelRadioButton_toggled(bool checked) {
    if (d!=nullptr && ui->inputMeshRadioButton->isChecked()){
        if (checked){
            d->setSmoothShading();
            mainWindow->updateGlCanvas();
        }
    }
    if (baseComplex!=nullptr && ui->baseComplexRadioButton->isChecked()) {
        if (checked){
            baseComplex->setSmoothShading();
            mainWindow->updateGlCanvas();
        }
    }
    if (he != nullptr && ui->heightfieldsRadioButton->isChecked()){
        if (checked){
            he->setSmoothShading();
            mainWindow->updateGlCanvas();
        }
    }
}

void EngineManager::on_trianglesCoveredPushButton_clicked() {
    if (d!=nullptr && b != nullptr) {
        Eigen::Matrix3d m[ORIENTATIONS];
        Eigen::Matrix3d mb = b->getRotationMatrix();
        m[0] = Eigen::Matrix3d::Identity();
        getRotationMatrix(Vec3(0,0,-1), 0.785398, m[1]);
        getRotationMatrix(Vec3(-1,0,0), 0.785398, m[2]);
        getRotationMatrix(Vec3(0,-1,0), 0.785398, m[3]);
        Dcel dd = *d;
        std::list<const Dcel::Face*> covered;
        if (mb == m[0]){
            CGALInterface::AABBTree t(dd);
            t.getIntersectedPrimitives(covered, *b);
        }
        else if (mb == m[1]){

            Eigen::Matrix3d mm;
            getRotationMatrix(Vec3(0,0,1), 0.785398, mm);
            dd.rotate(mm);
            CGALInterface::AABBTree t(dd);
            t.getIntersectedPrimitives(covered, *b);
        }
        else if (mb == m[2]){
            Eigen::Matrix3d mm;
            getRotationMatrix(Vec3(1,0,0), 0.785398, mm);
            dd.rotate(mm);
            CGALInterface::AABBTree t(dd);
            t.getIntersectedPrimitives(covered, *b);
        }
        else if (mb == m[3]){
            Eigen::Matrix3d mm;
            getRotationMatrix(Vec3(0,1,0), 0.785398, mm);
            dd.rotate(mm);
            CGALInterface::AABBTree t(dd);
            t.getIntersectedPrimitives(covered, *b);
        }
        else assert(0);


        std::list<const Dcel::Face*>::iterator i = covered.begin();
        while (i != covered.end()) {
            const Dcel::Face* f = *i;
            Pointd p1 = f->getVertex1()->getCoordinate(), p2 = f->getVertex2()->getCoordinate(), p3 = f->getVertex3()->getCoordinate();

            if (!b->isIntern(p1) || !b->isIntern(p2) || !b->isIntern(p3)) {
                i =covered.erase(i);
            }
            else ++i;
        }


        for (std::list<const Dcel::Face*>::iterator it = covered.begin(); it != covered.end(); ++it){
            const Dcel::Face* cf = *it;
            Dcel::Face* f = d->getFace(cf->getId());
            f->setColor(QColor(0,0,255));
        }
        d->update();

        mainWindow->updateGlCanvas();

    }
}

void EngineManager::on_deleteBoxesPushButton_clicked() {
    if (solutions!= nullptr && d != nullptr){
        Engine::deleteBoxesMemorySafe(*solutions, *d);
        solutions->setVisibleBox(0);
        ui->solutionsSlider->setMaximum(solutions->getNumberBoxes()-1);
        ui->setFromSolutionSpinBox->setValue(0);
        ui->setFromSolutionSpinBox->setMaximum(solutions->getNumberBoxes()-1);
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_deserializePreprocessingPushButton_clicked() {
    QString filename = QFileDialog::getOpenFileName(nullptr,
                       "Deserialize",
                       ".",
                       "BIN(*.bin)");

    if (!filename.isEmpty()) {
        deleteDrawableObject(d);
        d = new DrawableDcel();

        std::ifstream myfile;
        myfile.open (filename.toStdString(), std::ios::in | std::ios::binary);
        bool heightfields;

        d->deserialize(myfile);
        for (Dcel::FaceIterator fit = d->faceBegin(); fit != d->faceEnd(); ++fit)
            (*fit)->setColor(QColor(128,128,128));
        d->setWireframe(true);
        d->setPointsShading();
        d->update();
        mainWindow->pushObj(d, "Scaled Mesh");

        Serializer::deserialize(heightfields, myfile);
        //heightfields = true;
        if (!heightfields){
            Grid g[ORIENTATIONS];
            BoxList bl[ORIENTATIONS];
            for (unsigned int i = 0; i < ORIENTATIONS; i++){
                g[i].deserialize(myfile);
                bl[i].deserialize(myfile);
            }
            myfile.close();

            deleteDrawableObject(this->g);
            this->g = new DrawableGrid();
            *(this->g) = g[0];
            mainWindow->pushObj(this->g, "Grid");

            deleteDrawableObject(solutions);
            solutions = new BoxList();

            //caricamento solutions
            for (unsigned int i = 0; i < ORIENTATIONS; i++){
                solutions->insert(bl[i]);
            }

            solutions->setVisibleBox(0);
            solutions->setCylinders(false);
            mainWindow->pushObj(solutions, "Solutions");
            ui->showAllSolutionsCheckBox->setEnabled(true);
            ui->solutionsSlider->setEnabled(true);
            ui->solutionsSlider->setMaximum(solutions->getNumberBoxes()-1);
            ui->setFromSolutionSpinBox->setValue(0);
            ui->setFromSolutionSpinBox->setMaximum(solutions->getNumberBoxes()-1);

        }
        else {
            Grid g[ORIENTATIONS][TARGETS];
            BoxList bl[ORIENTATIONS][TARGETS];
            for (unsigned int i = 0; i < ORIENTATIONS; ++i){
                for (unsigned j = 0; j < TARGETS; ++j){
                    g[i][j].deserialize(myfile);
                    bl[i][j].deserialize(myfile);
                }
            }
            myfile.close();

            deleteDrawableObject(this->g);
            this->g = new DrawableGrid();
            *(this->g) = g[0][0];
            mainWindow->pushObj(this->g, "Grid");

            deleteDrawableObject(solutions);
            solutions = new BoxList();

            //caricamento solutions
            for (unsigned int i = 0; i < ORIENTATIONS; i++){
                for (unsigned int j = 0; j < TARGETS; ++j){
                    solutions->insert(bl[i][j]);
                }
            }

            solutions->setVisibleBox(0);
            solutions->setCylinders(false);
            mainWindow->pushObj(solutions, "Solutions");
            ui->showAllSolutionsCheckBox->setEnabled(true);
            ui->solutionsSlider->setEnabled(true);
            ui->solutionsSlider->setMaximum(solutions->getNumberBoxes()-1);
            ui->setFromSolutionSpinBox->setValue(0);
            ui->setFromSolutionSpinBox->setMaximum(solutions->getNumberBoxes()-1);
        }
        e = Energy(*g);
    }
    mainWindow->updateGlCanvas();
}

void EngineManager::on_serializePreprocessingPushButton_clicked() {
    QString filename = QFileDialog::getSaveFileName(nullptr,
                       "Serialize",
                       ".",
                       "BIN(*.bin)");
    if (!filename.isEmpty()) {
        Dcel scaled[ORIENTATIONS];
        Eigen::Matrix3d m[ORIENTATIONS];
        bool heightfields = ui->heightfieldsCheckBox->isChecked();
        scaled[0] = (Dcel)*d;
        m[0] = Eigen::Matrix3d::Identity();
        for (unsigned int i = 1; i < ORIENTATIONS; i++){
            scaled[i] = (Dcel)*d;
            m[i] = Engine::rotateDcelAlreadyScaled(scaled[i], i);
        }
        if (! heightfields){
            Grid g[ORIENTATIONS];
            BoxList bl[ORIENTATIONS];
            for (unsigned int i = 0; i < ORIENTATIONS; i++){
                Engine::generateGrid(g[i], scaled[i], ui->distanceSpinBox->value(), false);
                Engine::calculateInitialBoxes(bl[i],scaled[i], m[i], false);

            }
            std::ofstream myfile;
            myfile.open (filename.toStdString(), std::ios::out | std::ios::binary);
            scaled[0].serialize(myfile);
            Serializer::serialize(heightfields, myfile);
            for (unsigned int i = 0; i < ORIENTATIONS; i++){
                g[i].serialize(myfile);
                bl[i].serialize(myfile);
            }

            myfile.close();
        }
        else {
            Grid g[ORIENTATIONS][TARGETS];
            BoxList bl[ORIENTATIONS][TARGETS];
            for (unsigned int i = 0; i < ORIENTATIONS; ++i){
                for (unsigned j = 0; j < TARGETS; ++j){
                    Engine::generateGrid(g[i][j], scaled[i], ui->distanceSpinBox->value(), true, XYZ[j]);
                    Engine::calculateInitialBoxes(bl[i][j],scaled[i], m[i], true, XYZ[j]);
                }
            }
            std::ofstream myfile;
            myfile.open (filename.toStdString(), std::ios::out | std::ios::binary);
            scaled[0].serialize(myfile);
            Serializer::serialize(heightfields, myfile);
            for (unsigned int i = 0; i < ORIENTATIONS; i++){
                for (unsigned j = 0; j < TARGETS; ++j){
                    g[i][j].serialize(myfile);
                    bl[i][j].serialize(myfile);
                }
            }
            myfile.close();
        }
    }
}

void EngineManager::on_stepDrawGridSpinBox_valueChanged(double arg1) {
    if (g!=nullptr){
        if (arg1 > 0){
            g->setStepDrawGrid(arg1);
            mainWindow->updateGlCanvas();
        }
    }
}

void EngineManager::on_baseComplexPushButton_clicked() {
    if (d != nullptr){
        IGLMesh m = (Dcel)*d;
        baseComplex = new DrawableIGLMesh(m);
        mainWindow->pushObj(baseComplex, "Base Complex");
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_subtractPushButton_clicked() {
    if (solutions!= nullptr && baseComplex != nullptr && d != nullptr){
        CGALInterface::AABBTree aabb(*d, true);
        deleteDrawableObject(he);
        he = new HeightfieldsList();
        mainWindow->pushObj(he, "Heightfields");
        mainWindow->updateGlCanvas();
        he->resize(solutions->getNumberBoxes());
        SimpleIGLMesh bc((SimpleIGLMesh)*baseComplex);
        Timer timer("Boolean Operations");
        for (unsigned int i = 0; i <solutions->getNumberBoxes() ; i++){
        //for (int i = solutions->getNumberBoxes()-1; i >= 0 ; i--){
            bool b = true;
            for (unsigned int j = 0; j < bc.getNumberVertices(); j++) {
                Pointd p = bc.getVertex(j);
                double dist = aabb.getSquaredDistance(p);
                if (dist == 0) {
                    b = false;
                    break;
                }
            }
            if (b){
                std::cerr << "Finished! \n";
                break;
            }
            SimpleIGLMesh box;
            SimpleIGLMesh intersection;
            solutions->getBox(i).getIGLMesh(box);
            SimpleIGLMesh::intersection(intersection, bc, box);
            b = true;
            for (unsigned int j = 0; j < intersection.getNumberVertices(); j++) {
                Pointd p = intersection.getVertex(j);
                double dist = aabb.getSquaredDistance(p);
                if (dist == 0) {
                    b = false;
                    break;
                }
            }
            if (!b) {
                SimpleIGLMesh::difference(bc, bc, box);
                DrawableIGLMesh dimm(intersection);
                he->addHeightfield(dimm, solutions->getBox(i).getRotatedTarget(), i);
            }
            std::cerr << i << "\n";
        }
        timer.stopAndPrint();
        for (int i = he->getNumHeightfields()-1; i >= 0 ; i--){
            if (he->getNumberVerticesHeightfield(i) == 0){
                he->removeHeightfield(i);
                solutions->removeBox(i);
            }
        }
        ui->showAllSolutionsCheckBox->setEnabled(true);
        solutions->setVisibleBox(0);
        ui->heightfieldsSlider->setMaximum(he->getNumHeightfields()-1);
        ui->allHeightfieldsCheckBox->setChecked(true);
        ui->solutionsSlider->setEnabled(true);
        ui->solutionsSlider->setMaximum(solutions->getNumberBoxes()-1);
        ui->setFromSolutionSpinBox->setValue(0);
        ui->setFromSolutionSpinBox->setMaximum(solutions->getNumberBoxes()-1);
        mainWindow->deleteObj(baseComplex);
        delete baseComplex;
        baseComplex = new DrawableIGLMesh(bc);
        baseComplex->updateFaceNormals();
        for (unsigned int i = 0; i < baseComplex->getNumberFaces(); ++i){
            Vec3 n = baseComplex->getNormal(i);
            n.normalize();
            QColor c = colorOfNearestNormal(n);
            baseComplex->setColor(c.redF(), c.greenF(), c.blueF(), i);
        }
        mainWindow->pushObj(baseComplex, "Base Complex");
        mainWindow->updateGlCanvas();
        baseComplex->saveOnObj("BaseComplex.obj");
    }
}

void EngineManager::on_stickPushButton_clicked() {
    if (d!=nullptr && baseComplex != nullptr && he != nullptr && solutions != nullptr){
        CGALInterface::AABBTree aabb(*d, true);
        for (unsigned int i = 0; i < he->getNumHeightfields(); i++){
            bool b = true;
            for (unsigned int j = 0; j < he->getNumberVerticesHeightfield(i); j++) {
                Pointd p = he->getVertexOfHeightfield(i,j);
                double dist = aabb.getSquaredDistance(p);
                if (dist == 0) b = false;
            }
            if (b) {
                std::cerr << "Heightfield eliminabile\n";
                IGLMesh::unionn(*baseComplex, *baseComplex, he->getHeightfield(i));
                he->removeHeightfield(i);
                solutions->removeBox(i);
                i--;
            }
        }
    }
    for (unsigned int i = 0; i < baseComplex->getNumberFaces(); ++i){
        Vec3 n = baseComplex->getNormal(i);
        n.normalize();
        QColor c = colorOfNearestNormal(n);
        baseComplex->setColor(c.redF(), c.greenF(), c.blueF(), i);
    }
    ui->showAllSolutionsCheckBox->setEnabled(true);
    solutions->setVisibleBox(0);
    ui->solutionsSlider->setEnabled(true);
    ui->solutionsSlider->setMaximum(solutions->getNumberBoxes()-1);
    ui->heightfieldsSlider->setMaximum(he->getNumHeightfields()-1);
    ui->setFromSolutionSpinBox->setValue(0);
    ui->setFromSolutionSpinBox->setMaximum(solutions->getNumberBoxes()-1);
    mainWindow->updateGlCanvas();
}

void EngineManager::on_serializeBCPushButton_clicked() {
    if (baseComplex != nullptr && solutions != nullptr && d != nullptr && he != nullptr){
        QString filename = QFileDialog::getSaveFileName(nullptr,
                           "Serialize",
                           ".",
                           "BIN(*.bin)");
        if (!filename.isEmpty()) {
            std::ofstream myfile;
            myfile.open (filename.toStdString(), std::ios::out | std::ios::binary);
            d->serialize(myfile);
            solutions->serialize(myfile);
            baseComplex->serialize(myfile);
            he->serialize(myfile);
            std::vector<IGLMesh> hf;
            Serializer::serialize(hf, myfile);
            myfile.close();
        }
    }

}

void EngineManager::on_deserializeBCPushButton_clicked() {
    QString filename = QFileDialog::getOpenFileName(nullptr,
                       "Deserialize",
                       ".",
                       "BIN(*.bin)");

    if (!filename.isEmpty()) {
        deleteDrawableObject(d);
        deleteDrawableObject(solutions);
        deleteDrawableObject(baseComplex);
        deleteDrawableObject(he);
        d = new DrawableDcel();
        solutions = new BoxList();
        baseComplex = new DrawableIGLMesh();
        he = new HeightfieldsList();
        std::ifstream myfile;
        myfile.open (filename.toStdString(), std::ios::in | std::ios::binary);
        d->deserialize(myfile);
        solutions->deserialize(myfile);
        baseComplex->deserialize(myfile);
        he->deserialize(myfile);
        //manca gestione heightfields
        myfile.close();
        d->update();
        d->saveOnObjFile("sphere_sim3.obj");
        d->setPointsShading();
        d->setWireframe(true);
        mainWindow->pushObj(d, "Input Mesh");
        mainWindow->pushObj(solutions, "Boxes");
        mainWindow->pushObj(baseComplex, "Base Complex");
        mainWindow->pushObj(he, "Heightfields");
        mainWindow->updateGlCanvas();
        ui->showAllSolutionsCheckBox->setEnabled(true);
        solutions->setVisibleBox(0);
        ui->heightfieldsSlider->setMaximum(he->getNumHeightfields()-1);
        ui->allHeightfieldsCheckBox->setChecked(true);
        ui->solutionsSlider->setEnabled(true);
        ui->solutionsSlider->setMaximum(solutions->getNumberBoxes()-1);
        ui->setFromSolutionSpinBox->setValue(0);
        ui->setFromSolutionSpinBox->setMaximum(solutions->getNumberBoxes()-1);
        IGLMesh m(*d);
        m.saveOnObj("prova/model.obj");
        for (unsigned int i = 0; i < solutions->getNumberBoxes(); i++){
            SimpleIGLMesh bb;
            solutions->getBox(i).getIGLMesh(bb);
            std::stringstream ss;
            ss << "prova/box" << i << ".obj";
            bb.saveOnObj(ss.str());
        }
    }
}

void EngineManager::on_createAndMinimizeAllPushButton_clicked() {
    if (d!=nullptr){
        deleteDrawableObject(solutions);
        solutions = new BoxList();
        mainWindow->pushObj(solutions, "Solutions");
        double kernelDistance = ui->distanceSpinBox->value();
        Dcel scaled[ORIENTATIONS];
        Eigen::Matrix3d m[ORIENTATIONS];
        std::vector< std::tuple<int, Box3D, std::vector<bool> > > allVectorTriples;

        for (unsigned int i = 0; i < ORIENTATIONS; i++){
            scaled[i] = *d;
            m[i] = Engine::rotateDcelAlreadyScaled(scaled[i], i);
        }

        if (ui->onlyNearestTargetCheckBox->isChecked())
            Engine::setTrianglesTargets(scaled);

        if (!ui->heightfieldsCheckBox->isChecked()){
            Grid g[ORIENTATIONS];
            BoxList bl[ORIENTATIONS];
            std::set<int> coveredFaces;
            unsigned int numberFaces = 100;
            CGALInterface::AABBTree aabb0(scaled[0]);
            CGALInterface::AABBTree aabb1(scaled[1]);
            CGALInterface::AABBTree aabb2(scaled[2]);
            CGALInterface::AABBTree aabb3(scaled[3]);
            # pragma omp parallel for if(ORIENTATIONS>1)
            for (unsigned int i = 0; i < ORIENTATIONS; ++i){
                Engine::generateGrid(g[i], scaled[i], kernelDistance);
                g[i].resetSignedDistances();
                std::cerr << "Generated grid or " << i << "\n";
            }

            Timer t("Expanding all Boxes");
            while (coveredFaces.size() < scaled[0].getNumberFaces()){
                BoxList tmp[ORIENTATIONS];
                Eigen::VectorXi faces[ORIENTATIONS];
                for (unsigned int i = 0; i < ORIENTATIONS; i++){
                    IGLMesh m(scaled[i]);
                    IGLMesh dec;
                    bool b = m.getDecimatedMesh(dec, numberFaces, faces[i]);
                    std::cerr << "Decimated mesh " << i << ": " << (b ? "true" : "false") <<"\n";
                }

                for (unsigned int i = 0; i < ORIENTATIONS; i++){
                    std::cerr << "Calculating Boxes\n";
                    if (ui->onlyNearestTargetCheckBox->isChecked()){
                        Engine::calculateDecimatedBoxes(tmp[i], scaled[i], faces[i], coveredFaces, m[i], i);
                    }
                    else
                        Engine::calculateDecimatedBoxes(tmp[i], scaled[i], faces[i], coveredFaces, m[i]);
                    std::cerr << "Starting boxes growth\n";
                    Engine::expandBoxes(tmp[i], g[i], true);
                    std::cerr << "Orientation: " << i << " completed.\n";
                }

                for (unsigned int i = 0; i < ORIENTATIONS; ++i){

                        for (unsigned int k = 0; k < tmp[i].getNumberBoxes(); ++k){
                            std::list<const Dcel::Face*> list;
                            switch(i){
                                    case 0: aabb0.getIntersectedPrimitives(list, tmp[i].getBox(k)); break;
                                    case 1: aabb1.getIntersectedPrimitives(list, tmp[i].getBox(k)); break;
                                    case 2: aabb2.getIntersectedPrimitives(list, tmp[i].getBox(k)); break;
                                    case 3: aabb3.getIntersectedPrimitives(list, tmp[i].getBox(k)); break;
                            }
                            for (std::list<const Dcel::Face*>::iterator it = list.begin(); it != list.end(); ++it){
                                coveredFaces.insert((*it)->getId());
                            }
                        }
                }
                for (unsigned int i = 0; i < ORIENTATIONS; ++i){
                        bl[i].insert(tmp[i]);
                }

                std::cerr << "Starting Number Faces: " << numberFaces << "; Total Covered Faces: " << coveredFaces.size() << "\n";
                std::cerr << "Target: " << scaled[0].getNumberFaces() << "\n";
                numberFaces*=2;
                if (numberFaces > scaled[0].getNumberFaces())
                    numberFaces = scaled[0].getNumberFaces();
            }
            t.stopAndPrint();

            std::vector< std::tuple<int, Box3D, std::vector<bool> > > vectorTriples[ORIENTATIONS];
            for (unsigned int i = 0; i < ORIENTATIONS; i++){
                solutions->insert(bl[i]);
                Engine::createVectorTriples(vectorTriples[i], bl[i], scaled[i]);
                allVectorTriples.insert(allVectorTriples.end(), vectorTriples[i].begin(), vectorTriples[i].end());
            }

            Engine::deleteBoxes(*solutions, allVectorTriples, d->getNumberFaces());
        }
        else {
            Grid g[ORIENTATIONS][TARGETS];
            BoxList bl[ORIENTATIONS][TARGETS];
            std::set<int> coveredFaces;
            unsigned int numberFaces = 100;
            double angleTolerance = (double)ui->toleranceSlider->value()/100;
            double areaTolerance = ui->areaToleranceSpinBox->value();
            CGALInterface::AABBTree aabb0(scaled[0]);
            CGALInterface::AABBTree aabb1(scaled[1]);
            CGALInterface::AABBTree aabb2(scaled[2]);
            CGALInterface::AABBTree aabb3(scaled[3]);
            # pragma omp parallel for if(ORIENTATIONS>1)
            for (unsigned int i = 0; i < ORIENTATIONS; ++i){
                for (unsigned int j = 0; j < TARGETS; ++j) {
                    std::set<const Dcel::Face*> flippedFaces, savedFaces;
                    Engine::getFlippedFaces(flippedFaces, savedFaces, scaled[i], XYZ[j], angleTolerance, areaTolerance);
                    Engine::generateGrid(g[i][j], scaled[i], kernelDistance, true, XYZ[j], savedFaces);
                    g[i][j].resetSignedDistances();
                    std::cerr << "Generated grid or " << i << " t " << j << "\n";
                }
            }

            while (coveredFaces.size() < scaled[0].getNumberFaces()){
                BoxList tmp[ORIENTATIONS][TARGETS];
                Eigen::VectorXi faces[ORIENTATIONS];
                for (unsigned int i = 0; i < ORIENTATIONS; i++){
                    IGLMesh m(scaled[i]);
                    IGLMesh dec;
                    bool b = m.getDecimatedMesh(dec, numberFaces, faces[i]);
                    std::cerr << "Decimated mesh " << i << ": " << b <<"\n";
                }
                for (unsigned int i = 0; i < ORIENTATIONS; ++i){
                    for (unsigned int j = 0; j < TARGETS; ++j){
                        std::cerr << "Calculating Boxes\n";
                        if (ui->onlyNearestTargetCheckBox->isChecked()){
                            Engine::calculateDecimatedBoxes(tmp[i][j],scaled[i], faces[i], coveredFaces, m[i], i, true, XYZ[j]);
                        }
                        else
                            Engine::calculateDecimatedBoxes(tmp[i][j],scaled[i], faces[i], coveredFaces, m[i], -1, true, XYZ[j]);
                        std::cerr << "Starting boxes growth\n";
                        Engine::expandBoxes(tmp[i][j], g[i][j]);
                        std::cerr << "Orientation: " << i << " Target: " << j << " completed.\n";
                    }
                }

                for (unsigned int i = 0; i < ORIENTATIONS; ++i){
                    for (unsigned int j = 0; j < TARGETS; ++j){
                        for (unsigned int k = 0; k < tmp[i][j].getNumberBoxes(); ++k){
                            std::list<const Dcel::Face*> list;
                            switch(i){
                                    case 0: aabb0.getIntersectedPrimitives(list, tmp[i][j].getBox(k)); break;
                                    case 1: aabb1.getIntersectedPrimitives(list, tmp[i][j].getBox(k)); break;
                                    case 2: aabb2.getIntersectedPrimitives(list, tmp[i][j].getBox(k)); break;
                                    case 3: aabb3.getIntersectedPrimitives(list, tmp[i][j].getBox(k)); break;
                            }
                            for (std::list<const Dcel::Face*>::iterator it = list.begin(); it != list.end(); ++it){
                                coveredFaces.insert((*it)->getId());
                            }
                        }
                    }
                }
                for (unsigned int i = 0; i < ORIENTATIONS; ++i){
                    for (unsigned int j = 0; j < TARGETS; ++j){
                        bl[i][j].insert(tmp[i][j]);
                    }
                }

                std::cerr << "Starting Number Faces: " << numberFaces << "; Total Covered Faces: " << coveredFaces.size() << "\n";
                std::cerr << "Target: " << scaled[0].getNumberFaces() << "\n";
                numberFaces*=2;
                if (numberFaces > scaled[0].getNumberFaces())
                    numberFaces = scaled[0].getNumberFaces();
            }

            std::vector< std::tuple<int, Box3D, std::vector<bool> > > vectorTriples[ORIENTATIONS][TARGETS];
            for (unsigned int i = 0; i < ORIENTATIONS; i++){
                for (unsigned int j = 0; j < TARGETS; ++j){
                    solutions->insert(bl[i][j]);
                    Engine::createVectorTriples(vectorTriples[i][j], bl[i][j], scaled[i]);
                    allVectorTriples.insert(allVectorTriples.end(), vectorTriples[i][j].begin(), vectorTriples[i][j].end());
                }
            }

            Engine::deleteBoxes(*solutions, allVectorTriples, d->getNumberFaces());
        }
        ui->showAllSolutionsCheckBox->setEnabled(true);
        solutions->setVisibleBox(0);
        ui->solutionsSlider->setEnabled(true);
        ui->solutionsSlider->setMaximum(solutions->getNumberBoxes()-1);
        ui->setFromSolutionSpinBox->setValue(0);
        ui->setFromSolutionSpinBox->setMaximum(solutions->getNumberBoxes()-1);
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_allHeightfieldsCheckBox_stateChanged(int arg1) {
    if (arg1 == Qt::Checked){
        if (he != nullptr){
            he->setVisibleHeightfield(-1);
            mainWindow->updateGlCanvas();
        }
    }
    else {
        if (he != nullptr){
            he->setVisibleHeightfield(ui->heightfieldsSlider->value());
            mainWindow->updateGlCanvas();
        }
    }
}

void EngineManager::on_heightfieldsSlider_valueChanged(int value) {
    if (he != nullptr){
        ui->solutionsSlider->setValue(value);
        he->setVisibleHeightfield(value);
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_minXSpinBox_valueChanged(double arg1) {
    if (b!=nullptr){
        b->setMinX(arg1);
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_minYSpinBox_valueChanged(double arg1) {
    if (b!=nullptr){
        b->setMinY(arg1);
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_minZSpinBox_valueChanged(double arg1) {
    if (b!=nullptr){
        b->setMinZ(arg1);
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_maxXSpinBox_valueChanged(double arg1) {
    if (b!=nullptr){
        b->setMaxX(arg1);
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_maxYSpinBox_valueChanged(double arg1) {
    if (b!=nullptr){
        b->setMaxY(arg1);
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_maxZSpinBox_valueChanged(double arg1) {
    if (b!=nullptr){
        b->setMaxZ(arg1);
        mainWindow->updateGlCanvas();
    }
}

void EngineManager::on_toleranceSlider_valueChanged(int value) {
    if (d!= nullptr){
        updateColors(value, ui->areaToleranceSpinBox->value());
    }
}

void EngineManager::on_areaToleranceSpinBox_valueChanged(double arg1){
    if (d!= nullptr){
        updateColors(ui->toleranceSlider->value(), arg1);
    }
}
