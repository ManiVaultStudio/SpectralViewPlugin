#pragma once

#include <actions/ColorAction.h>
#include <actions/GroupAction.h>
#include <actions/StringAction.h>
#include <actions/ToggleAction.h>

class Endmember;

using namespace mv::gui;

/**
 * General action class
 *
 * Action class for general layer settings
 *
 * @author Thomas Kroes
 */
class GeneralAction : public GroupAction
{
public:

    /**
     * Constructor
     * @param layer Reference to layer
     */
    GeneralAction(Endmember& endmember, int index);

    /** Get reference to parent layer */
    Endmember& getEndmember() { return _endmember; }
   // mv::Dataset<mv::DatasetImpl>& getDataset() { return _dataset; }

public: /** Action getters */

    StringAction& getDatasetNameAction() { return _datasetNameAction; }
    ToggleAction& getVisibleAction() { return _visibleAction; }
    ColorAction& getColorAction() { return _colorAction; }
    StringAction& getNameAction() { return _nameAction; }

protected:
    Endmember& _endmember;                 /** Reference to layer */
    ToggleAction    _visibleAction;         /** Visible action */
    StringAction    _datasetNameAction;     /** Dataset name action */
    ColorAction     _colorAction;           /** Color action */
    StringAction    _nameAction;            /** Name action */
};