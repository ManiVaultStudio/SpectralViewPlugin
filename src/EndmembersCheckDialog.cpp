#include "EndmembersCheckDialog.h"
#include "PointData.h"

EndmembersCheckDialog::EndmembersCheckDialog(QWidget* parent, int noEndmembers) :
    QDialog(parent),
    _numberOfEndmembersAction(this, "Number of endmembers to load"),
    _loadAction(this, "Load"),
    _groupAction(this)
{
    setWindowTitle(tr("Check number of endmembers"));

    _numberOfEndmembersAction.setString(QString::number(noEndmembers));
    _numberOfEndmembersAction.setEnabled(false);

    _groupAction << _numberOfEndmembersAction;
    _groupAction << _loadAction;

    auto layout = new QVBoxLayout();

    layout->setMargin(0);
    layout->addWidget(_groupAction.createWidget(this));

    setLayout(layout);

    connect(&_loadAction, &TriggerAction::triggered, this, &EndmembersCheckDialog::closeDialogAction);
}

void EndmembersCheckDialog::closeDialogAction(bool checked)
{
    emit closeDialog();

    accept();
}