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
    _thresholdAction(this, "Threshold"),
    _updateAutoAction(this, "Update automatically"),
    _computeAction(this, "Perform algorithm")
{
    setText("Mapping");

    // Set tooltips
    _algorithmAction.setToolTip("Choose algorithm for mapping");
    _mapTypeAction.setToolTip("Choose map type");
    _angleAction.setToolTip("Map pixels with value less than threshold angle");
    _thresholdAction.setToolTip("Map pixels with value greater than threshold");
    _updateAutoAction.setToolTip("Perform selected algorithm automatically");
    _computeAction.setToolTip("Update map for the selected endmember");
    
    auto& fontAwesome = Application::getIconFont("FontAwesome");
    _computeAction.setIcon(fontAwesome.getIcon("play"));

    _thresholdAction.setDefaultValue(0.15);
    _thresholdAction.defaultValueChanged(0.15);
    _thresholdAction.setNumberOfDecimals(3);
    _thresholdAction.numberOfDecimalsChanged(3);
    _thresholdAction.setRange(0, 1);
    _thresholdAction.setSingleStep(0.05);
    _thresholdAction.singleStepChanged(0.05);

    _thresholdAction.setEnabled(false);

    _angleAction.setSuffix(" rad");
    _angleAction.setDefaultValue(0.15);
    _angleAction.defaultValueChanged(0.15);
    _angleAction.setNumberOfDecimals(3);
    _angleAction.numberOfDecimalsChanged(3);
    _angleAction.setRange(0, M_PI / 2);
    _angleAction.setSingleStep(0.05);
    _angleAction.singleStepChanged(0.05);
}