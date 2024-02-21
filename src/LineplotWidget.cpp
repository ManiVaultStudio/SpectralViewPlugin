#include "LineplotWidget.h"

#include "ClusterData/ClusterData.h"
#include "util/FileUtil.h"

#include <QVBoxLayout>

#include <cassert>
#include <iostream>


LinePlotCommunicationObject::LinePlotCommunicationObject(LineplotWidget* parent)
    :
    _parent(parent)
{

}

void LinePlotCommunicationObject::js_setRGBWavelength(float wavelength, int index)
{
    _parent->js_setRGBWavelength(wavelength, index);
}

LineplotWidget::LineplotWidget(SpectralViewPlugin& spectralViewPlugin) :
    loaded(false)
{
    Q_INIT_RESOURCE(lineplot_resources);
    _communicationObject = new LinePlotCommunicationObject(this);
    init(_communicationObject);
    
    setAcceptDrops(true);
    setMouseTracking(true);

    setWindowIcon(mv::Application::getIconFont("FontAwesome").getIcon("chart-line"));
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
    assert(dimNames.size() == numDimensions);

    // Check dimension names before parsing data
    std::vector<std::string> numericDimNames;

    // check if dimension name contains a) only numbers or "." b) numbers and trailing units c) text
    // if a) use the number b) remove the unit c) replace names with numeric dimension count
    auto determineNumberAndExtract = [](const std::string & input) -> std::pair<bool, std::string> {
        std::regex numberRegex(R"(\d+(\.\d+)?(\s*[a-zA-Z]+)?)");
        std::smatch match;

        if (std::regex_match(input, match, numberRegex)) {
            std::string extractedNumbers = std::regex_replace(match[0].str(), std::regex(R"([^\d.])"), "");
            return std::make_pair(true, extractedNumbers);
        }
        else {
            return std::make_pair(false, "00");
        }
    };

    bool replaceAllNames = false;
    for (const auto& dimName : dimNames)
    {
        auto res = determineNumberAndExtract(dimName.toStdString());

        if (!res.first)
        {
            replaceAllNames = true;
            break;
        }

        numericDimNames.push_back(res.second);
    }

    if (replaceAllNames)
    {
        numericDimNames.resize(numDimensions);
        for (size_t i = 0; i < numericDimNames.size(); i++)
            numericDimNames[i] = std::to_string(i);
    }

    // create json string that will be passed to js
    std::string jsonObject = "[\n";

    for (int i = 0; i < numDimensions; i++) {
        jsonObject +=
            "{ \"x\": " + 
            numericDimNames.at(i) +
            ", \"y\": " + 
            std::to_string(yVals.at(i)) +
            ", \"CI_Left\": " + 
            std::to_string(confIntervalLeft.at(i)) +
            ", \"CI_Right\": " + 
            std::to_string(confIntervalRight.at(i)) +
            " }";

        if (i < numDimensions - 1)
            jsonObject += ",\n";
    }

    jsonObject += "\n]";

    if (dataOrigin == "selection") {
        emit _communicationObject->qt_setData(QString(jsonObject.c_str()));
    }
    else if (dataOrigin == "endmember") {
        emit _communicationObject->qt_setEndmember(QString(jsonObject.c_str()));
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

void LineplotWidget::setHighlightSelection(int row) {
    emit _communicationObject->qt_setHighlightSelection(row);
}

void LineplotWidget::enableRGBWavelengths(bool checkedRGB) {
    emit _communicationObject->qt_enableRGBWavelengths(checkedRGB);
}

void LineplotWidget::enableStdArea(bool checkedStd) {
    emit _communicationObject->qt_enableStdArea(checkedStd);
}

void LineplotWidget::updateRGBLine(float newWavelength, int index) {
    if (index == 0) {
        emit _communicationObject->qt_updateRedLine(newWavelength);
    }
    else if (index == 1) {
        emit _communicationObject->qt_updateGreenLine(newWavelength);
    }
    else if (index == 2) {
        emit _communicationObject->qt_updateBlueLine(newWavelength);
    }
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

void LineplotWidget::js_setRGBWavelength(float wavelength, int index) {
    emit changeRGBWavelengths(wavelength, index);
}
