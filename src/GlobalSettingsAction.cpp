#include "GlobalSettingsAction.h"
#include "LineplotPlugin.h"

#include <Application.h> 

#include <QHBoxLayout>

using namespace hdps;

GlobalSettingsAction::GlobalSettingsAction(LineplotPlugin& lineplotPlugin) :
    WidgetAction(reinterpret_cast<QObject*>(&lineplotPlugin)),
    _lineplotPlugin(lineplotPlugin),
    _groupAction(this),
    _wavelengthsRGBEnabledAction(this, "Show RGB wavelengths", true, true),
    _stdAreaEnabledAction(this, "Show standard deviation area", true, true)
{
    setIcon(Application::getIconFont("FontAwesome").getIcon("cog"));
    setText("Global settings");

    _wavelengthsRGBEnabledAction.setToolTip("Show 3 RGB lines that can be moved");
    _stdAreaEnabledAction.setToolTip("Show the average +/- standard deviation interval");

    _wavelengthsRGBEnabledAction.setChecked(false);
    _stdAreaEnabledAction.setChecked(false);

    _groupAction << _wavelengthsRGBEnabledAction;
    _groupAction << _stdAreaEnabledAction;

    auto& lineplotWidget = _lineplotPlugin.getLineplotWidget();

    // Update RGB lines enabled
    const auto updateRGBWavelengths = [this, &lineplotWidget]() {
        lineplotWidget.enableRGBWavelengths(_wavelengthsRGBEnabledAction.isChecked());
    };

    const auto updateStdArea = [this, &lineplotWidget]() {
        lineplotWidget.enableStdArea(_stdAreaEnabledAction.isChecked());
    };

    connect(&_wavelengthsRGBEnabledAction, &ToggleAction::toggled, this, updateRGBWavelengths);
    connect(&_stdAreaEnabledAction, &ToggleAction::toggled, this, updateStdArea);

}

GlobalSettingsAction::Widget::Widget(QWidget* parent, GlobalSettingsAction* globalSettingsAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, globalSettingsAction)
{
    auto layout = new QVBoxLayout();

    layout->setMargin(0);
    layout->setSpacing(3);

    layout->addWidget(globalSettingsAction->getGroupAction().createWidget(this));

    setPopupLayout(layout);
}