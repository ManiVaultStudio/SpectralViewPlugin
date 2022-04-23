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
    _stdAreaEnabledAction(this, "Show standard deviation area", true, true),
    _redWavelengthAction(this, "Red", { "630" }, "630", "630"),
    _greenWavelengthAction(this, "Green", { "532" }, "532", "532"),
    _blueWavelengthAction(this, "Blue", { "464" }, "464", "464")
    
{
    setIcon(Application::getIconFont("FontAwesome").getIcon("cog"));
    setText("Global settings");

    _wavelengthsRGBEnabledAction.setToolTip("Show 3 RGB lines that can be moved");
    _stdAreaEnabledAction.setToolTip("Show the average +/- standard deviation interval");
    _redWavelengthAction.setToolTip("Red wavelength set by dragging the red line");
    _greenWavelengthAction.setToolTip("Green wavelength set by dragging the green line");
    _blueWavelengthAction.setToolTip("Blue wavelength set by dragging the blue line");

    _wavelengthsRGBEnabledAction.setChecked(false);
    _stdAreaEnabledAction.setChecked(false);

    _groupAction << _wavelengthsRGBEnabledAction;
    _groupAction << _stdAreaEnabledAction;
    _groupAction << _redWavelengthAction;
    _groupAction << _greenWavelengthAction;
    _groupAction << _blueWavelengthAction;
     
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

    connect(&_redWavelengthAction, &OptionAction::currentTextChanged, this, [this, &lineplotWidget](const QString newWavelength) {
        //qDebug() << "Wavelength to pass: " << newWavelength;
        lineplotWidget.updateRGBLine(newWavelength.toFloat(), 0);
        });
    connect(&_greenWavelengthAction, &OptionAction::currentTextChanged, this, [this, &lineplotWidget](const QString newWavelength) {
        lineplotWidget.updateRGBLine(newWavelength.toFloat(), 1);
        });
    connect(&_blueWavelengthAction, &OptionAction::currentTextChanged, this, [this, &lineplotWidget](const QString newWavelength) {
        lineplotWidget.updateRGBLine(newWavelength.toFloat(), 2);
        });
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