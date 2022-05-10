#include "Endmember.h"
#include "LineplotPlugin.h"
#include "SettingsAction.h"
#include "DataHierarchyItem.h"

#include <util/Exception.h>
#include <ClusterData.h>

#include <QPainter>
#include <QFontMetrics>
#include <QDebug>

#include <set>

Endmember::Endmember(LineplotPlugin& lineplotPlugin, const Dataset<DatasetImpl>& dataset, int index) :
    WidgetAction(&lineplotPlugin),
    EventListener(),
    _lineplotPlugin(lineplotPlugin),
    _active(false),
    _dataset(dataset),
    _data(),
    //_indices(),
    _generalAction(*this, index),
    _mapAction(*this)

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

void Endmember::setData(std::vector<float> data) {
    _data = data;
}

/*
void Endmember::setIndices(std::vector<unsigned int> indices) {
    _indices = indices;
}
*/

void Endmember::sendColor(QColor endmemberColor, int row) {
    
    _lineplotPlugin.getLineplotWidget().setEndmemberColor(endmemberColor, row);
}

void Endmember::updateVisibility(bool toggled, int row) {
    _lineplotPlugin.getLineplotWidget().setEndmemberVisibility(toggled, row);
}

void Endmember::sendEndmemberRemoved(int row) {
    _lineplotPlugin.getLineplotWidget().setEndmemberRemoved(row);
}

void Endmember::highlightSelection(int row) {
    _lineplotPlugin.getLineplotWidget().setHighlightSelection(row);
}

EndmembersAction& Endmember::getEndmembersAction()
{
    return _lineplotPlugin.getSettingsAction().getEndmembersAction();
}

LineplotPlugin& Endmember::getLineplotPlugin()
{
    return _lineplotPlugin;
}

void Endmember::computeMap(std::vector<float> endmemberData, float angle, int mapType, int algorithmType) {
    _lineplotPlugin.updateMap(endmemberData, angle, mapType, algorithmType);
}

void Endmember::updateThresholdAngle(float threshold, int mapType, int algorithmType) {
    _lineplotPlugin.updateThresholdAngle(threshold, mapType, algorithmType);
}

std::vector<float> Endmember::resample(std::vector<float> parentDim) {

    float noDim = parentDim.size();
    auto endmemberDatasetPoints = _dataset.get<Points>();
    auto endmemberDimString = endmemberDatasetPoints->getDimensionNames();
    float noEndmemberDim = endmemberDatasetPoints->getNumDimensions();
    std::vector<float> endmemberDim(noEndmemberDim);

    std::vector<float> values(noDim);

    if (noEndmemberDim == noDim) {
        return _data;
    }
    else {

        for (int v = 0; v < noEndmemberDim; v++) {
            endmemberDim[v] = endmemberDimString[v].toFloat();
        }

        // equally spaced values
        float step = endmemberDim[1] - endmemberDim[0];
        float last = 0;

        for (int x = 0; x < noDim; x++) {
            for (int y = 1; y < noEndmemberDim; y++) {
                if (endmemberDim[y] == parentDim[x]) {
                    values[x] = _data[x];
                    break;
                }
                else if (endmemberDim[y] > parentDim[x]) {
                    float weight = (endmemberDim[y] - parentDim[x]) / step;
                    values[x] = weight * _data[y - 1] + (1 - weight) * _data[y];
                    last = y;
                    break;
                }
            }
        }
    }

    return values;
}