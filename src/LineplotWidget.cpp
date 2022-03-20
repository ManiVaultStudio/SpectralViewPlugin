#include "LineplotWidget.h"

#include "ClusterData.h"
#include "util/FileUtil.h"

#include <QVBoxLayout>

#include <cassert>
#include <iostream>


LinePlotCommunicationObject::LinePlotCommunicationObject(LineplotWidget* parent)
    :
    _parent(parent)
{

}

void LinePlotCommunicationObject::js_setRGBWavelengths(float wavelengthR, float wavelengthG, float wavelengthB)
{
    _parent->js_setRGBWavelengths(wavelengthR, wavelengthG, wavelengthB);
}

/*
void LinePlotCommunicationObject::js_selectData(QString text)
{
    _parent->js_selectData(text);
}

void LinePlotCommunicationObject::js_selectionUpdated(QVariant selectedClusters)
{
    _parent->js_selectionUpdated(selectedClusters);
}

void LinePlotCommunicationObject::js_highlightUpdated(int highlightId)
{
    _parent->js_highlightUpdated(highlightId);
}
*/

LineplotWidget::LineplotWidget(LineplotPlugin& lineplotPlugin) :
    loaded(false)
{
    Q_INIT_RESOURCE(lineplot_resources);
    _communicationObject = new LinePlotCommunicationObject(this);
    init(_communicationObject);
    
    setAcceptDrops(true);
    setMouseTracking(true);

    setWindowIcon(hdps::Application::getIconFont("FontAwesome").getIcon("chart-line"));
}

LineplotWidget::~LineplotWidget() {

}

void LineplotWidget::addDataOption(const QString option)
{
    if (loaded)
        emit _communicationObject->qt_addAvailableData(option);
    else
        dataOptionBuffer.append(option);
}

void LineplotWidget::setData(std::vector<float>& yVals, std::vector<float>& confIntervalLeft, std::vector<float>& confIntervalRight, std::vector<QString>& dimNames, const int numDimensions)
{
    std::string _jsonObject = "";

    qDebug() << "Setting data";

    std::string spectra;

    for (int i = 0; i < numDimensions; i++) {
        std::string yVal = std::to_string(yVals.at(i));
        std::string ci_left = std::to_string(confIntervalLeft.at(i));
        std::string ci_right = std::to_string(confIntervalRight.at(i));
        spectra = spectra + "{ \"x\": " + dimNames.at(i).toStdString() + ", \"y\": " + yVal +
                    ", \"CI_Left\": " + ci_left + ", \"CI_Right\": " + ci_right;

        if (i == numDimensions - 1)
            spectra = spectra + +" }";
        else
            spectra = spectra + " },\n";
    }

    _jsonObject = "[\n" + spectra + "\n]";

    // qDebug() << _jsonObject.c_str();

    emit _communicationObject->qt_setData(QString(_jsonObject.c_str()));
}

void LineplotWidget::enableRGBWavelengths(bool checkedRGB) {
    emit _communicationObject->qt_enableRGBWavelengths(checkedRGB);
}

void LineplotWidget::enableStdArea(bool checkedStd) {
    emit _communicationObject->qt_enableStdArea(checkedStd);
}

void LineplotWidget::mousePressEvent(QMouseEvent* event)
{
    // UNUSED
}

void LineplotWidget::mouseMoveEvent(QMouseEvent* event)
{
    // UNUSED
}

void LineplotWidget::mouseReleaseEvent(QMouseEvent* event)
{
    // UNUSED
}

void LineplotWidget::onSelection(QRectF selection)
{
    // UNUSED
}

void LineplotWidget::cleanup()
{

}

void LineplotWidget::initWebPage()
{
    loaded = true;

    for (QString option : dataOptionBuffer) {
       emit _communicationObject->qt_addAvailableData(option);
    }
    dataOptionBuffer.clear();
}

void LineplotWidget::js_setRGBWavelengths(float wavelengthR, float wavelengthG, float wavelengthB) {

    emit changeRGBWavelengths(wavelengthR, wavelengthG, wavelengthB);
}

/*
void LineplotWidget::js_selectData(QString name)
{
    emit dataSetPicked(name);
}

void LineplotWidget::js_highlightUpdated(int highlightId)
{
    QList<int> selectedClusters;

    for (int i = 0; i < _numClusters; i++) {
        selectedClusters.append(i == highlightId ? 1 : 0);
    }

    selectedClusters.append(highlightId);

    //emit clusterSelectionChanged(selectedClusters);
}

void LineplotWidget::js_selectionUpdated(QVariant selectedClusters)
{
    QList<QVariant> selectedClustersList = selectedClusters.toList();

    QList<int> selectedIndices;
    for (const QVariant& variant : selectedClustersList)
    {
        selectedIndices.push_back(variant.toInt());
    }

    emit clusterSelectionChanged(selectedIndices);
}
*/