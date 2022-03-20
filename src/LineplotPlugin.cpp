// Code based on HeatMapPlugin
#pragma once

#include "LineplotPlugin.h"

#include "PointData.h"
#include "ClusterData.h"
#include "event/Event.h"
#include "ImageData/Images.h"

#include <QtCore>
#include <QtDebug>
#include <iostream>


Q_PLUGIN_METADATA(IID "nl.tudelft.LineplotPlugin")

using namespace hdps;
using namespace hdps::gui;

LineplotPlugin::LineplotPlugin(const PluginFactory* factory) :
    ViewPlugin(factory),
    _splitter(Qt::Horizontal, &getWidget()),
    _linePlotWidget(*this),
    _dropWidget(&_linePlotWidget),
    _mainToolbarAction(*this)
   // _settingsAction(*this)
{
    setObjectName("Line Plot");

    getWidget().setContextMenuPolicy(Qt::CustomContextMenu);
    getWidget().setFocusPolicy(Qt::ClickFocus);

    //_linePlotWidget = new LineplotWidget();
    //_dropWidget = new DropWidget(_linePlotWidget);
    _mainToolbarAction.setEnabled(false);
}

LineplotPlugin::~LineplotPlugin() {
}

void LineplotPlugin::init()
{
    _dropWidget.setDropIndicatorWidget(new DropWidget::DropIndicatorWidget(&getWidget(), "No data loaded", "Drag an item from the data hierarchy and drop it here to visualize data..."));

    _linePlotWidget.setPage(":/lineplot/lineplot.html", "qrc:/lineplot/");

    auto mainLayout = new QHBoxLayout();

    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    getWidget().setLayout(mainLayout);

    // Create main left widget
    auto mainWidget = new QWidget();

    // Create main widget layout
    auto mainWidgetLayout = new QVBoxLayout();

    // Configure main layout
    mainWidgetLayout->setMargin(0);
    mainWidgetLayout->setSpacing(0);

    // And add the toolbar, image viewer widget
    mainWidgetLayout->addWidget(_mainToolbarAction.createWidget(&getWidget()));
    mainWidgetLayout->addWidget(&_linePlotWidget, 1);

    // Apply layout to main widget
    mainWidget->setLayout(mainWidgetLayout);

    // Add viewer widget and settings panel to the splitter
    _splitter.addWidget(mainWidget);
    //_splitter.addWidget(_settingsAction.createWidget(&getWidget()));

    // Configure splitter
    _splitter.setStretchFactor(0, 1);
    //_splitter.setStretchFactor(1, 0);
    //_splitter.setCollapsible(1, true);

    // Add splitter to the main layout
    mainLayout->addWidget(&_splitter);

    _dropWidget.initialize([this](const QMimeData* mimeData) -> DropWidget::DropRegions {
        DropWidget::DropRegions dropRegions;

        const auto mimeText = mimeData->text();
        const auto tokens = mimeText.split("\n");

        if (tokens.count() == 1)
            return dropRegions;

        const auto datasetGuiName = tokens[0];
        const auto datasetId = tokens[1];
        const auto dataType = DataType(tokens[2]);
        const auto dataTypes = DataTypes({ PointType, ClusterType });

        // Check if the data type can be dropped
        if (!dataTypes.contains(dataType)) {
            dropRegions << new DropWidget::DropRegion(this, "Incompatible data", "This type of data is not supported", "exclamation-circle", false);
        }
    
        // Points dataset is about to be dropped
        if (dataType == PointType) {

            // Get points dataset from the core
            hdps::Dataset<Points>  candidateDataset = _core->requestDataset<Points>(datasetId);
            int numDimensions = candidateDataset->getNumDimensions();
            auto dimensions = candidateDataset->getDimensionNames();
               
               
            // Establish drop region description
            const auto description = QString("Visualize %1 as line plot").arg(datasetGuiName);

            if (!_points.isValid()) {
                //_points = candidateDataset;
                // Load as point positions when no dataset is currently loaded
                dropRegions << new DropWidget::DropRegion(this, "Point position", description, "map-marker-alt", true, [this, candidateDataset]() {
                    _points = candidateDataset;
                    
                    initializeImageRGB();

                    _mainToolbarAction.setEnabled(true);

                    });
            }
            else {
                if (_points == candidateDataset) {
                    // already loaded
                    dropRegions << new DropWidget::DropRegion(this, "Warning", "Data already loaded", "exclamation-circle", false);
                }
                else {
                    dropRegions << new DropWidget::DropRegion(this, "Points", description, "map-marker-alt", true, [this, candidateDataset]() {
                        _points = candidateDataset;
                        });
                }
            }
        }
        
        if (dataType == ClusterType) {
            const auto candidateDataset = _core->requestDataset<Clusters>(datasetId);
            const auto description = QString("Clusters points by %1").arg(candidateDataset->getGuiName());

            if (_points.isValid()) {
                if (candidateDataset == _clusters) {
                    dropRegions << new gui::DropWidget::DropRegion(this, "Clusters", "Cluster set is already in use", "exclamation-circle", false, [this]() {});
                }
                else {
                    dropRegions << new gui::DropWidget::DropRegion(this, "Clusters", description, "th-large", true, [this, candidateDataset]() {
                        _clusters = candidateDataset;
                        });
                }
            }
        } 

        return dropRegions;
    });

    
    const auto updateWindowTitle = [this]() -> void {
        if (!_points.isValid())
            getWidget().setWindowTitle(getGuiName());
        else
            getWidget().setWindowTitle(QString("%1: %2").arg(getGuiName(), _points->getGuiName()));
    };

    // Load points when the dataset name of the points dataset reference changes
    connect(&_points, &Dataset<Points>::changed, this, [this, updateWindowTitle]() {
        //loadPoints(newDatasetName);
        _dropWidget.setShowDropIndicator(false);
        updateWindowTitle();
        });

    // Load clusters when the dataset name of the clusters dataset reference changes
    connect(&_clusters, &Dataset<Clusters>::changed, this, [this, updateWindowTitle]() {
        //loadPoints(newDatasetName);
        updateWindowTitle();
        //updateData();
        });

    //connect(_linePlotWidget, SIGNAL(clusterSelectionChanged(QList<int>)), SLOT(clusterSelected(QList<int>)));
    connect(&_linePlotWidget, SIGNAL(changeRGBWavelengths(float, float, float)), SLOT(changeRGBWavelengths(float, float, float)));

    // Respond when the name of the dataset in the dataset reference changes
    connect(&_points, &Dataset<Points>::dataGuiNameChanged, this, [this](const QString& oldDatasetName, const QString& newDatasetName) {

        // Update the current dataset name label
        //_linePlotWidget->setText(QString("Current points dataset: %1").arg(newDatasetName));

        // Only show the drop indicator when nothing is loaded in the dataset reference
        _dropWidget.setShowDropIndicator(newDatasetName.isEmpty());
        });

    registerDataEventByType(PointType, std::bind(&LineplotPlugin::onDataEvent, this, std::placeholders::_1));
}


void LineplotPlugin::onDataEvent(hdps::DataEvent* dataEvent)
{
    // Get smart pointer to dataset that changed
    const auto changedDataSet = dataEvent->getDataset();

    // Get GUI name of the dataset that changed
    const auto datasetGuiName = changedDataSet->getGuiName();

    const auto datasetGuid = changedDataSet->getGuid();

    switch (dataEvent->getType()) {

        // Event which gets triggered when a dataset is added to the system.
        case EventType::DataAdded:
        {
            _linePlotWidget.addDataOption(dataEvent->getDataset()->getGuiName());

            // Cast the data event to a data added event
            const auto dataAddedEvent = static_cast<DataAddedEvent*>(dataEvent);

            // Get the GUI name of the added points dataset and print to the console
            std::cout << datasetGuiName.toStdString() << " was added" << std::endl;

            if (datasetGuiName == "endmemberList") {
                importEndmembersCSV(datasetGuid);
            }
            break;

            break;
        }

        // Event which gets triggered when the data contained in a dataset changes.
        case EventType::DataChanged:
        {
            // Cast the data event to a data changed event
            const auto dataChangedEvent = static_cast<DataChangedEvent*>(dataEvent);

            // Get the name of the points dataset of which the data changed and print to the console
            std::cout << datasetGuiName.toStdString() << " data changed" << std::endl;
        }

        // Points dataset data was removed
        case EventType::DataRemoved:
        {
            // Cast the data event to a data removed event
            const auto dataRemovedEvent = static_cast<DataRemovedEvent*>(dataEvent);

            // Get the name of the removed points dataset and print to the console
            std::cout << datasetGuiName.toStdString() << " was removed" << std::endl;

            break;
        }

        // Points dataset selection has changed
        case EventType::DataSelectionChanged:
        {
            // Cast the data event to a data selection changed event
            const auto dataSelectionChangedEvent = static_cast<DataSelectionChangedEvent*>(dataEvent);

            // Get the selection set that changed
            const auto& selectionSet = changedDataSet->getSelection<Points>();

            // Print to the console
            std::cout << datasetGuiName.toStdString() << "selection has changed" << std::endl;

            if(_points.isValid())
                updateSelection(selectionSet);

            break;
        }

        default:
            break;
    }
}

void LineplotPlugin::importEndmembersCSV(const QString datasetGuid) {

    auto endmembers = _core->requestDataset(datasetGuid);
    auto endmemberPoints = endmembers->getSourceDataset<Points>();
    auto numDimensions = endmemberPoints->getNumDimensions();
    auto endmembersNo = endmemberPoints->getNumPoints();

    std::vector<QString> dimNames;
    std::vector<float> endmember;

    for (int v = 0; v < numDimensions; v++) {
        auto value = endmemberPoints->getValueAt(v);
        dimNames.push_back(QString::number(value));
    }

    for (int i = 1; i < endmembersNo; i++) {

        //endmember.clear();

        for (int v = 0; v < numDimensions; v++) {
            endmember.push_back(endmemberPoints->getValueAt(i * numDimensions + v));
        }

        // pass the endmemeber
    }

    endmemberPoints->setData(endmember.data(), endmembersNo - 1, numDimensions);
    endmemberPoints->setDimensionNames(dimNames);
    _core->notifyDatasetChanged(endmemberPoints);


}

void LineplotPlugin::initializeImageRGB() {

    if (!_imageRGBPoints.isValid()) {
        _imageRGBPoints = _core->addDataset<Points>("Points", "imageRGBData");
        _imageRGB = _core->addDataset<Images>("Images", "images", hdps::Dataset<hdps::DatasetImpl>(*_imageRGBPoints));
        //_core->notifyDatasetAdded(_imageRGB);
        //_core->notifyDatasetAdded(_imageRGBPoints);

        qDebug() << "Create image";

        auto source = _points->getSourceDataset<Points>();
        auto dimNames = _points->getDimensionNames();
        unsigned int numDimensions = _points->getNumDimensions();
       // auto images = _points->getSourceDataset<Images>();
        //auto images = _points->getDataHierarchyItem().getChildren();
       // auto imageSize = images->getImageSize(); //source->getProperty("width").toInt();
       // int width = imageSize.width();
       // int height = imageSize.height();
        int width = source->getProperty("width").toInt();
        int height = source->getProperty("height").toInt();
        int numPoints = width * height;

        std::vector<float> imageRGBData(width * height * 3);
        std::vector<QString> imageDim;

        // needs to change
        float wavelengthR = 630;
        float wavelengthG = 532;
        float wavelengthB = 465;

        int dimR = 0;
        int dimG = 0;
        int dimB = 0;

        for (int v = 0; v < numDimensions; v++) {
            auto dimName = dimNames.at(v);
            float dimValue = dimName.toFloat();
      
            if (abs(wavelengthR - dimValue) < 1) {
                dimR = v;
                imageDim.push_back(dimName);
            }
            else if (abs(wavelengthG - dimValue) < 1) {
                dimG = v;
                imageDim.push_back(dimName);
            }
            else if (abs(wavelengthB - dimValue) < 1) {
                dimB = v;
                imageDim.push_back(dimName);
            }
        }

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                imageRGBData[width * 3 * (height - y - 1) + 3 * x] = source->getValueAt(width * numDimensions * (height - y - 1) + numDimensions * x + dimR);
                imageRGBData[width * 3 * (height - y - 1) + 3 * x + 1] = source->getValueAt(width * numDimensions * (height - y - 1) + numDimensions * x + dimG);
                imageRGBData[width * 3 * (height - y - 1) + 3 * x + 2] = source->getValueAt(width * numDimensions * (height - y - 1) + numDimensions * x + dimB);
            }
        }

        _imageRGBPoints->setData(imageRGBData.data(), numPoints, 3);
        _imageRGBPoints->setDimensionNames(imageDim);
        _imageRGBPoints->setProperty("width", width);
        _imageRGBPoints->setProperty("height", height);

        _core->notifyDatasetAdded(_imageRGBPoints);

        _imageRGB->setGuiName("RGB Image");
        _imageRGB->setType(ImageData::Type::Stack);
        _imageRGB->setNumberOfImages(1);
        _imageRGB->setImageSize(QSize(width, height));
        _imageRGB->setNumberOfComponentsPerPixel(3);

        _core->notifyDatasetAdded(_imageRGB);
    }
}

void LineplotPlugin::changeRGBWavelengths(const float wavelengthR, const float wavelengthG, const float wavelengthB) {

    auto source = _points->getSourceDataset<Points>();
    auto dimNames = _points->getDimensionNames();
    auto numDimensions = _points->getNumDimensions();
    int width = _points->getProperty("width").toInt();
    int height = _points->getProperty("height").toInt();
    int numPoints = width * height;

    std::vector<float> imageRGBData(width * height * 3);

    int dimR = 0;
    int dimG = 0;
    int dimB = 0;

    for (int v = 0; v < numDimensions; v++) {
        float dimValue = dimNames.at(v).toFloat();
        if (abs(wavelengthR - dimValue) < 0.001) {
            dimR = v;
        }
        if (abs(wavelengthG - dimValue) < 0.001) {
            dimG = v;
        }
        if (abs(wavelengthB - dimValue) < 0.001) {
            dimB = v;
        }
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            imageRGBData[width * 3 * (height - y - 1) + 3 * x] = source->getValueAt(width * numDimensions * (height - y - 1) + numDimensions * x + dimR);
            imageRGBData[width * 3 * (height - y - 1) + 3 * x + 1] = source->getValueAt(width * numDimensions * (height - y - 1) + numDimensions * x + dimG);
            imageRGBData[width * 3 * (height - y - 1) + 3 * x + 2] = source->getValueAt(width * numDimensions * (height - y - 1) + numDimensions * x + dimB);
        }
    }

    _imageRGBPoints->setData(imageRGBData.data(), numPoints, 3);
    _core->notifyDatasetChanged(_imageRGBPoints);
    _core->notifyDatasetChanged(_imageRGB);
  
}

void LineplotPlugin::updateSelection(Dataset<Points> selection) {

    if (selection.isValid()) {

        auto source = _points->getSourceDataset<Points>();
        auto numDimensions = source->getNumDimensions();
        int width = source->getProperty("width").toInt();
        int height = source->getProperty("height").toInt();

        auto selectedIndices = selection->indices;
        auto noSelectedPoints = selection->getSelectionSize();

        std::vector<float> averageSpectrum;
        std::vector<float> confIntervalLeft(numDimensions);
        std::vector <float> confIntervalRight(numDimensions);

        for (int v = 0; v < numDimensions; v++) {

            float sum = 0;

            for (int i = 0; i < noSelectedPoints; i++) {
                auto index = selectedIndices.at(i);
                int x = index / width;
                int y = index - (x * width);
                sum = sum + source->getValueAt(width * numDimensions * (height - x - 1) + numDimensions * y + v);
            
            }

            float mean = sum / noSelectedPoints;
            averageSpectrum.push_back(mean);
            
            if (noSelectedPoints > 1) {
                // compute standard deviation per dimension                
                float std = 0;

                for (int i = 0; i < noSelectedPoints; i++) {
                    auto index = selectedIndices.at(i);
                    int x = index / width;
                    int y = index - (x * width);
                    float value = source->getValueAt(width * numDimensions * (height - x - 1) + numDimensions * y + v);
                    std = std + (value - mean) * (value - mean);
                }

                std = sqrt(std / noSelectedPoints);

                confIntervalRight[v] = mean + std;
                confIntervalLeft[v] = mean - std;
            }
        }   

        qDebug() << "Got average";

        std::vector<QString> names;
        if (source->getDimensionNames().size() == source->getNumDimensions()) {
            names = source->getDimensionNames();
        }

        qDebug() << "Send data";

        _linePlotWidget.setData(averageSpectrum, confIntervalLeft, confIntervalRight, names, numDimensions);
    }
}


QIcon LineplotPluginFactory::getIcon() const
{
    return Application::getIconFont("FontAwesome").getIcon("chart-line");
}

// =============================================================================
// Factory
// =============================================================================

ViewPlugin* LineplotPluginFactory::produce()
{
    return new LineplotPlugin(this);
}

hdps::DataTypes LineplotPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    supportedTypes.append(ClusterType);
    return supportedTypes;
}
