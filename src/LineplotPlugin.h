#pragma once

#include <ViewPlugin.h>

#include "EndmembersModel.h"
#include "LineplotWidget.h"
#include "MainToolbarAction.h"
#include "SettingsAction.h"

#include "Dataset.h"
#include "PointData.h"
#include "Set.h"

#include <widgets/DropWidget.h>

#include <QList>
#include <QItemSelectionModel>
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

    /**
     * Add dataset to the viewer
     * @param dataset Smart pointer to dataset
     */
    void addDataset(const hdps::Dataset<hdps::DatasetImpl>& dataset);

    void computeAverageSpectrum(hdps::Dataset<Points> source, int noPoints, std::vector<unsigned int> indices, std::string dataOrigin);

public:

    /** Get the layers model */
    EndmembersModel& getModel() {
        return _model;
    }

    /** Get the layers selection model */
    QItemSelectionModel& getSelectionModel() {
        return _selectionModel;
    }

    /** Get reference to the lineplot widget */
    LineplotWidget& getLineplotWidget() {
        return _linePlotWidget;
    }


public: // Action getters

    MainToolbarAction& getMainToolbarAction() { return _mainToolbarAction; }
    SettingsAction& getSettingsAction() { return _settingsAction; }

protected slots:
    void changeRGBWavelengths(const float wavelengthR, const float wavelengthG, const float wavelengthB);
    // void dataSetPicked(const QString& name);
    // void clusterSelected(QList<int> selectedClusters);

private:
    void importEndmembersCSV(const QString datasetGuid);
    void initializeImageRGB();
    void updateSelection(hdps::Dataset<Points> selection);
    //std::vector<float> createRGBImage(int dimR, int dimG, int dimB);

    hdps::Dataset<Points>              _points;        /** Currently loaded points dataset */
    hdps::Dataset<Clusters>            _clusters;      /** Currently loaded clusters dataset */
    hdps::Dataset<Points>              _imageRGBPoints;
    hdps::Dataset<Images>              _imageRGB;

    EndmembersModel             _model;                 /** Endmembers model */
    QItemSelectionModel     _selectionModel;        /** Layers selection model */
    LineplotWidget       _linePlotWidget;       /** Heatmap widget displaying cluster data */
    QSplitter               _splitter;             /** Splitter which divides the lineplot view and editor */
    hdps::gui::DropWidget   _dropWidget;            /** Widget allowing users to drop in data */
    MainToolbarAction       _mainToolbarAction;     /** Main toolbar action */
    SettingsAction          _settingsAction;        /** Line chart settings action */
    int                     _childrenLen;
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
    LineplotPluginFactory() {}
    ~LineplotPluginFactory() override {}

    /** Returns the plugin icon */
    QIcon getIcon() const override;

    ViewPlugin* produce() override;

    hdps::DataTypes supportedDataTypes() const override;
};
