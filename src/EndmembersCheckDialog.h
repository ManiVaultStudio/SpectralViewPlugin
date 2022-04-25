#pragma once

#include <actions/StringAction.h>
#include <actions/TriggerAction.h>
#include <actions/GroupAction.h>

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

#include "LineplotPlugin.h"

using namespace hdps::gui;

// =============================================================================
// Check the number of endmembers to load
// =============================================================================

class EndmembersCheckDialog : public QDialog
{
    Q_OBJECT
public:
    EndmembersCheckDialog(QWidget* parent, int noEndmembers);

signals:
    void closeDialog();

public slots:
    void closeDialogAction(bool checked = false);

private:
    StringAction        _numberOfEndmembersAction;  /** Number of endmembers action */
    TriggerAction       _loadAction;                /** Load action */
    GroupAction         _groupAction;               /** Group action */
};