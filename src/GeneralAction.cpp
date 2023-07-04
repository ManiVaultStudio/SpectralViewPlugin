#include "GeneralAction.h"
#include "Endmember.h"
#include "EndmembersAction.h"
#include "SpectralViewPlugin.h"
#include "ClusterData/ClusterData.h"

#include <QtCore>

GeneralAction::GeneralAction(Endmember& endmember, int index) :
    GroupAction(&endmember, "GeneralAction", true),
    _endmember(endmember),
    _visibleAction(this, "Visible", true),
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


    auto dataset = _endmember.getDataset();
    auto type = dataset->getDataType();
    QColor endmemberColor;
    QVector<Cluster> clusters;

    _visibleAction.setChecked(false);

    // Get the name of the images dataset
    const auto guiName = dataset->getGuiName();

    // Set dataset name
    _datasetNameAction.setString(guiName);

    if (type == PointType) {
        // set name with avg
        if (index == 0) {

            // Set layer name and default name
            _nameAction.setString(guiName + " average");
            //_nameAction.setDefaultString(guiName + "average");
        }
        else {
            _nameAction.setString(guiName);
            //_nameAction.setDefaultString(guiName);
        }

        endmemberColor = _endmember.getEndmembersAction().getRandomLayerColor();
        //endmemberColor = _endmember.getGeneralAction().getColorAction().getColor();

    }
    else if (type == ClusterType) {
        clusters = dataset.get<Clusters>()->getClusters();
        auto noClusters = clusters.length();

        endmemberColor = clusters[index].getColor();
        auto endmemberName = clusters[index].getName();

        _nameAction.setString(endmemberName);
        //_nameAction.setDefaultString(endmemberName);
    }

    // Assign the color and default color
    _colorAction.setColor(endmemberColor);

    const auto render = [this]() {
        _endmember.getSpectralViewPlugin().getLineplotWidget().update();

    };

    connect(&_nameAction, &StringAction::stringChanged, this, render);
    connect(&_colorAction, &ColorAction::colorChanged, this, render);
}
