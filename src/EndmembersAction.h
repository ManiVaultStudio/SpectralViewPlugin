#pragma once

#include "actions/Actions.h"

#include <QRandomGenerator>

class QMenu;
class SettingsAction;

using namespace mv::gui;

/**
 * Endmembers action class
 *
 * Action class for endmember display and interaction
 *
 * Based on LayersAction in ImageViewerPlugin
 */
class EndmembersAction : public WidgetAction
{
public:

    /** Widget class for layers action */
    class Widget : public WidgetActionWidget
    {
    protected:

        /**
         * Constructor
         * @param parent Pointer to parent widget
         * @param layersAction Pointer to layers action
         */
        Widget(QWidget* parent, EndmembersAction* endmembersAction);

    protected:
        TriggerAction   _removeEndmemberAction;         /** Remove endmember action */
        TriggerAction   _saveEndmembersAction;      /** Save endmembers action */

        friend class EndmembersAction;
    };

protected:

    /**
     * Get widget representation of the layers action
     * @param parent Pointer to parent widget
     * @param widgetFlags Widget flags for the configuration of the widget
     */
    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this);
    };

public:

    /**
     * Constructor
     * @param settingsAction Reference to settings action
     */
    EndmembersAction(SettingsAction& settingsAction);

    /** Get pseudo-random layer color */
    QColor getRandomLayerColor();

public: // Action getters

    SettingsAction& getSettingsAction() { return _settingsAction; }
    GroupsAction& getCurrentEndmemberAction() { return _currentEndmemberAction; }

protected:
    SettingsAction&     _settingsAction;        /** Reference to settings action */
    GroupsAction        _currentEndmemberAction;    /** Current endmember action */
    QRandomGenerator    _rng;                   /** Random number generator for pseudo-random colors */
};