// Code based on HeatMapPlugin
#pragma once

#include "LineplotPlugin.h"
#include "EndmembersCheckDialog.h"

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
    _angleMap(),
    _mapAngleImage(),
    _corMap(),
    _mapCorImage(),
    _angleDataset(),
    _corDataset(),
    _averageDataset()
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

                                    auto noEndmembers = candidateDataset.get<Points>()->getNumPoints();

                                    if (noEndmembers > 15) {
                                        EndmembersCheckDialog endmembersCheckDialog(nullptr, noEndmembers);

                                        connect(&endmembersCheckDialog, &EndmembersCheckDialog::closeDialog, this, [this, candidateDataset]() {
                                            addDataset(candidateDataset);
                                            });

                                        endmembersCheckDialog.exec();
                                                   
                                    }
                                    else
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

    connect(&_linePlotWidget, SIGNAL(changeRGBWavelengths(float, int)), SLOT(changeRGBWavelengths(float, int)));

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

    connect(&_linePlotWidget, &LineplotWidget::customContextMenuRequested, this, [this](const QPoint& point) {

        // Only show a context menu when there is data loaded
        if (_model.rowCount() <= 0)
            return;

        // Show the context menu
        _settingsAction.getContextMenu()->exec(getWidget().mapToGlobal(point));
        });
}

void LineplotPlugin::loadData(const Datasets& datasets) {

    // Only load data if we at least have one set
    if (datasets.isEmpty())
        return;
    
    if (datasets[0]->getDataType() == PointType && !_points.isValid())
        _points = datasets[0];
}

void LineplotPlugin::onDataEvent(hdps::DataEvent* dataEvent)
{
    // Get smart pointer to dataset that changed
    const auto& changedDataSet = dataEvent->getDataset();
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

            if(_points.isValid() && _mainToolbarAction.getGlobalSettingsAction().getShowSelectionAction().isChecked())
                updateSelection(selectionSet);

            break;
        }

        default:
            break;
    }
}

void LineplotPlugin::addDataset(const Dataset<DatasetImpl>& dataset) {
    
    auto type = dataset->getDataType();
    auto parent = _points;
    
    if (type == PointType) {

        auto numDimensions = parent.get<Points>()->getNumDimensions();

        auto points = dataset.get<Points>();
        auto noPoints = points->getNumPoints();
        auto indices = points->indices;

        if (indices.size() != 0) {
            for (int i = 0; i < noPoints; i++) {

                auto endmember = new Endmember(*this, dataset, -1);
                _model.addEndmember(endmember, -1);

                auto endmemberData = computeAverageSpectrum(parent, 1, { indices[i] }, "endmember");
                endmember->setData(std::get<0>(endmemberData));
                //endmember->setIndices(indices);
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

                auto endmember = new Endmember(*this, dataset, -1);
                _model.addEndmember(endmember, -1);

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
        auto numDimensions = parent->getNumDimensions();
        
        // remove clusters of this dataset
        _model.removeEndmembers(dataset->getGuid());
        
        // add clsuters
        for (int i = 0; i < noClusters; i++) {

            auto noPointsCluster = clusters[i].getNumberOfIndices();
            auto indices = clusters[i].getIndices();

            auto average = clusters[i].getMean();
            auto std = clusters[i].getStandardDeviation();

            auto endmember = new Endmember(*this, dataset, i);
            _model.addEndmember(endmember, i);

            if (average.size() == 0) {

                average.resize(numDimensions);
                std.resize(numDimensions);

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

                for (int v = 0; v < numDimensions; v++) {
                    confIntervalLeft[v] = average[v] - std[v];
                    confIntervalRight[v] = average[v] + std[v];
                }
                _linePlotWidget.setData(average, confIntervalLeft, confIntervalRight, dimNames, numDimensions, "endmember");
            }

            endmember->setData(average);
            //endmember->setIndices(indices);
        }
    }
}

void LineplotPlugin::addAverageDataset(const Dataset<DatasetImpl>& dataset) {

    auto type = dataset->getDataType();
    auto parent = _points;

    if (type == PointType) {
        auto points = dataset.get<Points>();
        auto noPoints = points->getNumPoints();
        auto indices = points->indices;

        if (indices.size() != 0) {

            auto endmember = new Endmember(*this, dataset, 0);
            _model.addEndmember(endmember, 0);

            auto endmemberData = computeAverageSpectrum(parent, noPoints, indices, "endmember");
            endmember->setData(std::get<0>(endmemberData));
        }
    }
}

void LineplotPlugin::initializeImageRGB() {

    QStringList dimensionNames;
    auto numDimensions = _points->getNumDimensions();
    auto dimNames = _points->getDimensionNames();

    // Populate dimension names
    if (_points->getDimensionNames().size() == _points->getNumDimensions()) {
        for (const auto& dimensionName : _points->getDimensionNames())
            dimensionNames << dimensionName;
    }
    else {
        for (std::uint32_t dimensionIndex = 0; dimensionIndex < _points->getNumDimensions(); dimensionIndex++)
            dimensionNames << QString("Dim %1").arg(QString::number(dimensionIndex));
    }

    _mainToolbarAction.getGlobalSettingsAction().getRedWavelengthAction().setOptions(dimensionNames);
    _mainToolbarAction.getGlobalSettingsAction().getGreenWavelengthAction().setOptions(dimensionNames);
    _mainToolbarAction.getGlobalSettingsAction().getBlueWavelengthAction().setOptions(dimensionNames);

    float wavelengthR = 630;
    float wavelengthG = 532;
    float wavelengthB = 464;

    QString dimR;
    QString dimG;
    QString dimB;
    float step = dimNames.at(1).toFloat() - dimNames.at(0).toFloat();

    for (int v = 0; v < numDimensions; v++) {
        auto dimName = dimNames.at(v);
        float dimValue = dimName.toFloat();

        if (abs(wavelengthR - dimValue) < step) {
            dimR = dimName;
        }
        else if (abs(wavelengthG - dimValue) < step) {
            dimG = dimName;
        }
        else if (abs(wavelengthB - dimValue) < step) {
            dimB = dimName;
        }
    }

    _mainToolbarAction.getGlobalSettingsAction().getRedWavelengthAction().setCurrentText(dimR);
    _mainToolbarAction.getGlobalSettingsAction().getGreenWavelengthAction().setCurrentText(dimG);
    _mainToolbarAction.getGlobalSettingsAction().getBlueWavelengthAction().setCurrentText(dimB);

    _mainToolbarAction.getGlobalSettingsAction().getRedWavelengthAction().setDefaultText(dimR);
    _mainToolbarAction.getGlobalSettingsAction().getGreenWavelengthAction().setDefaultText(dimG);
    _mainToolbarAction.getGlobalSettingsAction().getBlueWavelengthAction().setDefaultText(dimB);
}

void LineplotPlugin::changeRGBWavelengths(const float wavelength, int index) {

    QString newValue = QString::number(wavelength);

    if (index == 0) {
        _mainToolbarAction.getGlobalSettingsAction().getRedWavelengthAction().setCurrentText(newValue);
    }
    else if (index == 1) {
        _mainToolbarAction.getGlobalSettingsAction().getGreenWavelengthAction().setCurrentText(newValue);
    }
    else if (index == 2) {
        _mainToolbarAction.getGlobalSettingsAction().getBlueWavelengthAction().setCurrentText(newValue);
    }

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

/*
void LineplotPlugin::setSelection(std::vector<unsigned int> indices) {
    
    if (_points.isValid()) {
        _points->setSelectionIndices(indices);
        Application::core()->notifyDatasetSelectionChanged(_points);
    }
}
*/

std::tuple<std::vector<float>, std::vector<float>> LineplotPlugin::computeAverageSpectrum(Dataset<Points> source, int noPoints, std::vector<unsigned int> indices, std::string dataOrigin) {

    auto numDimensions = source->getNumDimensions();
    auto children = source->getChildren({ ImageType });

    std::vector<float> averageSpectrum(numDimensions);
    std::vector<float> standardDeviation(numDimensions);

    if (children.size() != 0) {
        auto imagesId = children[0].getDatasetGuid();
        auto images = _core->requestDataset<Images>(imagesId);
        auto imageSize = images->getImageSize();
        int width = imageSize.width();
        int height = imageSize.height();

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
    }

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

    if ( (algorithmType == 0 && !_angleMap.isValid()) || (algorithmType == 1 && !_corMap.isValid()) ) {

        auto children = _points->getChildren({ ImageType });
        auto imagesId = children[0].getDatasetGuid();
        auto images = _core->requestDataset<Images>(imagesId);
        auto imageSize = images->getImageSize();
        int width = imageSize.width();
        int height = imageSize.height();
        auto numDimensions = _points->getNumDimensions();


        if (algorithmType == 0) {

            _angleMap = _core->createDerivedDataset("endmemberAngleMapPoints", _points);

            spectralAngleMapper(endmemberData, thresholdAngle, mapType);

            _mapAngleImage = _core->addDataset<Images>("Images", "images", hdps::Dataset<hdps::DatasetImpl>(*_angleMap));
            _mapAngleImage->setGuiName("endmemberAngleMap");
            _mapAngleImage->setType(ImageData::Type::Stack);
            _mapAngleImage->setNumberOfImages(1);
            _mapAngleImage->setImageSize(QSize(width, height));
            _mapAngleImage->setNumberOfComponentsPerPixel(1);

            _core->notifyDatasetAdded(_angleMap);
            _core->notifyDatasetAdded(_mapAngleImage);

        }
        else {

            _corMap = _core->createDerivedDataset("endmemberCorMapPoints", _points);

            spectralCorrelationMapper(endmemberData, thresholdAngle, mapType);

            _mapCorImage = _core->addDataset<Images>("Images", "images", hdps::Dataset<hdps::DatasetImpl>(*_corMap));
            _mapCorImage->setGuiName("endmemberCorMap");
            _mapCorImage->setType(ImageData::Type::Stack);
            _mapCorImage->setNumberOfImages(1);
            _mapCorImage->setImageSize(QSize(width, height));
            _mapCorImage->setNumberOfComponentsPerPixel(1);

            _core->notifyDatasetAdded(_corMap);
            _core->notifyDatasetAdded(_mapCorImage);
        }
    }
    else {

        if (algorithmType == 0) {
            spectralAngleMapper(endmemberData, thresholdAngle, mapType);
            _core->notifyDatasetChanged(_angleMap);
            _core->notifyDatasetChanged(_mapAngleImage);
        }
        else {
            spectralCorrelationMapper(endmemberData, thresholdAngle, mapType);
            _core->notifyDatasetChanged(_corMap);
            _core->notifyDatasetChanged(_mapCorImage);
        }

       
    }
}

// implementation of spectral angle mapper and spectral correlation mapper
void LineplotPlugin::spectralAngleMapper(std::vector<float> endmemberData, float thresholdAngle, int mapType) {
    
    auto children = _points->getChildren({ ImageType });
    auto imagesId = children[0].getDatasetGuid();
    auto images = _core->requestDataset<Images>(imagesId);
    auto imageSize = images->getImageSize();
    int width = imageSize.width();
    int height = imageSize.height();

    _angleDataset.resize(width * height);

    auto numDimensions = _points->getNumDimensions();
    auto noPoints = _points->getNumPoints();

    std::vector<float> mapData(width * height);

    float referenceSum = 0;

    for (int v = 0; v < numDimensions; v++) {
        referenceSum += pow(endmemberData[v], 2);
    }

    referenceSum = sqrt(referenceSum);
    float angle;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float sum = 0;
            float pointSum = 0;
            int index = y * width + x;

            for (int v = 0; v < numDimensions; v++) {
                auto pointValue = _points->getValueAt(index * numDimensions + v);

                sum += endmemberData[v] * pointValue;
                pointSum += pow(pointValue, 2);
            }
        
            pointSum = sqrt(pointSum);
          
            if (pointSum == 0 || referenceSum == 0) {
                angle = 10;
            }
            else {
                float value = sum / (referenceSum * pointSum);

                if (value > 1)
                    angle = 0;
                else if (value < 0)
                    angle = M_PI / 2;
                else
                    angle = acos(value);
            }

            _angleDataset[index] = angle;

            if (mapType == 0) {
                if (angle <= thresholdAngle) {
                    mapData[index] = 1;                    
                }
                else {
                    mapData[index] = 0;
                }
            }
            else if (mapType == 1) {
                if (angle <= thresholdAngle) {
                    mapData[index] = 1 - 2 * angle / M_PI;
                }
                else {
                    mapData[index] = 0;
                }
            }
            else if (mapType == 2) {
                if (pointSum != 0) {
                    mapData[index] = 1 - 2 * angle / M_PI;
                }
                else {
                    mapData[index] = 0;
                }
            }
        }
    }

    _angleMap->setData(mapData.data(), noPoints, 1);
}

void LineplotPlugin::spectralCorrelationMapper(std::vector<float> endmemberData, float threshold, int mapType) {

    auto children = _points->getChildren({ ImageType });
    auto imagesId = children[0].getDatasetGuid();
    auto images = _core->requestDataset<Images>(imagesId);
    auto imageSize = images->getImageSize();
    int width = imageSize.width();
    int height = imageSize.height();

    _corDataset.resize(width * height);

    auto numDimensions = _points->getNumDimensions();
    auto noPoints = _points->getNumPoints();

    std::vector<float> mapData(width * height);

    float referenceSum = 0;
    float referenceMean = 0;

    computeAverageDataset(width, height, numDimensions);

    for (int v = 0; v < numDimensions; v++) {
        referenceMean += endmemberData[v];
    }

    referenceMean = referenceMean / numDimensions;

    for (int v = 0; v < numDimensions; v++) {
        referenceSum += pow(endmemberData[v] - referenceMean, 2);
    }
    
    referenceSum = sqrt(referenceSum);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float sum = 0;
            float pointSum = 0;
            int index = y * width + x;

            for (int v = 0; v < numDimensions; v++) {
                auto pointValue = _points->getValueAt(index * numDimensions + v);

                sum += (endmemberData[v] - referenceMean) * (pointValue - _averageDataset[index]);
                pointSum += pow(pointValue - _averageDataset[index], 2);
            }
            
            pointSum = sqrt(pointSum);

            float value = 0;

            if (pointSum == 0 || referenceSum == 0) {
                value = -3;
            }
            else {
                value = sum / (referenceSum * pointSum);

                if (value > 1)
                    value = 1;
                else if (value < -1)
                    value = -1;
            }
            
            _corDataset[index] = value;

            if (mapType == 0) {

                if (value >= threshold)
                    mapData[index] = 1;
                else
                    mapData[index] = 0;
            }
            else if (mapType == 1) {
                if (value >= threshold) {
                    mapData[index] = (value + 1) / 2;
                }
                else {
                    mapData[index] = 0;
                }
            }
            else if (mapType == 2) {
                if (pointSum != 0) {
                    mapData[index] = (value + 1) / 2;
                }
                else {
                    mapData[index] = 0;
                }
            }
        }
    }

    _corMap->setData(mapData.data(), noPoints, 1);
}

void LineplotPlugin::updateThresholdAngle(float threshold, int mapType, int algorithmType) {    

    if ( (algorithmType == 0 && _angleDataset.size() != 0) || (algorithmType == 1 && _corDataset.size() != 0) ) {

        auto children = _points->getChildren({ ImageType });
        auto imagesId = children[0].getDatasetGuid();
        auto images = _core->requestDataset<Images>(imagesId);
        auto imageSize = images->getImageSize();
        int width = imageSize.width();
        int height = imageSize.height();

        auto numDimensions = _points->getNumDimensions();
        auto noPoints = _points->getNumPoints();

        std::vector<float> mapData(width * height);
        float value = 0;

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {

                int index = y * width + x;

                if (algorithmType == 0) {

                    value = _angleDataset[index];

                    if (mapType == 0) {
                        if (value <= threshold) {
                            mapData[index] = 1;
                        }
                        else {
                            mapData[index] = 0;
                        }
                    }
                    else if (mapType == 1) {
                        if (value <= threshold) {
                            mapData[index] = 1 - 2 * value / M_PI;
                        }
                        else {
                            mapData[index] = 0;
                        }
                    }
                    else if (mapType == 2) {
                        if (value > M_PI/2)
                            mapData[index] = 0;
                        else
                            mapData[index] = 1 - 2 * value / M_PI;
                    }
                }
                else if (algorithmType == 1) {

                    value = _corDataset[index];

                    if (mapType == 0) {
                        if (value >= threshold) {
                            mapData[index] = 1;
                        }
                        else {
                            mapData[index] = 0;
                        }
                    }
                    else if (mapType == 1) {
                        if (value >= threshold) {
                            mapData[index] = (value + 1) / 2;
                        }
                        else {
                            mapData[index] = 0;
                        }
                    }
                    else if (mapType == 2) {

                        if (value < -1)
                            mapData[index] = 0;
                        else
                            mapData[index] = (value + 1) / 2; 
                    }
                }
            }
        }

        if (algorithmType == 0) {
            _angleMap->setData(mapData.data(), noPoints, 1);

            _core->notifyDatasetChanged(_angleMap);
            _core->notifyDatasetChanged(_mapAngleImage);
        }
        else if (algorithmType == 1) {
            _corMap->setData(mapData.data(), noPoints, 1);

            _core->notifyDatasetChanged(_corMap);
            _core->notifyDatasetChanged(_mapCorImage);
        }
    }
}

void LineplotPlugin::computeAverageDataset(int width, int height, int numDimensions) {

    _averageDataset.resize(width * height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            float avg = 0;
            int index = y * width + x;

            for (int v = 0; v < numDimensions; v++) {
                avg += _points->getValueAt(index * numDimensions + v);
            }

            _averageDataset[index] = avg / numDimensions;
        }
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
