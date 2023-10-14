#include "WavelengthsRGBAction.h"
#include "SpectralViewPlugin.h"

#include <Application.h> 
#include <QHBoxLayout>



using namespace mv;

WavelengthsRGBAction::WavelengthsRGBAction(SpectralViewPlugin& spectralViewPlugin) :
    WidgetAction(reinterpret_cast<QObject*>(&spectralViewPlugin), "WavelengthsRGBAction"),
    _spectralViewPlugin(spectralViewPlugin),
    _wavelengthsRGBEnabledAction(this, "Show RGB lines", true),
    _redWavelengthAction(this, "Red", { "630" }, "630"),
    _greenWavelengthAction(this, "Green", { "532" }, "532"),
    _blueWavelengthAction(this, "Blue", { "464" }, "464")
{
    setIcon(Application::getIconFont("FontAwesome").getIcon("grip-lines-vertical"));
    setText("RGB Wavelengths Settings");

    _wavelengthsRGBEnabledAction.setToolTip("Show 3 RGB lines that can be moved");
    _redWavelengthAction.setToolTip("Red wavelength set by dragging the red line");
    _greenWavelengthAction.setToolTip("Green wavelength set by dragging the green line");
    _blueWavelengthAction.setToolTip("Blue wavelength set by dragging the blue line");

    _wavelengthsRGBEnabledAction.setChecked(false);
    
    auto& lineplotWidget = _spectralViewPlugin.getLineplotWidget();

    // Update RGB lines enabled
    const auto updateRGBWavelengths = [this, &lineplotWidget]() {
        lineplotWidget.enableRGBWavelengths(_wavelengthsRGBEnabledAction.isChecked());
    };

    connect(&_wavelengthsRGBEnabledAction, &ToggleAction::toggled, this, updateRGBWavelengths);

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

QMenu* WavelengthsRGBAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu("RGB Lines Settings", parent);

    const auto addActionToMenu = [menu](QAction* action) -> void {
        auto actionMenu = new QMenu(action->text());

        actionMenu->addAction(action);

        menu->addMenu(actionMenu);
    };

    return menu;
}

WavelengthsRGBAction::Widget::Widget(QWidget* parent, WavelengthsRGBAction* wavelengthsRGBAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, wavelengthsRGBAction, widgetFlags)
{
    auto layout = new QHBoxLayout();

    // Create widgets for actions
    auto showRGBWidget = wavelengthsRGBAction->getWavelengthsRGBEnabledAction().createWidget(this);
    auto redLabelWidget = wavelengthsRGBAction->getRedWavelengthAction().createLabelWidget(this);
    auto redWidget = wavelengthsRGBAction->getRedWavelengthAction().createWidget(this);
    auto greenLabelWidget = wavelengthsRGBAction->getGreenWavelengthAction().createLabelWidget(this);
    auto greenWidget = wavelengthsRGBAction->getGreenWavelengthAction().createWidget(this);
    auto blueLabelWidget = wavelengthsRGBAction->getBlueWavelengthAction().createLabelWidget(this);
    auto blueWidget = wavelengthsRGBAction->getBlueWavelengthAction().createWidget(this);
    
    if (widgetFlags & PopupLayout) {

        auto layout = new QGridLayout();

        layout->addWidget(showRGBWidget, 0, 0);
        layout->addWidget(redLabelWidget, 1, 0);
        layout->addWidget(redWidget, 1, 1);
        layout->addWidget(greenLabelWidget, 2, 0);
        layout->addWidget(greenWidget, 2, 1);
        layout->addWidget(blueLabelWidget, 3, 0);
        layout->addWidget(blueWidget, 3, 1);

        setLayout(layout);
    }
    else { 

        auto layout = new QHBoxLayout();

        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(showRGBWidget);
        layout->addWidget(redLabelWidget);
        layout->addWidget(redWidget);
        layout->addWidget(greenLabelWidget);
        layout->addWidget(greenWidget);
        layout->addWidget(blueLabelWidget);
        layout->addWidget(blueWidget);

        setLayout(layout);
    }
    
}
