#pragma once

#include <ViewPlugin.h>

#include "EndmembersModel.h"
#include "LineplotWidget.h"
#include "SettingsAction.h"
#include "ViewSettingsAction.h"
#include "WavelengthsRGBAction.h"

#include "Dataset.h"
#include "PointData/PointData.h"
#include "Set.h"

#include <actions/HorizontalToolbarAction.h>
#include <widgets/DropWidget.h>

#include <QItemSelectionModel>
#include <QList>
#include <QSplitter>

using namespace mv::plugin;
using namespace mv::util;

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

    void onDataEvent(mv::DatasetEvent* dataEvent);

    /**
     * Add dataset to the viewer
     * @param dataset Smart pointer to dataset
     */
    void addDataset(const mv::Dataset<mv::DatasetImpl>& dataset);
    void updateDataset(const mv::Dataset<mv::DatasetImpl>& dataset);
    void addNewCluster(const mv::Dataset<mv::DatasetImpl>& dataset);
    void addAverageDataset(const mv::Dataset<mv::DatasetImpl>& dataset);
    void loadData(const mv::Datasets& datasets) override;

    std::tuple<std::vector<float>, std::vector<float>> computeAverageSpectrum(mv::Dataset<mv::DatasetImpl> source, unsigned int noPoints, std::vector<unsigned int> indices, std::string dataOrigin);
    
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
    //void setSelection(std::vector<unsigned int> indices);

public: // Action getters

    HorizontalToolbarAction& getMainToolbarAction() { return _mainToolbarAction; }
    SettingsAction& getSettingsAction() { return _settingsAction; }

protected slots:
    void changeRGBWavelengths(const float wavelengthR, int index);
    // void dataSetPicked(const QString& name);
    // void clusterSelected(QList<int> selectedClusters);

private:
    void initializeImageRGB();
    void updateSelection(mv::Dataset<Points> selection);
    //std::vector<float> createRGBImage(int dimR, int dimG, int dimB);

    mv::Dataset<Points>         _points;                /** Currently loaded points dataset */
    std::vector<QString>        _clusterNames;          /** Names of currently loaded cluster datasets */
    mv::Dataset<Points>         _angleMap;
    std::vector<float>          _mapAngleData;
    mv::Dataset<Images>         _mapAngleImage;
    mv::Dataset<Points>         _corMap;
    std::vector<float>          _mapCorData;
    mv::Dataset<Images>         _mapCorImage;
    std::vector<float>          _angleDataset;
    std::vector<float>          _corDataset;
    std::vector<float>          _averageDataset;
    std::vector<int>            _noLoadedClusters;
    std::vector<unsigned int>   _prevSelection;
            
    EndmembersModel             _model;                 /** Endmembers model */
    QItemSelectionModel         _selectionModel;        /** Layers selection model */
    LineplotWidget              _linePlotWidget;        /** Heatmap widget displaying cluster data */
    QSplitter                   _splitter;              /** Splitter which divides the lineplot view and editor */
    mv::gui::DropWidget         _dropWidget;            /** Widget allowing users to drop in data */

    SettingsAction              _settingsAction;        /** Line chart settings action */

    HorizontalToolbarAction     _mainToolbarAction;     /** Main toolbar action */
    ViewSettingsAction          _viewSettingsAction;    /** Global view settings action */
    WavelengthsRGBAction        _wavelengthsRGBAction;

    mv::EventListener           _eventListener;         /** Listen to HDPS events */
};


// =============================================================================
// Factory
// =============================================================================

class SpectralViewPluginFactory : public ViewPluginFactory
{
    Q_INTERFACES(mv::plugin::ViewPluginFactory mv::plugin::PluginFactory)
        Q_OBJECT
        Q_PLUGIN_METADATA(IID   "nl.tudelft.SpectralViewPlugin"
            FILE  "SpectralViewPlugin.json")

public:
    SpectralViewPluginFactory() {}
    ~SpectralViewPluginFactory() override {}

    /** Returns the plugin icon */
    QIcon getIcon() const;// override;

    ViewPlugin* produce() override;

    mv::DataTypes supportedDataTypes() const override;

    PluginTriggerActions getPluginTriggerActions(const mv::Datasets& datasets) const override;
};
