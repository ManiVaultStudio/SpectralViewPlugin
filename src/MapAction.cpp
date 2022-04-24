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
    _mapTypeAction(this, "Map type", { "Binary", "Threshold distance", "Distance based"}, "Binary", "Binary"),
    _angleAction(this, "Angle"),
    _computeAction(this, "Perform algorithm")
{
    setText("Mapping");

    // Set tooltips
    _algorithmAction.setToolTip("Choose algorithm for mapping");
    _mapTypeAction.setToolTip("Choose map type");
    _angleAction.setToolTip("Threshold angle for mapping");
    _computeAction.setToolTip("Update map for the selected endmember");
    
    auto& fontAwesome = Application::getIconFont("FontAwesome");
    _computeAction.setIcon(fontAwesome.getIcon("play"));

    _angleAction.setSuffix(" rad");
    _angleAction.setDefaultValue(0.15);
    _angleAction.defaultValueChanged(0.15);
    _angleAction.setNumberOfDecimals(3);
    _angleAction.numberOfDecimalsChanged(3);
    _angleAction.setRange(0, M_PI / 2);
    _angleAction.setSingleStep(0.05);
    _angleAction.singleStepChanged(0.05);
}