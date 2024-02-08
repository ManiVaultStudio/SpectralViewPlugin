#include "MapAction.h"

#include "Endmember.h"
#include "EndmembersAction.h"
#include "SpectralViewPlugin.h"

#include "ClusterData/ClusterData.h"

#include <QtCore>

MapAction::MapAction(Endmember& endmember) :
    GroupAction(&endmember, "MapAction", true),
    _endmember(endmember),
    _algorithmAction(this, "Algorithm", {"Spectral Angle Mapper", "Spectral Correlation Mapper"}, "Spectral Angle Mapper"),
    _mapTypeAction(this, "Map type", { "Binary", "Threshold distance", "Distance based"}, "Binary"),
    _angleAction(this, "Angle"),
    _thresholdAction(this, "Threshold"),
    _updateAutoAction(this, "Update automatically"),
    _computeAction(this, "Perform algorithm")
{
    setText("Mapping");

    addAction(&_algorithmAction);
    addAction(&_mapTypeAction);
    addAction(&_angleAction);
    addAction(&_thresholdAction);
    addAction(&_updateAutoAction);
    addAction(&_computeAction);

    // Set tooltips
    _algorithmAction.setToolTip("Choose algorithm for mapping");
    _mapTypeAction.setToolTip("Choose map type");
    _angleAction.setToolTip("Map pixels with value less than threshold angle");
    _thresholdAction.setToolTip("Map pixels with value greater than threshold");
    _updateAutoAction.setToolTip("Perform selected algorithm automatically");
    _computeAction.setToolTip("Update map for the selected endmember");
    
    auto& fontAwesome = Application::getIconFont("FontAwesome");
    _computeAction.setIcon(fontAwesome.getIcon("play"));

    //_thresholdAction.setDefaultValue(0.15f);
    //_thresholdAction.defaultValueChanged(0.15f);
    _thresholdAction.setNumberOfDecimals(3);
    _thresholdAction.numberOfDecimalsChanged(3);
    _thresholdAction.setRange(0.f, 1.f);
    _thresholdAction.setSingleStep(0.05f);
    _thresholdAction.singleStepChanged(0.05f);

    _thresholdAction.setEnabled(false);

    _angleAction.setSuffix(" rad");
    //_angleAction.setDefaultValue(0.15f);
    //_angleAction.defaultValueChanged(0.15f);
    _angleAction.setNumberOfDecimals(3);
    _angleAction.numberOfDecimalsChanged(3);
    _angleAction.setRange(0.f, static_cast<float>(M_PI / 2));
    _angleAction.setSingleStep(0.05f);
    _angleAction.singleStepChanged(0.05f);
}
