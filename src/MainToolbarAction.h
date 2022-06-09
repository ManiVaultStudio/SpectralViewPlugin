#pragma once

#include "actions/WidgetActionStateWidget.h"
#include "GlobalSettingsAction.h"
#include "WavelengthsRGBAction.h"
#include <QHBoxLayout>


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

        bool eventFilter(QObject* object, QEvent* event);

protected:
    void addStateWidget(WidgetAction* widgetAction, const std::int32_t& priority = 0);

private:
    void updateLayout();

    protected:
        QHBoxLayout                         _layout;
        QWidget                             _toolBarWidget;
        QHBoxLayout                         _toolBarLayout;
        QVector<WidgetActionStateWidget*>   _stateWidgets;

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

    QMenu* getContextMenu();

    /** Get reference to lineplot widget */
    LineplotWidget& getLineplotWidget();

public: // Action getters

    GlobalSettingsAction& getGlobalSettingsAction() { return _globalSettingsAction; }
    WavelengthsRGBAction& getWavelengthsRGBAction() { return _wavelengthsRGBAction; }

protected:
    LineplotPlugin& _lineplotPlugin;                 /** Reference to image viewer plugin */
    GlobalSettingsAction    _globalSettingsAction;          /** Global view settings action */
    WavelengthsRGBAction    _wavelengthsRGBAction;
};