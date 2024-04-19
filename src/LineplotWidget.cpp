#include "LineplotWidget.h"

#include "Application.h"

#include <cassert>
#include <cstdlib>
#include <regex>
#include <string>
#include <utility>

#include <QColor>
#include <QMouseEvent>

LinePlotCommunicationObject::LinePlotCommunicationObject(LineplotWidget* parent) :
    mv::gui::WebCommunicationObject(),
    _parent(parent)
{

}

void LinePlotCommunicationObject::js_setRGBWavelength(float wavelength, int index)
{
    _parent->js_setRGBWavelength(wavelength, index);
}

LineplotWidget::LineplotWidget() :
    mv::gui::WebWidget(),
    _communicationObject(new LinePlotCommunicationObject(this)),
    dataOptionBuffer(),
    loaded(false)
{
    Q_INIT_RESOURCE(lineplot_resources);

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

void LineplotWidget::setData(const std::vector<float>& yVals, const std::vector<float>& confIntervalLeft, const std::vector<float>& confIntervalRight, const std::vector<QString>& dimNames, const int numDimensions, const std::string& dataOrigin)
{
    assert(dimNames.size() == numDimensions);

    // Check dimension names before parsing data
    std::vector<QString> numericDimNames;

    // check if dimension name contains a) only numbers or "." b) numbers and trailing units c) text
    // if a) use the number b) remove the unit c) replace names with numeric dimension count
    // https://godbolt.org/z/8Yeej4cj6
    auto determineNumberAndExtract = [](const std::string & input) -> std::pair<bool, std::string> {
        std::regex pattern(R"(^\D*?\s*?(\d+(\.\d+)?)\D*?$)");
        std::smatch match;

        if (std::regex_search(input, match, pattern) && match.size() > 1)
            return std::make_pair(true, match[1]);

        return std::make_pair(false, "");
    };

    // add a decimal point if there is none
    auto unsureFloatString = [](std::string& input) -> void {
        // Regular expressions to match float numbers
        std::regex decimalRegex("([0-9]+)\\.([0-9]+)");
        std::smatch match;

        // append missing decimal point
        if (!std::regex_match(input, match, decimalRegex))
            input.append(".0");

        // remove leading 0s
        if (!(input.size() > 2 && input[0] == '0' && input[1] == '.' && isdigit(input[2])))
            input.erase(0, input.find_first_not_of('0'));
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

        std::string extractedDimName = res.second;
        unsureFloatString(extractedDimName);

        numericDimNames.push_back(QString::fromStdString(extractedDimName));
    }

    if (replaceAllNames)
    {
        numericDimNames.resize(numDimensions);
        for (size_t i = 0; i < numericDimNames.size(); i++)
            numericDimNames[i] = QString::number(i) + ".0";
    }

    // create json string that will be passed to js
    QString jsonObject = "[\n";

    for (int i = 0; i < numDimensions; i++) {
        jsonObject +=
            "{ \"x\": " + 
            numericDimNames.at(i) +
            ", \"y\": " + 
            QString::number(yVals.at(i)) +
            ", \"CI_Left\": " + 
            QString::number(confIntervalLeft.at(i)) +
            ", \"CI_Right\": " + 
            QString::number(confIntervalRight.at(i)) +
            " }";

        if (i < numDimensions - 1)
            jsonObject += ",\n";
    }

    jsonObject += "\n]";

    if (dataOrigin == "selection") {
        emit _communicationObject->qt_setData(jsonObject);
    }
    else if (dataOrigin == "endmember") {
        emit _communicationObject->qt_setEndmember(jsonObject);
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

void LineplotWidget::setSelectionVisibility(bool toggled) {
    emit _communicationObject->qt_setSelectionVisibility(toggled);
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
