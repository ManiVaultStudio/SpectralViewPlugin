#include "MainToolbarAction.h"
#include "LineplotPlugin.h"
#include "LineplotWidget.h"

#include <Application.h>
#include <QHBoxLayout>

using namespace hdps::util;

MainToolbarAction::MainToolbarAction(LineplotPlugin& lineplotPlugin) :
	WidgetAction(&lineplotPlugin),
	_lineplotPlugin(lineplotPlugin),
	_globalSettingsAction(lineplotPlugin),
    _wavelengthsRGBAction(lineplotPlugin)
{
	setText("Settings");

	auto& fontAwesome = hdps::Application::getIconFont("FontAwesome");

}

QMenu* MainToolbarAction::getContextMenu() {

    auto menu = new QMenu();

    menu->addMenu(_globalSettingsAction.getContextMenu());

    return menu;
}

MainToolbarAction::Widget::Widget(QWidget* parent, MainToolbarAction* interactionAction) :
    WidgetActionWidget(parent, interactionAction, Widget::State::Standard),
    _layout(),
    _toolBarWidget(),
    _toolBarLayout(),
    _stateWidgets()
{
    setAutoFillBackground(true);

    _toolBarLayout.setSpacing(3);
    _toolBarLayout.setMargin(4);
    _toolBarLayout.setSizeConstraint(QLayout::SetFixedSize);

    addStateWidget(&interactionAction->_globalSettingsAction, 2);
    addStateWidget(&interactionAction->_wavelengthsRGBAction, 1);

    _toolBarLayout.addStretch(1);

    _toolBarWidget.setLayout(&_toolBarLayout);

    _layout.addWidget(&_toolBarWidget);
    _layout.addStretch(1);

    setLayout(&_layout);

    _layout.setMargin(4);

    this->installEventFilter(this);
    _toolBarWidget.installEventFilter(this);
}

bool MainToolbarAction::Widget::eventFilter(QObject* object, QEvent* event)
{
    switch (event->type())
    {
    case QEvent::Resize:
        updateLayout();
        break;

    default:
        break;
    }

    return QObject::eventFilter(object, event);
}

void MainToolbarAction::Widget::addStateWidget(WidgetAction* widgetAction, const std::int32_t& priority /*= 0*/)
{
    _stateWidgets << new WidgetActionStateWidget(this, widgetAction, priority);

    _layout.addWidget(_stateWidgets.back());
}

void MainToolbarAction::Widget::updateLayout()
{
    QMap<WidgetActionStateWidget*, Widget::State> states;

    for (auto stateWidget : _stateWidgets)
        states[stateWidget] = Widget::State::Collapsed;

    const auto getWidth = [this, &states]() -> std::uint32_t {
        std::uint32_t width = 2 * _layout.margin();

        for (auto stateWidget : _stateWidgets)
            width += stateWidget->getSizeHint(states[stateWidget]).width();

        return width;
    };

    auto prioritySortedStateWidgets = _stateWidgets;

    std::sort(prioritySortedStateWidgets.begin(), prioritySortedStateWidgets.end(), [](WidgetActionStateWidget* stateWidgetA, WidgetActionStateWidget* stateWidgetB) {
        return stateWidgetA->getPriority() > stateWidgetB->getPriority();
        });

    for (auto stateWidget : prioritySortedStateWidgets) {
        auto cachedStates = states;

        states[stateWidget] = Widget::State::Standard;

        if (getWidth() > static_cast<std::uint32_t>(width())) {
            states = cachedStates;
            break;
        }
    }

    for (auto stateWidget : _stateWidgets)
        stateWidget->setState(states[stateWidget]);
}

LineplotWidget& MainToolbarAction::getLineplotWidget()
{
    return _lineplotPlugin.getLineplotWidget();
}
