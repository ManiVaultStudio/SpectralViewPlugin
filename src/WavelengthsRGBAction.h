#pragma once

#include <actions/WidgetAction.h>
#include <actions/ToggleAction.h>
#include <actions/OptionAction.h>

using namespace mv::gui;

class SpectralViewPlugin;

/**
 * Wavelengths RGB action class
 *
 * Settings action class for RGB wavelengths settings
 *
 * Based on the implementations in ScatterplotPlugin
 */
class WavelengthsRGBAction : public WidgetAction
{
    Q_OBJECT

protected: // Widget

/** Widget class for subset action */
    class Widget : public WidgetActionWidget {
    public:

        /**
         * Constructor
         * @param parent Pointer to parent widget
         * @param globalViewSettingsAction Pointer to global view settings action
         * @param widgetFlags Widget flags for the configuration of the widget (type)
         */
        Widget(QWidget* parent, WavelengthsRGBAction* wavelengthsRGBAction, const std::int32_t& widgetFlags);
    };

    /**
     * Get widget representation of the global view settings action
     * @param parent Pointer to parent widget
     * @param widgetFlags Widget flags for the configuration of the widget (type)
     */
    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this, widgetFlags);
    };

public:

    /**
     * Constructor
     * @param imageViewerPlugin Reference to image viewer plugin
     */
    WavelengthsRGBAction(SpectralViewPlugin& spectralViewPlugin);

    /**
     * Get the context menu for the action
     * @param parent Parent widget
     * @return Context menu
     */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;


public: /** Action getters */

    ToggleAction& getWavelengthsRGBEnabledAction() { return _wavelengthsRGBEnabledAction; }
    OptionAction& getRedWavelengthAction() { return _redWavelengthAction; }
    OptionAction& getGreenWavelengthAction() { return _greenWavelengthAction; }
    OptionAction& getBlueWavelengthAction() { return _blueWavelengthAction; }

protected:
    SpectralViewPlugin& _spectralViewPlugin;         /** Reference to image viewer plugin */
    ToggleAction    _wavelengthsRGBEnabledAction;
    OptionAction    _redWavelengthAction;
    OptionAction    _greenWavelengthAction;
    OptionAction    _blueWavelengthAction;
};
