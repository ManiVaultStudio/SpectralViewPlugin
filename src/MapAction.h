#pragma once

#include <actions/GroupAction.h>
#include <actions/ToggleAction.h>
#include <actions/DecimalAction.h>
#include <actions/OptionAction.h>
#include <actions/TriggerAction.h>

class Endmember;

using namespace hdps::gui;

/**
 * General action class
 *
 * Action class for general layer settings
 *
 * Based on GeneralAction in ImageViewerPlugin
 */
class MapAction : public GroupAction
{
public:

    /**
     * Constructor
     * @param layer Reference to layer
     */
    MapAction(Endmember& endmember);

    /** Get reference to parent layer */
    Endmember& getEndmember() { return _endmember; }
    // hdps::Dataset<hdps::DatasetImpl>& getDataset() { return _dataset; }

public: /** Action getters */

    OptionAction& getAlgorithmAction() { return _algorithmAction; }
    OptionAction& getMapTypeAction() { return _mapTypeAction; }
    DecimalAction& getAngleAction() { return _angleAction; }
    TriggerAction& getComputeAction() { return _computeAction; }

protected:
    Endmember& _endmember;                 /** Reference to layer */
    OptionAction    _algorithmAction;     /** Dataset name action */
    OptionAction    _mapTypeAction;
    DecimalAction   _angleAction;
    TriggerAction    _computeAction;         /** Visible action */

};