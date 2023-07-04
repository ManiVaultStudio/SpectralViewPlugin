#pragma once

#include "EndmembersAction.h"
#include <actions/WidgetAction.h>

class SpectralViewPlugin;

using namespace hdps::gui;

/**
 * Settings action class
 *
 * Action class for line plot plugin settings (panel on the right of the view)
 * 
 * MainToolbarAction, SettingsAction and ViewSettingsAction 
 * based on the implementations in ImageViewerPlugin
 */
class SettingsAction : public WidgetAction
{
public:

    /** Widget class for settings action */
    class Widget : public WidgetActionWidget
    {
    protected:

        /**
         * Constructor
         * @param parent Pointer to parent widget
         * @param settingsAction Pointer to settings action
         */
        Widget(QWidget* parent, SettingsAction* settingsAction);

        /** Return preferred size */
        QSize sizeHint() const override {
            return QSize(350, 100);
        }

    protected:
        friend class SettingsAction;
    };

protected:

    /**
     * Get widget representation of the settings action
     * @param parent Pointer to parent widget
     * @param widgetFlags Widget flags for the configuration of the widget (type)
     */
    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this);
    };

public:

    /**
     * Constructor
     * @param SpectralViewPlugin Reference to line plot plugin
     */
    SettingsAction(SpectralViewPlugin& spectralViewPlugin);

    /**
     * Get the context menu for the action
     * @param parent Parent widget
     * @return Context menu
     */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;

    /** Get reference to the line plot plugin */
    SpectralViewPlugin& getSpectralViewPlugin() { return _spectralViewPlugin; };

 public: // Action getters

    EndmembersAction& getEndmembersAction() { return _endmembersAction; }

protected:
    SpectralViewPlugin&     _spectralViewPlugin;    /** Reference to line plot plugin */
    EndmembersAction        _endmembersAction;      /** Endmembers action */
};
