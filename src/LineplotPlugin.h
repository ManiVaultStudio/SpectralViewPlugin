#pragma once

#include <ViewPlugin.h>

#include "LineplotWidget.h"
#include "MainToolbarAction.h"
//#include "SettingsAction.h"

#include "Dataset.h"

#include <widgets/DropWidget.h>

#include <QList>
#include <QSplitter>

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

public:
    /** Get reference to the lineplot widget */
    LineplotWidget& getLineplotWidget() {
        return _linePlotWidget;
    }

protected slots:
    void changeRGBWavelengths(const float wavelengthR, const float wavelengthG, const float wavelengthB);
    // void dataSetPicked(const QString& name);
    // void clusterSelected(QList<int> selectedClusters);

private:
    void importEndmembersCSV(const QString datasetGuid);
    void updateSelection(hdps::Dataset<Points> selection);
    void initializeImageRGB();
    //std::vector<float> createRGBImage(int dimR, int dimG, int dimB);

    hdps::Dataset<Points>              _points;        /** Currently loaded points dataset */
    hdps::Dataset<Clusters>            _clusters;      /** Currently loaded clusters dataset */
    hdps::Dataset<Points>              _imageRGBPoints;
    hdps::Dataset<Images>              _imageRGB;

    LineplotWidget       _linePlotWidget;       /** Heatmap widget displaying cluster data */
    QSplitter               _splitter;             /** Splitter which divides the lineplot view and editor */
    hdps::gui::DropWidget   _dropWidget;            /** Widget allowing users to drop in data */
    MainToolbarAction       _mainToolbarAction;     /** Main toolbar action */
  //  SettingsAction          _settingsAction;        /** Line chart settings action */
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
