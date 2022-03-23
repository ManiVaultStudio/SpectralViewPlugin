#include "EndmembersAction.h"
#include "SettingsAction.h"
#include "LineplotPlugin.h"
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
    //_currentLayerAction(this),
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
    WidgetActionWidget(parent, endmembersAction)
{
    auto& lineplotPlugin = endmembersAction->getSettingsAction().getLineplotPlugin();

    auto layout = new QVBoxLayout();
    auto treeView = new QTreeView();

    treeView->setFixedHeight(500);
    treeView->setModel(&lineplotPlugin.getModel());
    treeView->setRootIsDecorated(false);
    treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeView->setSelectionModel(&lineplotPlugin.getSelectionModel());
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

    layout->setMargin(0);
    layout->addWidget(treeView);

    setLayout(layout);
}