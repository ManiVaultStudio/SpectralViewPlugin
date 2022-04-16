#include "MapAction.h"
#include "Endmember.h"
#include "EndmembersAction.h"
#include "LineplotPlugin.h"
#include "ClusterData.h"
#include <QtCore>

MapAction::MapAction(Endmember& endmember) :
    GroupAction(&endmember, true),
    _endmember(endmember),
    _algorithmAction(this, "Algorithm", {"Spectral Angle Mapper", "Spectral Correlation Mapper"}, "Spectral Angle Mapper", "Spectral Angle Mapper"),
    _mapTypeAction(this, "Map type", { "Binary", "Distance based"}, "Binary", "Binary"),
    _angleAction(this, "Angle"),
    _updateAction(this, "Update automatically"),
    _computeAction(this, "Perform algorithm")
{
    setText("Mapping");

    // Set tooltips
    _algorithmAction.setToolTip("Choose algorithm for mapping");
    _mapTypeAction.setToolTip("Choose map type");
    _angleAction.setToolTip("Threshold angle for mapping");
    _updateAction.setToolTip("Update map on release of angle bar");
    _computeAction.setToolTip("Update map");
    
    auto& fontAwesome = Application::getIconFont("FontAwesome");
    _computeAction.setIcon(fontAwesome.getIcon("play"));

    _angleAction.setSuffix(" rad");
    _angleAction.setUpdateDuringDrag(false);
    _angleAction.setMinimum(0);
    _angleAction.minimumChanged(0);
    _angleAction.setSingleStep(0.05);
    _angleAction.singleStepChanged(0.05);
    _angleAction.setMaximum(M_PI/2);
    _angleAction.maximumChanged(M_PI/2);
}