#include "GeneralAction.h"
#include "Endmember.h"
#include "EndmembersAction.h"
#include "LineplotPlugin.h"
#include "ClusterData.h"

GeneralAction::GeneralAction(Endmember& endmember) :
    GroupAction(&endmember, true),
    _endmember(endmember),
    _visibleAction(this, "Visible", true, true),
    _datasetNameAction(this, "Dataset name"),
    _colorAction(this, "Color"),
    _nameAction(this, "Name")
{
    setText("General");

    _datasetNameAction.setEnabled(false);

    // Set tooltips
    _visibleAction.setToolTip("Visibility of the endmember");
    _datasetNameAction.setToolTip("Name of the endmember dataset");
    _nameAction.setToolTip("Name of the endmember");

    // Get initial random layer color
    const auto endmemberColor = _endmember.getEndmembersAction().getRandomLayerColor();

    // Assign the color and default color
    _colorAction.initialize(endmemberColor, endmemberColor);

    // Get the name of the images dataset
    const auto guiName = _endmember.getDataset()->getGuiName();

    // Set dataset name
    _datasetNameAction.setString(guiName);

    // Set layer name and default name
    _nameAction.setString(guiName);
    _nameAction.setDefaultString(guiName);
    
    const auto render = [this]() {
        _endmember.getLineplotPlugin().getLineplotWidget().update();

    };
   // const auto updateBounds = [this]() {
   //     _layer.getImageViewerPlugin().getImageViewerWidget().updateWorldBoundingRectangle();
   // };

    connect(&_nameAction, &StringAction::stringChanged, this, render);
    //connect(&_visibleAction, &ToggleAction::toggled, this, render);
    //connect(&_colorAction, &ColorAction::colorChanged, this, updateBounds);
    connect(&_colorAction, &ColorAction::colorChanged, this, render);
}
