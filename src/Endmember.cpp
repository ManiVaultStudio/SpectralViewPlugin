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

Endmember::Endmember(LineplotPlugin& lineplotPlugin, const Dataset<DatasetImpl>& dataset) :
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

void Endmember::sendData(hdps::Dataset<DatasetImpl>& dataset, std::string dataOrigin) {

    auto type = dataset->getDataType();

    if (type == PointType) {
        auto points = dataset.get<Points>();

        if (dataOrigin == "subset") {
            auto parent = dataset->getParent();
            auto parentPoints = parent->getSourceDataset<Points>();
            //auto source = dataset->getSourceDataset<Points>();
            auto noPoints = points->getNumPoints();
            auto indices = points->indices;

            _lineplotPlugin.computeAverageSpectrum(parentPoints, noPoints, indices, "subset");
        }
        else if (dataOrigin == "list") {

            auto numDimensions = points->getNumDimensions();
            auto names = points->getDimensionNames();
            std::vector<float> spectrum;

            for (int v = 0; v < numDimensions; v++) {
                spectrum.push_back(points->getValueAt(v));
            }

            std::vector<float> confIntervalLeft(numDimensions);
            std::vector <float> confIntervalRight(numDimensions);

            _lineplotPlugin.getLineplotWidget().setData(spectrum, confIntervalLeft, confIntervalRight, names, numDimensions, "list");
        }
    }
    else if (type == ClusterType) {

        auto parent = dataset->getParent();
        auto clusters = dataset.get<Clusters>()->getClusters();
        auto noClusters = clusters.length();

        for (int i = 0; i < noClusters; i++) {
             auto indices = clusters[i].getIndices();
             auto noPoints = clusters[i].getNumberOfIndices();

             _lineplotPlugin.computeAverageSpectrum(parent, noPoints, indices, "cluster");
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