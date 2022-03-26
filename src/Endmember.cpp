#include "Endmember.h"
#include "LineplotPlugin.h"
#include "SettingsAction.h"
#include "DataHierarchyItem.h"

#include <util/Exception.h>
#include <PointData.h>
#include <ClusterData.h>

#include <QPainter>
#include <QFontMetrics>
#include <QDebug>

#include <set>

Endmember::Endmember(LineplotPlugin& lineplotPlugin, const hdps::Dataset<Points>& dataset) :
    WidgetAction(&lineplotPlugin),
    EventListener(),
    _lineplotPlugin(lineplotPlugin),
    _active(false),
    _dataset(dataset),
    _generalAction(*this)
{
    setText("Endmember");
    setObjectName("Endmember");
    setEventCore(Application::core());

    if (!_dataset.isValid()) {
        throw std::runtime_error("The dataset is not valid after initialization");
    }

    // Update dataset name action when the images dataset GUI name changes
    connect(&_dataset, &Dataset<Points>::dataGuiNameChanged, this, [this](const QString& oldGuiName, const QString& newGuiName) {
        _generalAction.getDatasetNameAction().setString(newGuiName);
        _generalAction.getNameAction().setDefaultString(newGuiName);
        });

}

Endmember::~Endmember()
{

}

void Endmember::sendData(hdps::Dataset<Points>& dataset, std::string dataOrigin) {

    auto type = dataset->getDataType();

    if (type == PointType) {

        if (dataOrigin == "subset") {
            auto parent = dataset->getParent();
            auto parentPoints = parent->getSourceDataset<Points>();
            //auto source = dataset->getSourceDataset<Points>();
            auto noPoints = dataset->getNumPoints();
            auto indices = dataset->indices;

            _lineplotPlugin.computeAverageSpectrum(parentPoints, noPoints, indices, "subset");
        }
        else if (dataOrigin == "list") {

            auto numDimensions = dataset->getNumDimensions();
            auto names = dataset->getDimensionNames();
            std::vector<float> spectrum;

            for (int v = 0; v < numDimensions; v++) {
                spectrum.push_back(dataset->getValueAt(v));
            }

            std::vector<float> confIntervalLeft(numDimensions);
            std::vector <float> confIntervalRight(numDimensions);

            _lineplotPlugin.getLineplotWidget().setData(spectrum, confIntervalLeft, confIntervalRight, names, numDimensions, "list");
        }
    }
}

void Endmember::sendColor(QColor endmemberColor, int row) {
    
    _lineplotPlugin.getLineplotWidget().setEndmemberColor(endmemberColor, row);
}

void Endmember::updateVisibility(bool toggled, int row) {
    _lineplotPlugin.getLineplotWidget().setEndmemberVisibility(toggled, row);
}

void Endmember::sendEndmemberRemoved(int row) {
    _lineplotPlugin.getLineplotWidget().setEndmemberRemoved(row);
}

EndmembersAction& Endmember::getEndmembersAction()
{
    return _lineplotPlugin.getSettingsAction().getEndmembersAction();
}

LineplotPlugin& Endmember::getLineplotPlugin()
{
    return _lineplotPlugin;
}