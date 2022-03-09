#pragma once

#include <ViewPlugin.h>

#include "Dataset.h"

#include "LineplotWidget.h"
#include "widgets/DropWidget.h"

#include <QList>

using namespace hdps::plugin;
using namespace hdps::util;

class Points;
class Clusters;

class LineplotPlugin : public ViewPlugin
{
    Q_OBJECT

public:
    LineplotPlugin(const PluginFactory* factory);
    ~LineplotPlugin() override;

    void init() override;

    void onDataEvent(hdps::DataEvent* dataEvent);

protected slots:
   // void dataSetPicked(const QString& name);
   // void clusterSelected(QList<int> selectedClusters);

private:
    void updateData();
    void updateSelection(hdps::Dataset<Points> selection);

    hdps::Dataset<Points>              _points;        /** Currently loaded points dataset */
    hdps::Dataset<Clusters>            _clusters;      /** Currently loaded clusters dataset */

    LineplotWidget* _linePlotWidget;       /** Heatmap widget displaying cluster data */
    hdps::gui::DropWidget* _dropWidget;    /** Widget allowing users to drop in data */
};

// =============================================================================
// Factory
// =============================================================================

class LineplotPluginFactory : public ViewPluginFactory
{
    Q_INTERFACES(hdps::plugin::ViewPluginFactory hdps::plugin::PluginFactory)
        Q_OBJECT
        Q_PLUGIN_METADATA(IID   "nl.tudelft.LineplotPlugin"
            FILE  "LineplotPlugin.json")

public:
    LineplotPluginFactory(void) {}
    ~LineplotPluginFactory(void) override {}

    /** Returns the plugin icon */
    QIcon getIcon() const override;

    ViewPlugin* produce() override;

    hdps::DataTypes supportedDataTypes() const override;
};
