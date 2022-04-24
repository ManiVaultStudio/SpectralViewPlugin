#pragma once

#include "EndmembersAction.h"
#include "GeneralAction.h"
#include "MapAction.h"

#include <util/Interpolation.h>
#include <event/EventListener.h>
#include <actions/WidgetAction.h>
#include <Set.h>
#include <ImageData/Images.h>

#include "PointData.h"
#include <QMenu>

using namespace hdps::util;

class LineplotPlugin;

class Endmember : public WidgetAction, public hdps::EventListener
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param lineplotPlugin Reference to line plot plugin
     * @param dataset Smart pointer to dataset
     */
    Endmember(LineplotPlugin& lineplotPlugin, const hdps::Dataset<hdps::DatasetImpl>& dataset, int index);

    /** Destructor */
    virtual ~Endmember();

    /** Get reference to image viewer plugin */
    LineplotPlugin& getLineplotPlugin();

    void sendColor(QColor endmemberColor, int row);
    void updateVisibility(bool toggled, int row);
    void sendEndmemberRemoved(int row);
    void highlightSelection(int row);
    void setData(std::vector<float>);
    void computeMap(std::vector<float> endmemberData, float angle, int mapType, int algorithmType);
    void updateThresholdAngle(float threshold, int mapType);
    std::vector<float> resample(std::vector<float> parentDim);

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
    hdps::Dataset<hdps::DatasetImpl>& getDataset() { return _dataset; }
    std::vector<float> getData() { return _data;  }
    GeneralAction& getGeneralAction() { return _generalAction; }
    MapAction& getMapAction() { return _mapAction; }

protected:
    LineplotPlugin& _lineplotPlugin;                              /** Reference to line plot plugin */
    bool                                _active;                  /** Whether the layer is active (editable) */  
    hdps::Dataset<hdps::DatasetImpl>    _dataset;                 /** Smart pointer to endmember dataset */
    GeneralAction                       _generalAction;
    MapAction                           _mapAction;
    std::vector<float>                  _data;                    /** Smart pointer to endmember data */

    friend class LineplotWidget;

};