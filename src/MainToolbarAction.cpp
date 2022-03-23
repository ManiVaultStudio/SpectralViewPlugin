#include "MainToolbarAction.h"
#include "LineplotPlugin.h"
#include "LineplotWidget.h"

#include <Application.h>
#include <QHBoxLayout>

using namespace hdps::util;

MainToolbarAction::MainToolbarAction(LineplotPlugin& lineplotPlugin) :
	WidgetAction(&lineplotPlugin),
	_lineplotPlugin(lineplotPlugin),
	_globalSettingsAction(lineplotPlugin)
{
	setText("Settings");

	auto& fontAwesome = hdps::Application::getIconFont("FontAwesome");

}

MainToolbarAction::Widget::Widget(QWidget* parent, MainToolbarAction* interactionAction) :
    WidgetActionWidget(parent, interactionAction)
{
    setAutoFillBackground(true);

    auto layout = new QHBoxLayout();

    layout->setSpacing(3);
    layout->setMargin(4);

    layout->addStretch(1);
    layout->addWidget(interactionAction->getGlobalSettingsAction().createCollapsedWidget(this));

    setLayout(layout);
}

LineplotWidget& MainToolbarAction::getLineplotWidget()
{
    return _lineplotPlugin.getLineplotWidget();
}
