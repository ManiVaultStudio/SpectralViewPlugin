#pragma once

#include <ViewPlugin.h>

#include "EndmembersModel.h"
#include "LineplotWidget.h"
#include "MainToolbarAction.h"
#include "SettingsAction.h"

#include "Dataset.h"
#include "PointData/PointData.h"
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

class SpectralViewPlugin : public ViewPlugin
{
    Q_OBJECT

public:
    SpectralViewPlugin(const PluginFactory* factory);
    ~SpectralViewPlugin() override;

    void init() override;

    void onDataEvent(hdps::DataEvent* dataEvent);

    /**
     * Add dataset to the viewer
     * @param dataset Smart pointer to dataset
     */
    void addDataset(const hdps::Dataset<hdps::DatasetImpl>& dataset);
    void updateDataset(const hdps::Dataset<hdps::DatasetImpl>& dataset);
    void addNewCluster(const hdps::Dataset<hdps::DatasetImpl>& dataset);
    void addAverageDataset(const hdps::Dataset<hdps::DatasetImpl>& dataset);
    void loadData(const hdps::Datasets& datasets) override;


    std::tuple<std::vector<float>, std::vector<float>> computeAverageSpectrum(hdps::Dataset<hdps::DatasetImpl> source, unsigned int noPoints, std::vector<unsigned int> indices, std::string dataOrigin);
    

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

    QString getDatasetName();

    void updateMap(QString endmemberName, std::vector<float> endmemberData, float angle, int mapType, int algorithmType);
    void spectralAngleMapper(QString endmemeberName, std::vector<float> endmemberData, float thresholdAngle, int mapType);
    void spectralCorrelationMapper(QString endmemberName, std::vector<float> endmemberData, float thresholdAngle, int mapType);
    void updateThresholdAngle(QString endmemberName, float threshold, int mapType, int algorithmType);
    void computeAverageDataset(int width, int height, int numDimensions);
    void setSelection(std::vector<unsigned int> indices);



public: // Action getters

    MainToolbarAction& getMainToolbarAction() { return _mainToolbarAction; }
    SettingsAction& getSettingsAction() { return _settingsAction; }

protected slots:
    void changeRGBWavelengths(const float wavelengthR, int index);
    // void dataSetPicked(const QString& name);
    // void clusterSelected(QList<int> selectedClusters);

private:
    void initializeImageRGB();
    void updateSelection(hdps::Dataset<Points> selection);
    //std::vector<float> createRGBImage(int dimR, int dimG, int dimB);

    hdps::Dataset<Points>              _points;        /** Currently loaded points dataset */
    std::vector<QString>               _clusterNames;      /** Names of currently loaded cluster datasets */
    hdps::Dataset<Points>              _angleMap;
    std::vector<float>                 _mapAngleData;
    hdps::Dataset<Images>              _mapAngleImage;
    hdps::Dataset<Points>              _corMap;
    std::vector<float>                 _mapCorData;
    hdps::Dataset<Images>              _mapCorImage;
    std::vector<float>                 _angleDataset;
    std::vector<float>                 _corDataset;
    std::vector<float>                 _averageDataset;
    std::vector<int>                   _noLoadedClusters;
    std::vector<unsigned int>          _prevSelection;
            
    EndmembersModel         _model;                 /** Endmembers model */
    QItemSelectionModel     _selectionModel;        /** Layers selection model */
    LineplotWidget          _linePlotWidget;       /** Heatmap widget displaying cluster data */
    QSplitter               _splitter;             /** Splitter which divides the lineplot view and editor */
    hdps::gui::DropWidget   _dropWidget;            /** Widget allowing users to drop in data */
    MainToolbarAction       _mainToolbarAction;     /** Main toolbar action */
    SettingsAction          _settingsAction;        /** Line chart settings action */

    hdps::EventListener     _eventListener;     /** Listen to HDPS events */
};


// =============================================================================
// Factory
// =============================================================================

class SpectralViewPluginFactory : public ViewPluginFactory
{
    Q_INTERFACES(hdps::plugin::ViewPluginFactory hdps::plugin::PluginFactory)
        Q_OBJECT
        Q_PLUGIN_METADATA(IID   "nl.tudelft.SpectralViewPlugin"
            FILE  "SpectralViewPlugin.json")

public:
    SpectralViewPluginFactory() {}
    ~SpectralViewPluginFactory() override {}

    /** Returns the plugin icon */
    QIcon getIcon() const;// override;

    ViewPlugin* produce() override;

    hdps::DataTypes supportedDataTypes() const override;

    PluginTriggerActions getPluginTriggerActions(const hdps::Datasets& datasets) const override;
};
