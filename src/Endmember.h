#pragma once

#include "EndmembersAction.h"
#include "GeneralAction.h"
#include "MapAction.h"

#include <util/Interpolation.h>
#include <event/EventListener.h>
#include <actions/WidgetAction.h>
#include <Set.h>

#include <ImageData/Images.h>
#include <PointData/PointData.h>

#include <QMenu>

using namespace mv::util;

class SpectralViewPlugin;

class Endmember : public WidgetAction
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param SpectralViewPlugin Reference to line plot plugin
     * @param dataset Smart pointer to dataset
     */
    Endmember(SpectralViewPlugin& spectralViewPlugin, const mv::Dataset<mv::DatasetImpl>& dataset, int index);

    /** Destructor */
    virtual ~Endmember();

    /** Get reference to image viewer plugin */
    SpectralViewPlugin& getSpectralViewPlugin();

    void sendColor(QColor endmemberColor, int row);
    void updateVisibility(bool toggled, int row);
    void sendEndmemberRemoved(int row);
    void highlightSelection(int row);
    void setData(std::vector<float> data);
    void setIndices(std::vector<unsigned int> indices);
    void computeMap(QString endmemberName, std::vector<float> endmemberData, float angle, int mapType, int algorithmType);
    void updateThresholdAngle(QString endmemberName, float threshold, int mapType, int algorithmType);
    std::vector<float> resample(std::vector<float> parentDim);
    int getIndex();

    /**
     * Get the context menu
     * @param parent Parent widget
     * @return Context menu
     */
    //QMenu* getContextMenu(QWidget* parent = nullptr);

    /** Activate the endmember */
    void activate();

    /** De-activate the endmember */
    void deactivate();

    /** Get whether the layer is active or not */
    bool isActive() const;

public: /** Action getters */

    EndmembersAction& getEndmembersAction();
    mv::Dataset<mv::DatasetImpl>& getDataset() { return _dataset; }
    std::vector<float> getData() { return _data;  }
    //std::vector<unsigned int> getIndices() { return _indices; }
    GeneralAction& getGeneralAction() { return _generalAction; }
    MapAction& getMapAction() { return _mapAction; }

protected:
    SpectralViewPlugin& _spectralViewPlugin;                              /** Reference to line plot plugin */
    bool                                _active;                  /** Whether the layer is active (editable) */  
    mv::Dataset<mv::DatasetImpl>    _dataset;                 /** Smart pointer to endmember dataset */
    GeneralAction                       _generalAction;
    MapAction                           _mapAction;
    std::vector<float>                  _data;                    /** Smart pointer to endmember data */
    int                                 _index;
    //std::vector<unsigned int>           _indices;

    friend class LineplotWidget;

};
