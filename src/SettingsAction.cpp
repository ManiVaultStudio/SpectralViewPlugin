#include "SettingsAction.h"
#include "LineplotPlugin.h"
#include "MainToolbarAction.h"

#include <QMenu>
#include <QHeaderView>

using namespace hdps::gui;

SettingsAction::SettingsAction(LineplotPlugin& lineplotPlugin) :
    WidgetAction(&lineplotPlugin),
    _lineplotPlugin(lineplotPlugin),
    _endmembersAction(*this)
{
    setText("Settings");
}


QMenu* SettingsAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu();

    return menu;
}

SettingsAction::Widget::Widget(QWidget* parent, SettingsAction* settingsAction) :
    WidgetActionWidget(parent, settingsAction)
{
    setAutoFillBackground(true);
    setFixedWidth(350);

    auto layout = new QVBoxLayout();

    layout->setMargin(4);
    layout->addWidget(settingsAction->getEndmembersAction().createWidget(this));

    setLayout(layout);
}
