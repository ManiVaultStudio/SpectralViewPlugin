#pragma once

#include "EndmembersModel.h"
#include "widgets/WebWidget.h"
#include <QMouseEvent>

class QWebEngineView;
class QWebEnginePage;
class QWebChannel;

class Cluster;

class SpectralViewPlugin;
class LineplotWidget;

class LinePlotCommunicationObject : public mv::gui::WebCommunicationObject
{
    Q_OBJECT
public:
    LinePlotCommunicationObject(LineplotWidget* parent);

signals:
    void qt_setData(QString data);
    void qt_setEndmember(QString data);
    void qt_setEndmemberColor(int r, int g, int b, int row);
    void qt_setEndmemberVisibility(bool toggled, int row);
    void qt_setEndmemberRemoved(int row);
    void qt_setHighlightSelection(int row);
    void qt_addAvailableData(QString name);
    void qt_enableRGBWavelengths(bool checkedRGB);
    void qt_enableStdArea(bool checkedStd);
    void qt_updateRedLine(float newWavelength);
    void qt_updateGreenLine(float newWavelength);
    void qt_updateBlueLine(float newWavelength);

public slots:
    void js_setRGBWavelength(float wavelengthR, int index);
 
private:
    LineplotWidget* _parent;
};

class LineplotWidget : public mv::gui::WebWidget
{
    Q_OBJECT
public:
    LineplotWidget(SpectralViewPlugin& spectralViewPlugin);
    ~LineplotWidget() override;
    
    void addDataOption(const QString option);
    void setData(std::vector<float>& yVals, std::vector<float>& confIntervalLeft, std::vector<float>& confIntervalRight, std::vector<QString>& dimNames, const int numDimensions, std::string dataOrigin);
    void setEndmemberColor(QColor endmemberColor, int row);
    void setEndmemberVisibility(bool toggled, int row);
    void setEndmemberRemoved(int row);
    void setHighlightSelection(int row);
    void enableRGBWavelengths(bool checkedRGB);
    void enableStdArea(bool checkedStd);
    void updateRGBLine(float newWavelength, int index);

protected:
    void mousePressEvent(QMouseEvent* event)   Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* event)    Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

    void onSelection(QRectF selection);
    void cleanup();


signals:
    void changeRGBWavelengths(float wavelengthR, int index);

public:
    void js_setRGBWavelength(float wavelength, int index);
  
private slots:
    void initWebPage() override;

private:
    LinePlotCommunicationObject* _communicationObject;

    /** Whether the web view has loaded and web-functions are ready to be called. */
    bool loaded;
    /** Temporary storage for added data options until webview is loaded */
    QList<QString> dataOptionBuffer;
};
