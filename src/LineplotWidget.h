#pragma once

#include "SelectionListener.h"

#include "widgets/WebWidget.h"
#include <QMouseEvent>

class QWebEngineView;
class QWebEnginePage;
class QWebChannel;

class Cluster;

class LineplotWidget;

class LinePlotCommunicationObject : public hdps::gui::WebCommunicationObject
{
    Q_OBJECT
public:
    LinePlotCommunicationObject(LineplotWidget* parent);

signals:
    void qt_setData(QString data);
    void qt_addAvailableData(QString name);
    //void qt_setSelection(QList<int> selection);
    //void qt_setHighlight(int highlightId);
    //void qt_setMarkerSelection(QList<int> selection);

public slots:
    void js_setRGBWavelengths(float wavelengthR, float wavelengthG, float wavelengthB);
    //void js_selectData(QString text);
    //void js_selectionUpdated(QVariant selectedClusters);
    //void js_highlightUpdated(int highlightId);
 
private:
    LineplotWidget* _parent;
};

class LineplotWidget : public hdps::gui::WebWidget
{
    Q_OBJECT
public:
    LineplotWidget();
    ~LineplotWidget() override;
    
    void addDataOption(const QString option);
    void setData(std::vector<float>& yVals, std::vector<float>& confIntervalLeft, std::vector<float>& confIntervalRight, std::vector<QString>& dimNames, const int numDimensions);
    
protected:
    void mousePressEvent(QMouseEvent* event)   Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* event)    Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

    void onSelection(QRectF selection);
    void cleanup();


signals:
    //void clusterSelectionChanged(QList<int> selectedClusters);
   // void dataSetPicked(QString name);
    void changeRGBWavelengths(float wavelengthR, float wavelengthG, float wavelengthB);

public:
    void js_setRGBWavelengths(float wavelengthR, float wavelengthG, float wavelengthB);
   // void js_selectData(QString text);
   // void js_selectionUpdated(QVariant selectedClusters);
   // void js_highlightUpdated(int highlightId);
  
private slots:
    void initWebPage() override;

private:
    LinePlotCommunicationObject* _communicationObject;

    //unsigned int _numClusters;

    /** Whether the web view has loaded and web-functions are ready to be called. */
    bool loaded;
    /** Temporary storage for added data options until webview is loaded */
    QList<QString> dataOptionBuffer;
};