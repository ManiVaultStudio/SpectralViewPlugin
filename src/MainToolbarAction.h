#pragma once

#include <actions/WidgetAction.h>
#include "GlobalSettingsAction.h"

class LineplotPlugin;
class LineplotWidget;

/**
 * Main toolbar action class
 *
 * Action class for main toolbar
 * 
 * MainToolbarAction, SettingsAction and GlobalSettingsAction 
 * based on the implementations in ImageViewerPlugin
 */
class MainToolbarAction : public WidgetAction
{
public:

    /** Widget class for main toolbar action */
    class Widget : public WidgetActionWidget
    {
    protected:

        /**
         * Constructor
         * @param parent Pointer to parent widget
         * @param mainToolbarAction Pointer to main toolbar action
         */
        Widget(QWidget* parent, MainToolbarAction* mainToolbarAction);

    protected:
        friend class MainToolbarAction;
    };

protected:

    /**
     * Get widget representation of the main toolbar action
     * @param parent Pointer to parent widget
     * @param widgetFlags Widget flags for the configuration of the widget (type)
     */
    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this);
    };

public:

    /**
     * Constructor
     * @param imageViewerPlugin Reference to image viewer plugin
     */
    MainToolbarAction(LineplotPlugin& lineplotPlugin);

    /** Get reference to image viewer widget */
    LineplotWidget& getLineplotWidget();

public: // Action getters

    GlobalSettingsAction& getGlobalSettingsAction() { return _globalSettingsAction; }

protected:
    LineplotPlugin& _lineplotPlugin;                 /** Reference to image viewer plugin */
    GlobalSettingsAction    _globalSettingsAction;          /** Global view settings action */
};