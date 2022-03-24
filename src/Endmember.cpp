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

Endmember::Endmember(LineplotPlugin& lineplotPlugin, const hdps::Dataset<hdps::DatasetImpl>& dataset) :
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
}

Endmember::~Endmember()
{

}

void Endmember::sendData(hdps::Dataset<hdps::DatasetImpl>& dataset) {

    auto type = dataset->getDataType();

    if (type == PointType) {
        auto parent = dataset->getParent();
        auto parentPoints = parent->getSourceDataset<Points>();
        auto source = dataset->getSourceDataset<Points>();
        auto noPoints = source->getNumPoints();
        auto indices = source->indices;

        _lineplotPlugin.computeAverageSpectrum(parentPoints, noPoints, indices, "subset");
    }
}

void Endmember::sendColor(QColor endmemberColor) {
    
    _lineplotPlugin.getLineplotWidget().setEndmemberColor(endmemberColor);
}


EndmembersAction& Endmember::getEndmembersAction()
{
    return _lineplotPlugin.getSettingsAction().getEndmembersAction();
}

LineplotPlugin& Endmember::getLineplotPlugin()
{
    return _lineplotPlugin;
}