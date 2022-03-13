#pragma once

#include <ViewPlugin.h>

#include "LineplotWidget.h"

#include "Dataset.h"

#include "widgets/DropWidget.h"

#include <QList>

using namespace hdps::plugin;
using namespace hdps::util;

class Points;
class Clusters;
class Images;

class LineplotPlugin : public ViewPlugin
{
    Q_OBJECT

public:
    LineplotPlugin(const PluginFactory* factory);
    ~LineplotPlugin() override;

    void init() override;

    void onDataEvent(hdps::DataEvent* dataEvent);

protected slots:
    void changeRGBWavelengths(const float wavelengthR, const float wavelengthG, const float wavelengthB);
   // void dataSetPicked(const QString& name);
   // void clusterSelected(QList<int> selectedClusters);

private:
    void updatePixel(const QString datasetGuid);
    void updateSelection(hdps::Dataset<Points> selection);
    void initializeImageRGB();
    //std::vector<float> createRGBImage(int dimR, int dimG, int dimB);

    hdps::Dataset<Points>              _points;        /** Currently loaded points dataset */
    hdps::Dataset<Clusters>            _clusters;      /** Currently loaded clusters dataset */
    hdps::Dataset<Points>              _imageRGBPoints;
    hdps::Dataset<Images>              _imageRGB;

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
