#include "SettingsAction.h"

#include "SpectralViewPlugin.h"

#include <QMenu>

using namespace mv::gui;

SettingsAction::SettingsAction(SpectralViewPlugin& spectralViewPlugin) :
    WidgetAction(&spectralViewPlugin, "SettingsAction"),
    _spectralViewPlugin(spectralViewPlugin),
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
    setFixedWidth(300);

    auto layout = new QVBoxLayout();

    layout->setContentsMargins(4, 4, 4, 4);
    layout->addWidget(settingsAction->getEndmembersAction().createWidget(this));

    setLayout(layout);
}
