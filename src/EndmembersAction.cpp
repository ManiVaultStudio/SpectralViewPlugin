#include "EndmembersAction.h"
#include "SettingsAction.h"
#include "SpectralViewPlugin.h"
#include "LineplotWidget.h"

#include <Application.h>

#include <widgets/Divider.h>

#include <QTreeView>
#include <QHeaderView>

using namespace hdps;
using namespace hdps::gui;

EndmembersAction::EndmembersAction(SettingsAction& settingsAction) :
    WidgetAction(reinterpret_cast<QObject*>(&settingsAction)),
    _settingsAction(settingsAction),
    _currentEndmemberAction(this),
    _rng(0)
{
    setText("Endmembers");
}

QColor EndmembersAction::getRandomLayerColor()
{
    // Randomize HSL parameters
    const auto randomHue = _rng.bounded(360);
    const auto randomSaturation = _rng.bounded(150, 255);
    const auto randomLightness = _rng.bounded(150, 220);

    // Create random color from hue, saturation and lightness
    return QColor::fromHsl(randomHue, randomSaturation, randomLightness);
}

EndmembersAction::Widget::Widget(QWidget* parent, EndmembersAction* endmembersAction) :
    WidgetActionWidget(parent, endmembersAction),
    _removeEndmemberAction(this, ""),
    _saveEndmembersAction(this, "")
{
    auto& spectralViewPlugin = endmembersAction->getSettingsAction().getSpectralViewPlugin();

    _removeEndmemberAction.setToolTip("Remove the selected endmember");
    _saveEndmembersAction.setToolTip("Save the checked endmembers in the list");

    auto& fontAwesome = Application::getIconFont("FontAwesome");

    _removeEndmemberAction.setIcon(fontAwesome.getIcon("trash-alt"));
    _saveEndmembersAction.setIcon(fontAwesome.getIcon("save"));

    auto layout = new QVBoxLayout();
    auto treeView = new QTreeView();

    treeView->setFixedHeight(300);
    treeView->setModel(&spectralViewPlugin.getModel());
    treeView->setRootIsDecorated(false);
    treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeView->setSelectionModel(&spectralViewPlugin.getSelectionModel());
    treeView->setSortingEnabled(false);

    // Configure header view
    auto header = treeView->header();

    header->setStretchLastSection(true);

    const auto minimumSectionSize = 20;

    header->setMinimumSectionSize(minimumSectionSize);

    header->resizeSection(EndmembersModel::Visible, minimumSectionSize);
    header->resizeSection(EndmembersModel::Color, minimumSectionSize);

    header->setSectionResizeMode(EndmembersModel::Visible, QHeaderView::Fixed);
    header->setSectionResizeMode(EndmembersModel::Color, QHeaderView::Fixed);
    header->setSectionResizeMode(EndmembersModel::Name, QHeaderView::Interactive);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(treeView);

    auto toolbarLayout = new QHBoxLayout();

    toolbarLayout->setSpacing(3);

    toolbarLayout->addWidget(_removeEndmemberAction.createWidget(this, TriggerAction::Icon));
    toolbarLayout->addWidget(_saveEndmembersAction.createWidget(this, TriggerAction::Icon));

    layout->addLayout(toolbarLayout);

    auto currentEndmemberWidget = endmembersAction->getCurrentEndmemberAction().createWidget(this);

    layout->addWidget(createHorizontalDivider());
    layout->addWidget(currentEndmemberWidget);

    setLayout(layout);

    const auto modelSelectionChanged = [this, endmembersAction, &spectralViewPlugin, treeView, layout]() -> void {
        const auto selectedRows = spectralViewPlugin.getSelectionModel().selectedRows();
        const auto hasSelection = !selectedRows.isEmpty();

        GroupsAction::GroupActions groupActions;

        if (hasSelection) {
            auto endmember = static_cast<Endmember*>(selectedRows.first().internalPointer());

            int row = selectedRows.first().row();
            endmember->highlightSelection(row);
            
            /*
            if (endmember->getIndices().size() != 0) {
                spectralViewPlugin.setSelection(endmember->getIndices());
            }
            */

            groupActions << &endmember->getGeneralAction() << &endmember->getMapAction();
        }
        else {
            spectralViewPlugin.getLineplotWidget().setHighlightSelection(-1);
        }

        endmembersAction->getCurrentEndmemberAction().setGroupActions(groupActions);
    };

    // Update various actions when the model is somehow changed (rows added/removed etc.)
    const auto updateButtons = [this, &spectralViewPlugin, treeView, layout]() -> void {
        const auto selectedRows = spectralViewPlugin.getSelectionModel().selectedRows();
        const auto hasSelection = !selectedRows.isEmpty();

        _removeEndmemberAction.setEnabled(hasSelection);
        _saveEndmembersAction.setEnabled(spectralViewPlugin.getModel().rowCount() > 0);

        // Render
        spectralViewPlugin.getLineplotWidget().update();

    };

    connect(&spectralViewPlugin.getSelectionModel(), &QItemSelectionModel::selectionChanged, this, modelSelectionChanged);
    connect(&spectralViewPlugin.getSelectionModel(), &QItemSelectionModel::selectionChanged, this, updateButtons);
    connect(treeView->model(), &QAbstractListModel::rowsRemoved, updateButtons);

    // Select an endmember when it is inserted into the model
    const auto onRowsInserted = [treeView, &spectralViewPlugin, updateButtons](const QModelIndex& parent, int first, int last) {

        // Get model of inserted endmember
        const auto index = treeView->model()->index(first, 0);

        // Select the endmember if the index is valid
        if (index.isValid())
            spectralViewPlugin.getSelectionModel().select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

        // Update endmember position buttons
        updateButtons();
    };

    // Special behavior when a row is inserted into the model
    connect(treeView->model(), &QAbstractListModel::rowsInserted, this, onRowsInserted);

    // Remove the endmember when the corresponding action is triggered
    connect(&_removeEndmemberAction, &TriggerAction::triggered, this, [this, &spectralViewPlugin, treeView]() {
        const auto selectedRows = spectralViewPlugin.getSelectionModel().selectedRows();

        if (selectedRows.isEmpty())
            return;

        auto rowIndex = selectedRows.first();

        spectralViewPlugin.getModel().removeEndmember(rowIndex);

        if (selectedRows.length() > 1) {
            spectralViewPlugin.getLineplotWidget().setHighlightSelection(rowIndex.row());
        }

        });

    // Save the endmembers in the list when the corresponding action is triggered
    connect(&_saveEndmembersAction, &TriggerAction::triggered, this, [this, &spectralViewPlugin, treeView]() {
        auto name = spectralViewPlugin.getDatasetName();

        spectralViewPlugin.getModel().saveEndmembers(name);
        });

    updateButtons();
    modelSelectionChanged();

}
