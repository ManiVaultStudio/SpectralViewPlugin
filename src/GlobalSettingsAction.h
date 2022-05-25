#pragma once

#include <actions/WidgetAction.h>
#include <actions/ToggleAction.h>

using namespace hdps::gui;

class LineplotPlugin;

/**
 * Global view settings action class
 *
 * Settings action class for global view settings
 *
 * MainToolbarAction, SettingsAction and GlobalSettingsAction 
 * based on the implementations in ImageViewerPlugin
 */
class GlobalSettingsAction : public WidgetAction
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
        Widget(QWidget* parent, GlobalSettingsAction* globalViewSettingsAction, const std::int32_t& widgetFlags);
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
    GlobalSettingsAction(LineplotPlugin& lineplotPlugin);

    /**
     * Get the context menu for the action
     * @param parent Parent widget
     * @return Context menu
     */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;


public: /** Action getters */
    
    ToggleAction& getStdAreaEnabledAction() { return _stdAreaEnabledAction; }
    ToggleAction& getShowSelectionAction() { return _showSelectionAction; }

protected:
    LineplotPlugin& _lineplotPlugin;         /** Reference to image viewer plugin */
    ToggleAction    _stdAreaEnabledAction;
    ToggleAction    _showSelectionAction;
};