#include "GlobalSettingsAction.h"
#include "SpectralViewPlugin.h"

#include <Application.h> 

#include <QHBoxLayout>

using namespace hdps;

GlobalSettingsAction::GlobalSettingsAction(SpectralViewPlugin& spectralViewPlugin) :
    WidgetAction(reinterpret_cast<QObject*>(&spectralViewPlugin)),
    _spectralViewPlugin(spectralViewPlugin),
    _showSelectionAction(this, "Show selection", true, true),
    _stdAreaEnabledAction(this, "Show std. dev.", true, true)
{
    setIcon(Application::getIconFont("FontAwesome").getIcon("cog"));
    setText("Global settings");

    _showSelectionAction.setToolTip("Show a line for the current selection");
    _stdAreaEnabledAction.setToolTip("Show the average +/- standard deviation interval");

    _stdAreaEnabledAction.setChecked(false);
    _showSelectionAction.setChecked(true);
    
    auto& lineplotWidget = _spectralViewPlugin.getLineplotWidget();

    const auto updateStdArea = [this, &lineplotWidget]() {
        lineplotWidget.enableStdArea(_stdAreaEnabledAction.isChecked());
    };

    connect(&_stdAreaEnabledAction, &ToggleAction::toggled, this, updateStdArea);
}

QMenu* GlobalSettingsAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu("Settings", parent);

    const auto addActionToMenu = [menu](QAction* action) -> void {
        auto actionMenu = new QMenu(action->text());

        actionMenu->addAction(action);

        menu->addMenu(actionMenu);
    };

    return menu;
}

GlobalSettingsAction::Widget::Widget(QWidget* parent, GlobalSettingsAction* globalSettingsAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, globalSettingsAction, widgetFlags)
{
    auto layout = new QHBoxLayout();

    // Create widgets for actions
    auto showStdWidget = globalSettingsAction->getStdAreaEnabledAction().createWidget(this);
    auto showSelectionWidget = globalSettingsAction->getShowSelectionAction().createWidget(this);

    if (widgetFlags & PopupLayout) {

        auto layout = new QGridLayout();

        layout->addWidget(showSelectionWidget, 0, 0);
        layout->addWidget(showStdWidget, 1, 0);

        setPopupLayout(layout);
    }
    else { 

        auto layout = new QHBoxLayout();

        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(showSelectionWidget);
        layout->addWidget(showStdWidget);

        setLayout(layout);
    }
    
}
