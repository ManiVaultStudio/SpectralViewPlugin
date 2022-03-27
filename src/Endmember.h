#pragma once

#include "EndmembersAction.h"
#include "GeneralAction.h"

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
    Endmember(LineplotPlugin& lineplotPlugin, const hdps::Dataset<hdps::DatasetImpl>& dataset);

    /** Destructor */
    virtual ~Endmember();

    /** Get reference to image viewer plugin */
    LineplotPlugin& getLineplotPlugin();

    void sendData(hdps::Dataset<hdps::DatasetImpl>& dataset, std::string dataOrigin);
    void sendColor(QColor endmemberColor, int row);
    void updateVisibility(bool toggled, int row);
    void sendEndmemberRemoved(int row);

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
    GeneralAction& getGeneralAction() { return _generalAction; }

protected:
    LineplotPlugin& _lineplotPlugin;             /** Reference to image viewer plugin */
    bool                                _active;                        /** Whether the layer is active (editable) */                /** Smart pointer to images dataset */
    hdps::Dataset<hdps::DatasetImpl>    _dataset;                 /** Smart pointer to source dataset of the images */
    GeneralAction _generalAction;

    friend class LineplotWidget;

};