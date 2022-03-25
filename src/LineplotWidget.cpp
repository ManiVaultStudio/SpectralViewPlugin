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

void LineplotWidget::setData(std::vector<float>& yVals, std::vector<float>& confIntervalLeft, std::vector<float>& confIntervalRight, std::vector<QString>& dimNames, const int numDimensions, std::string dataOrigin)
{
    std::string _jsonObject = "";

    //qDebug() << "Setting data";

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

    //qDebug() << _jsonObject.c_str();

    if (dataOrigin == "selection") {
        emit _communicationObject->qt_setData(QString(_jsonObject.c_str()));
    }
    else if (dataOrigin == "subset" || dataOrigin == "list") {
        emit _communicationObject->qt_setEndmember(QString(_jsonObject.c_str()));
    }
}

void LineplotWidget::setEndmemberColor(QColor endmemberColor, int row) {

    int r = endmemberColor.red();
    int g = endmemberColor.green();
    int b = endmemberColor.blue();

    emit _communicationObject->qt_setEndmemberColor(r, g, b, row);
}

void LineplotWidget::setEndmemberVisibility(bool toggled, int row) {
    emit _communicationObject->qt_setEndmemberVisibility(toggled, row);
}

void LineplotWidget::setEndmemberRemoved(int row) {
    emit _communicationObject->qt_setEndmemberRemoved(row);
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