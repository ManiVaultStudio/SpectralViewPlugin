#include "EndmembersCheckDialog.h"
#include "PointData/PointData.h"

EndmembersCheckDialog::EndmembersCheckDialog(QWidget* parent, int noEndmembers) :
    QDialog(parent),
    _numberOfEndmembersAction(this, "Number of endmembers to load"),
    _loadAction(this, "Load"),
    _closeAction(this, "Don't load"),
    _groupAction(this, "EndmemberGroupAction")
{
    setWindowTitle(tr("Check number of endmembers"));

    _numberOfEndmembersAction.setString(QString::number(noEndmembers));
    _numberOfEndmembersAction.setEnabled(false);

    _groupAction.addAction(&_numberOfEndmembersAction);
    _groupAction.addAction(&_closeAction);
    _groupAction.addAction(&_loadAction);
    
    auto layout = new QVBoxLayout();

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_groupAction.createWidget(this));

    setLayout(layout);

    connect(&_loadAction, &TriggerAction::triggered, this, &EndmembersCheckDialog::loadEndmembersAction);
    connect(&_closeAction, &TriggerAction::triggered, this, &EndmembersCheckDialog::closeDialogAction);
}

void EndmembersCheckDialog::closeDialogAction(bool checked)
{
    accept();
}

void EndmembersCheckDialog::loadEndmembersAction(bool checked)
{
    emit closeDialog();

    accept();
}
