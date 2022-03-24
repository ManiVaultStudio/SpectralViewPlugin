#pragma once

#include "SelectionListener.h"

#include "EndmembersModel.h"
#include "widgets/WebWidget.h"
#include <QMouseEvent>

class QWebEngineView;
class QWebEnginePage;
class QWebChannel;

class Cluster;

class LineplotPlugin;
class LineplotWidget;

class LinePlotCommunicationObject : public hdps::gui::WebCommunicationObject
{
    Q_OBJECT
public:
    LinePlotCommunicationObject(LineplotWidget* parent);

signals:
    void qt_setData(QString data);
    void qt_setEndmember(QString data);
    void qt_setEndmemberColor(int r, int g, int b);
    void qt_addAvailableData(QString name);
    void qt_enableRGBWavelengths(bool checkedRGB);
    void qt_enableStdArea(bool checkedStd);

public slots:
    void js_setRGBWavelengths(float wavelengthR, float wavelengthG, float wavelengthB);
 
private:
    LineplotWidget* _parent;
};

class LineplotWidget : public hdps::gui::WebWidget
{
    Q_OBJECT
public:
    LineplotWidget(LineplotPlugin& lineplotPlugin);
    ~LineplotWidget() override;
    
    void addDataOption(const QString option);
    void setData(std::vector<float>& yVals, std::vector<float>& confIntervalLeft, std::vector<float>& confIntervalRight, std::vector<QString>& dimNames, const int numDimensions, std::string dataOrigin);
    void setEndmemberColor(QColor endmemberColor);
    void enableRGBWavelengths(bool checkedRGB);
    void enableStdArea(bool checkedStd);

protected:
    void mousePressEvent(QMouseEvent* event)   Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* event)    Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

    void onSelection(QRectF selection);
    void cleanup();


signals:
    void changeRGBWavelengths(float wavelengthR, float wavelengthG, float wavelengthB);

public:
    void js_setRGBWavelengths(float wavelengthR, float wavelengthG, float wavelengthB);
  
private slots:
    void initWebPage() override;

private:
    LinePlotCommunicationObject* _communicationObject;

    /** Whether the web view has loaded and web-functions are ready to be called. */
    bool loaded;
    /** Temporary storage for added data options until webview is loaded */
    QList<QString> dataOptionBuffer;
};