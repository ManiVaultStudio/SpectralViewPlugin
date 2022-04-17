// Code based on HeatMapPlugin
#pragma once

#include "LineplotPlugin.h"

#include "PointData.h"
#include "ClusterData.h"
#include "event/Event.h"
#include "ImageData/Images.h"

#include "DataHierarchyItem.h"

#include <QtCore>
#include <QtDebug>
#include <iostream>

#include<tuple>


Q_PLUGIN_METADATA(IID "nl.tudelft.LineplotPlugin")

using namespace hdps;
using namespace hdps::gui;

LineplotPlugin::LineplotPlugin(const PluginFactory* factory) :
    ViewPlugin(factory),
    _model(this),
    _selectionModel(&_model),
    _splitter(Qt::Horizontal, &getWidget()),
    _linePlotWidget(*this),
    _dropWidget(&_linePlotWidget),
    _mainToolbarAction(*this),
    _settingsAction(*this),
    _map(),
    _mapImage(),
    _imageRGB(),
    _imageRGBPoints()
{
    setObjectName("Line Plot");

    getWidget().setContextMenuPolicy(Qt::CustomContextMenu);
    getWidget().setFocusPolicy(Qt::ClickFocus);

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
    _splitter.addWidget(_settingsAction.createWidget(&getWidget()));

    // Configure splitter
    //_splitter.setStretchFactor(0, 1);
    _splitter.setStretchFactor(1, 0);
    _splitter.setCollapsible(1, true);

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

        // Get points dataset from the core
        auto candidateDataset = _core->requestDataset(datasetId);

        // Check if the data type can be dropped
        if (!dataTypes.contains(dataType)) {
            dropRegions << new DropWidget::DropRegion(this, "Incompatible data", "This type of data is not supported", "exclamation-circle", false);
        }
    
        // Points dataset is about to be dropped
        if (dataType == PointType) {
            auto noPointsCandiateData = candidateDataset.get<Points>()->getNumPoints();

            // Establish drop region description
            const auto description = QString("Visualize %1 as line plot").arg(datasetGuiName);

            try {

                if (!_points.isValid()) {
                    
                    // Load as point positions when no dataset is currently loaded
                    dropRegions << new DropWidget::DropRegion(this, "Point position", description, "map-marker-alt", true, [this, candidateDataset]() {
                        _points = candidateDataset.get<Points>();

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
                        // Establish drop region description
                        const auto description1 = QString("Visualize every point in %1 as one line").arg(datasetGuiName);
                        const auto description2 = QString("Visaualize the points in %1 as an average line").arg(datasetGuiName);

                        if (!candidateDataset->isFull() || candidateDataset->isDerivedData()) {

                            DataHierarchyItems parents;

                            hdps::DataHierarchyItem::getParents(candidateDataset->getDataHierarchyItem(), parents);

                            if (parents.at(0)->getGuiName() == _points->getGuiName()) {

                                dropRegions << new DropWidget::DropRegion(this, "Endmembers", description1, "map-marker-alt", true, [this, candidateDataset]() {
                                    addDataset(candidateDataset);
                                    });

                                dropRegions << new DropWidget::DropRegion(this, "Average endmember", description2, "map-marker-alt", true, [this, candidateDataset]() {
                                    addAverageDataset(candidateDataset);
                                    });
                            }
                        }
                        else {
                            dropRegions << new DropWidget::DropRegion(this, "Points", description, "map-marker-alt", true, [this, candidateDataset]() {
                                _points = candidateDataset;

                                initializeImageRGB();
                                _mainToolbarAction.setEnabled(true);
                                });
                        }
                    }
                }
            }
            catch (std::exception& e)
            {
                exceptionMessageBox(QString("Unable to load '%1'").arg(datasetGuiName), e);
            }
            catch (...) {
                exceptionMessageBox(QString("Unable to load '%1'").arg(datasetGuiName));
            }
        }

        if (dataType == ClusterType) {
           // const auto candidateDataset = _core->requestDataset<Clusters>(datasetId);
            const auto description = QString("Visualize every cluster in %1 as one line").arg(candidateDataset->getGuiName());

            if (_points.isValid()) {
                if (candidateDataset == _clusters) {
                    dropRegions << new gui::DropWidget::DropRegion(this, "Clusters", "Cluster set is already in use", "exclamation-circle", false, [this]() {});
                }
                else {

                    DataHierarchyItems parents;

                    hdps::DataHierarchyItem::getParents(candidateDataset->getDataHierarchyItem(), parents);

                    if (parents.at(0)->getGuiName() == _points->getGuiName()) {

                        dropRegions << new DropWidget::DropRegion(this, "Endmembers", description, "map-marker-alt", true, [this, candidateDataset]() {
                            _clusters = candidateDataset;
                            addDataset(candidateDataset);

                            });
                    }
                }
            }
            else {
                // Only allow user to visualize clusters as lines when there is a points dataset loaded
                dropRegions << new DropWidget::DropRegion(this, "No points data loaded", "Clusters can only be visualized in concert with points data", "exclamation-circle", false);

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
        _dropWidget.setShowDropIndicator(false);
        updateWindowTitle();
        });

    // Load clusters when the dataset name of the clusters dataset reference changes
    connect(&_clusters, &Dataset<Clusters>::changed, this, [this, updateWindowTitle]() {
        updateWindowTitle();
        });

    connect(&_linePlotWidget, SIGNAL(changeRGBWavelengths(float, float, float)), SLOT(changeRGBWavelengths(float, float, float)));

    // Respond when the name of the dataset in the dataset reference changes
    connect(&_points, &Dataset<Points>::dataGuiNameChanged, this, [this](const QString& oldDatasetName, const QString& newDatasetName) {

        // Update the current dataset name label
        //_linePlotWidget.set(QString("Current points dataset: %1").arg(newDatasetName));

        // Only show the drop indicator when nothing is loaded in the dataset reference
        _dropWidget.setShowDropIndicator(newDatasetName.isEmpty());
        });

    registerDataEventByType(PointType, std::bind(&LineplotPlugin::onDataEvent, this, std::placeholders::_1));
    registerDataEventByType(ClusterType, std::bind(&LineplotPlugin::onDataEvent, this, std::placeholders::_1));

    const auto endmembersInsertedRemovedChanged = [this]() {
       // _dropWidget.setShowDropIndicator(_model.rowCount() == 0);

        // Establish the number of visible layers
        const auto hasVisibleEndmembers = _model.rowCount() == 0 ? false : !_model.match(_model.index(0, EndmembersModel::Visible), Qt::EditRole, true, -1).isEmpty();

        // Enabled/disable navigation tool bar
        _mainToolbarAction.setEnabled(hasVisibleEndmembers || _points.isValid());
    };

    // Enable/disable the navigation action when rows are inserted/removed
    connect(&_model, &EndmembersModel::rowsInserted, this, endmembersInsertedRemovedChanged);
    connect(&_model, &EndmembersModel::rowsRemoved, this, endmembersInsertedRemovedChanged);
    connect(&_model, &EndmembersModel::dataChanged, this, endmembersInsertedRemovedChanged);
}


void LineplotPlugin::onDataEvent(hdps::DataEvent* dataEvent)
{
    // Get smart pointer to dataset that changed
    const auto changedDataSet = dataEvent->getDataset();
    auto type = changedDataSet->getDataType();

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

            break;
        }

        // Event which gets triggered when the data contained in a dataset changes.
        case EventType::DataChanged:
        {
            // Cast the data event to a data changed event
            const auto dataChangedEvent = static_cast<DataChangedEvent*>(dataEvent);

            // Get the name of the points dataset of which the data changed and print to the console
            std::cout << datasetGuiName.toStdString() << " data changed" << std::endl;

            if (_clusters.isValid() && type == ClusterType) {
                if (_clusters->getGuiName() == datasetGuiName) {
                    addDataset(changedDataSet);
                }
            }
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

            if(_points.isValid())
                updateSelection(selectionSet);

            break;
        }

        default:
            break;
    }
}

void LineplotPlugin::addDataset(const Dataset<DatasetImpl>& dataset) {
    
    auto type = dataset->getDataType();
    auto parent = dataset->getParent();
    auto numDimensions = parent.get<Points>()->getNumDimensions();

    if (type == PointType) {
        auto points = dataset.get<Points>();
        auto noPoints = points->getNumPoints();
        auto indices = points->indices;

        if (indices.size() != 0) {
            for (int i = 0; i < noPoints; i++) {

                auto endmember = new Endmember(*this, dataset);
                _model.addEndmember(endmember);
                _model.setEndmemberProperties(endmember, -1);

                auto endmemberData = computeAverageSpectrum(parent, 1, { indices[i] }, "endmember");
                endmember->setData(std::get<0>(endmemberData));
            }
        }
        else {
            auto numDimensions = points->getNumDimensions();
            auto dimNames = points->getDimensionNames();

            std::vector<float> endmemberData(numDimensions);
            std::vector<float> confInterval(numDimensions);

            for (int i = 0; i < noPoints; i++) {
                
                for (int v = 0; v < numDimensions; v++) {
                    endmemberData[v] = points->getValueAt(i * numDimensions + v);
                }

                auto endmember = new Endmember(*this, dataset);
                _model.addEndmember(endmember);
                _model.setEndmemberProperties(endmember, -1);

                _linePlotWidget.setData(endmemberData, confInterval, confInterval, dimNames, numDimensions, "endmember");
                endmember->setData(endmemberData);
                
            }
        }
    }
    else if (type == ClusterType) {

        auto clusters = dataset.get<Clusters>()->getClusters();
        auto noClusters = clusters.length();

        //need to change
        auto parent = _points;

        // remove clusters of this dataset
        _model.removeEndmembers(dataset->getGuid());
        
        // add clsuters
        for (int i = 0; i < noClusters; i++) {

            auto noPointsCluster = clusters[i].getNumberOfIndices();
            auto indices = clusters[i].getIndices();

            auto average = clusters[i].getMean();
            auto std = clusters[i].getStandardDeviation();

            auto endmember = new Endmember(*this, dataset);
            _model.addEndmember(endmember);

            if (average.size() == 0) {

                average.resize(numDimensions);
                std.resize(numDimensions);

                _model.setEndmemberProperties(endmember, i);
                auto endmemberData = computeAverageSpectrum(parent, noPointsCluster, indices, "endmember");
                auto computedAvg = std::get<0>(endmemberData);
                auto computedStd = std::get<1>(endmemberData);

                for (int v = 0; v < numDimensions; v++) {
                    average[v] = computedAvg[v];
                    std[v] = computedStd[v];
                }
            }
            else {
                std::vector<float> confIntervalLeft;
                std::vector<float> confIntervalRight;
                auto dimNames = parent.get<Points>()->getDimensionNames();

                _model.setEndmemberProperties(endmember, i);

                for (int v = 0; v < numDimensions; v++) {
                    confIntervalLeft[v] = average[v] - std[v];
                    confIntervalRight[v] = average[v] + std[v];
                }
                _linePlotWidget.setData(average, confIntervalLeft, confIntervalRight, dimNames, numDimensions, "endmember");
            }

            endmember->setData(average);
        }
    }
}

void LineplotPlugin::addAverageDataset(const Dataset<DatasetImpl>& dataset) {

    auto type = dataset->getDataType();
    auto parent = dataset->getParent();

    if (type == PointType) {
        auto points = dataset.get<Points>();
        auto noPoints = points->getNumPoints();
        auto indices = points->indices;

        if (indices.size() != 0) {

            auto endmember = new Endmember(*this, dataset);
            _model.addEndmember(endmember);
            _model.setEndmemberProperties(endmember, 0);

            auto endmemberData = computeAverageSpectrum(parent, noPoints, indices, "endmember");
            endmember->setData(std::get<0>(endmemberData));
        }
    }
}

void LineplotPlugin::initializeImageRGB() {

    if (!_imageRGBPoints.isValid()) {
        _imageRGBPoints = _core->createDerivedDataset("imageRGBData", _points, _points);
        _imageRGB = _core->addDataset<Images>("Images", "images", hdps::Dataset<hdps::DatasetImpl>(*_imageRGBPoints));
        

       // qDebug() << "Create image";

        auto source = _points->getSourceDataset<Points>();
        auto dimNames = _points->getDimensionNames();
        unsigned int numDimensions = _points->getNumDimensions();

        auto children = source->getChildren({ ImageType });
        auto imagesId = children[0].getDatasetGuid();
        auto images = _core->requestDataset<Images>(imagesId);
        auto imageSize = images->getImageSize();
        int width = imageSize.width();
        int height = imageSize.height();

        int numPoints = width * height;

        std::vector<float> imageRGBData(width * height * 3);
        std::vector<QString> imageDim(3);

        // needs to change
        float wavelengthR = 630;
        float wavelengthG = 532;
        float wavelengthB = 464;

        int dimR = 0;
        int dimG = 0;
        int dimB = 0;

        for (int v = 0; v < numDimensions; v++) {
            auto dimName = dimNames.at(v);
            float dimValue = dimName.toFloat();

            if (abs(wavelengthR - dimValue) < 1) {
                dimR = v;
                imageDim[0] = dimName;
            }
            else if (abs(wavelengthG - dimValue) < 1) {
                dimG = v;
                imageDim[1] = dimName;
            }
            else if (abs(wavelengthB - dimValue) < 1) {
                dimB = v;
                imageDim[2] = dimName;
            }
        }

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {

                int index = y * width + x;
                imageRGBData[index * 3] = source->getValueAt(index * numDimensions + dimR);
                imageRGBData[index * 3 + 1] = source->getValueAt(index * numDimensions + dimG);
                imageRGBData[index * 3 + 2] = source->getValueAt(index * numDimensions + dimB);

            }
        }

        _imageRGBPoints->setData(imageRGBData.data(), numPoints, 3);
        _imageRGBPoints->setDimensionNames(imageDim);

        _core->notifyDatasetAdded(_imageRGBPoints);

        _imageRGB->setGuiName("RGB Image");
        _imageRGB->setType(ImageData::Type::Stack);
        _imageRGB->setNumberOfImages(1);
        _imageRGB->setImageSize(QSize(width, height));
        _imageRGB->setNumberOfComponentsPerPixel(3);

        _core->notifyDatasetAdded(_imageRGB);
    }
}

// improve: change one wavelength at a time
void LineplotPlugin::changeRGBWavelengths(const float wavelengthR, const float wavelengthG, const float wavelengthB) {

    auto source = _points->getSourceDataset<Points>();
    auto dimNames = _points->getDimensionNames();
    auto numDimensions = _points->getNumDimensions();
    
    auto children = source->getChildren({ ImageType });
    auto imagesId = children[0].getDatasetGuid();
    auto images = _core->requestDataset<Images>(imagesId);
    auto imageSize = images->getImageSize();
    int width = imageSize.width();
    int height = imageSize.height();

    int numPoints = width * height;

    std::vector<float> imageRGBData(width * height * 3);
    std::vector<QString> imageDim(3);

    int dimR = 0;
    int dimG = 0;
    int dimB = 0;

    for (int v = 0; v < numDimensions; v++) {
        auto dimName = dimNames.at(v);
        float dimValue = dimName.toFloat();
        if (abs(wavelengthR - dimValue) < 1) {
            dimR = v;
            imageDim[0] = dimName;
        }
        if (abs(wavelengthG - dimValue) < 1) {
            dimG = v;
            imageDim[1] = dimName;
        }
        if (abs(wavelengthB - dimValue) < 1) {
            dimB = v;
            imageDim[2] = dimName;
        }
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            int index = y * width + x;

            imageRGBData[index * 3] = source->getValueAt(index * numDimensions + dimR);
            imageRGBData[index * 3 + 1] = source->getValueAt(index * numDimensions + dimG);
            imageRGBData[index * 3 + 2] = source->getValueAt(index * numDimensions + dimB);
        }
    }

    _imageRGBPoints->setData(imageRGBData.data(), numPoints, 3);
    _imageRGBPoints->setDimensionNames(imageDim);
    _core->notifyDatasetChanged(_imageRGBPoints);
    _core->notifyDatasetChanged(_imageRGB);
  
}

void LineplotPlugin::updateSelection(Dataset<Points> selection) {

    if (selection.isValid()) {
       
        auto selectedIndices = selection->indices;
        auto noSelectedPoints = selection->getSelectionSize();

        if (noSelectedPoints > 0) {
            computeAverageSpectrum(_points, noSelectedPoints, selectedIndices, "selection");
        }
        
    }
}

std::tuple<std::vector<float>, std::vector<float>> LineplotPlugin::computeAverageSpectrum(Dataset<Points> source, int noPoints, std::vector<unsigned int> indices, std::string dataOrigin) {

    auto numDimensions = source->getNumDimensions();
    auto children = source->getChildren({ ImageType });
    auto imagesId = children[0].getDatasetGuid();
    auto images = _core->requestDataset<Images>(imagesId);
    auto imageSize = images->getImageSize();
    int width = imageSize.width();
    int height = imageSize.height();  

    std::vector<float> averageSpectrum(numDimensions);
    std::vector<float> standardDeviation(numDimensions);
    std::vector<float> confIntervalLeft(numDimensions);
    std::vector <float> confIntervalRight(numDimensions);

    for (int v = 0; v < numDimensions; v++) {
        float sum = 0;

        for (int i = 0; i < noPoints; i++) {
            
            auto index = indices.at(i);
            sum += source->getValueAt(index * numDimensions + v);
        }

        float mean = noPoints == 0 ? 0 : sum / noPoints;
        averageSpectrum[v] = mean;

        if (noPoints > 1) {
            // compute standard deviation per dimension                
            float std = 0;

            for (int i = 0; i < noPoints; i++) {
                auto index = indices.at(i);
                float value = source->getValueAt(index * numDimensions + v);
                std += (value - mean) * (value - mean);
            }

            std = sqrt(std / noPoints);

            standardDeviation[v] = std;
            confIntervalRight[v] = mean + std;
            confIntervalLeft[v] = mean - std;
        }
    }

    std::vector<QString> names;
    if (source->getDimensionNames().size() == source->getNumDimensions()) {
        names = source->getDimensionNames();
    }

    _linePlotWidget.setData(averageSpectrum, confIntervalLeft, confIntervalRight, names, numDimensions, dataOrigin);

    return { averageSpectrum, standardDeviation };
}

QString LineplotPlugin::getDatasetName() {
    if (_points.isValid()) {
        return _points->getGuiName();
    }
    else
        return "";
}

void LineplotPlugin::updateMap(std::vector<float> endmemberData, float thresholdAngle, int mapType, int algorithmType) {

    if (!_points.isValid()) {
        return;
    }

    if (!_map.isValid()) {

        qDebug() << "Setting map." << endmemberData.size();

        auto children = _points->getChildren({ ImageType });
        auto imagesId = children[0].getDatasetGuid();
        auto images = _core->requestDataset<Images>(imagesId);
        auto imageSize = images->getImageSize();
        int width = imageSize.width();
        int height = imageSize.height();

        _map = _core->createDerivedDataset("endmemberMapPoints", _points);

        spectralMapper(endmemberData, thresholdAngle, mapType, algorithmType);

        _mapImage = _core->addDataset<Images>("Images", "images", hdps::Dataset<hdps::DatasetImpl>(*_map));
        _mapImage->setGuiName("endmemberMap");
        _mapImage->setType(ImageData::Type::Stack);
        _mapImage->setNumberOfImages(1);
        _mapImage->setImageSize(QSize(width, height));
        _mapImage->setNumberOfComponentsPerPixel(1);

        _core->notifyDatasetAdded(_map);
        _core->notifyDatasetAdded(_mapImage);
    }
    else {

       spectralMapper(endmemberData, thresholdAngle, mapType, algorithmType);

        _core->notifyDatasetChanged(_map);
        _core->notifyDatasetChanged(_mapImage);
    }
}

// implementation of spectral angle mapper and spectral correlation mapper
void LineplotPlugin::spectralMapper(std::vector<float> endmemberData, float thresholdAngle, int mapType, int algorithmType) {
    
    auto children = _points->getChildren({ ImageType });
    auto imagesId = children[0].getDatasetGuid();
    auto images = _core->requestDataset<Images>(imagesId);
    auto imageSize = images->getImageSize();
    int width = imageSize.width();
    int height = imageSize.height();

    auto numDimensions = _points->getNumDimensions();
    auto noPoints = _points->getNumPoints();

    std::vector<float> mapData(width * height);

    float referenceSum = 0;
    float referenceMean = 0;

    if (algorithmType == 0) {

        for (int v = 0; v < numDimensions; v++) {
            referenceSum += pow(endmemberData[v], 2);
        }
    }
    else if (algorithmType == 1) {
        referenceMean = computeAverageValue(endmemberData);

        for (int v = 0; v < numDimensions; v++) {
            referenceSum += pow(endmemberData[v] - referenceMean, 2);
        }
    }

    referenceSum = sqrt(referenceSum);
    float angle;

    for (int y = 0; y < height; y++) {
       for (int x = 0; x < width; x++) {
            float sum = 0;
            float pointSum = 0;
            int index = y * width + x;

            if (algorithmType == 0) {
                for (int v = 0; v < numDimensions; v++) {
                    auto pointValue = _points->getValueAt(index * numDimensions + v);

                    sum += endmemberData[v] * pointValue;
                    pointSum += pow(pointValue, 2);
                }
            }
            else if (algorithmType == 1) {

                std::vector<float> pointData(noPoints);

                for (int v = 0; v < numDimensions; v++) {
                    pointData[v] = _points->getValueAt(index * numDimensions + v);
                }
                float pointMean = computeAverageValue(pointData);
                qDebug() << pointMean;

                for (int v = 0; v < numDimensions; v++) {
                    auto pointValue = _points->getValueAt(index * numDimensions + v);

                    sum += (endmemberData[v] - referenceMean) * (pointValue - pointMean);
                    pointSum += pow(pointValue - pointMean, 2);
                }
            } 

            pointSum = sqrt(pointSum);
           
            if (pointSum == 0 || referenceSum == 0) {
                angle = thresholdAngle + 0.2;
            }
            else {
                float value = sum / (referenceSum * pointSum);

                if (value > 1)
                    angle = 0;
                else if (value < 0)
                    angle = M_PI/2;
                else
                    angle = acos(value);
            }

            if (mapType == 0) {
                if (angle <= thresholdAngle) {
                    mapData[index] = 1;                    
                }
                else {
                    mapData[index] = 0;
                }
            }
            else if (mapType == 1) {
                if (pointSum != 0) {
                    mapData[index] = 1 - 2 * angle / M_PI;
                }
                else {
                    mapData[index] = 0;
                }
            }
        }
    }

    _map->setData(mapData.data(), noPoints, 1);
}

float LineplotPlugin::computeAverageValue(std::vector<float> data) {

    int dataLen = data.size();
    float avg = 0;

    for (int v = 0; v < dataLen; v++) {
        avg += data.at(v);
    }

    avg = avg / dataLen;

    return avg;
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
