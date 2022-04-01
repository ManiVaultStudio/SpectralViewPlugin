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
    _data(),
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

void Endmember::setData(std::vector<float> data) {
    _data = data;
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

void Endmember::updateAngle(std::vector<float> endmemberData, float angle) {
    _lineplotPlugin.updateMap(endmemberData, angle);
}