#include "Endmember.h"
#include "SpectralViewPlugin.h"
#include "SettingsAction.h"
#include "DataHierarchyItem.h"

#include <util/Exception.h>
#include <ClusterData/ClusterData.h>

#include <QPainter>
#include <QFontMetrics>
#include <QDebug>

#include <set>

Endmember::Endmember(SpectralViewPlugin& spectralViewPlugin, const Dataset<DatasetImpl>& dataset, int index) :
    WidgetAction(&spectralViewPlugin),
    _spectralViewPlugin(spectralViewPlugin),
    _active(false),
    _dataset(dataset),
    _data(),
    //_indices(),
    _generalAction(*this, index),
    _mapAction(*this)

{
    setText("Endmember");
    setObjectName("Endmember");

    _index = index;

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

int Endmember::getIndex() {
    return _index;
}

/*
void Endmember::setIndices(std::vector<unsigned int> indices) {
    _indices = indices;
}
*/

void Endmember::sendColor(QColor endmemberColor, int row) {
    
    _spectralViewPlugin.getLineplotWidget().setEndmemberColor(endmemberColor, row);
}

void Endmember::updateVisibility(bool toggled, int row) {
    _spectralViewPlugin.getLineplotWidget().setEndmemberVisibility(toggled, row);
}

void Endmember::sendEndmemberRemoved(int row) {
    _spectralViewPlugin.getLineplotWidget().setEndmemberRemoved(row);
}

void Endmember::highlightSelection(int row) {
    _spectralViewPlugin.getLineplotWidget().setHighlightSelection(row);
}

EndmembersAction& Endmember::getEndmembersAction()
{
    return _spectralViewPlugin.getSettingsAction().getEndmembersAction();
}

SpectralViewPlugin& Endmember::getSpectralViewPlugin()
{
    return _spectralViewPlugin;
}

void Endmember::computeMap(QString endmemberName, std::vector<float> endmemberData, float angle, int mapType, int algorithmType) {
    _spectralViewPlugin.updateMap(endmemberName, endmemberData, angle, mapType, algorithmType);
}

void Endmember::updateThresholdAngle(QString endmemberName, float threshold, int mapType, int algorithmType) {
    _spectralViewPlugin.updateThresholdAngle(endmemberName, threshold, mapType, algorithmType);
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
