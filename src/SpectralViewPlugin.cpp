#include "SpectralViewPlugin.h"

#include "EndmembersCheckDialog.h"

#include "ClusterData/ClusterData.h"
#include "event/Event.h"
#include "ImageData/Images.h"
#include "PointData/PointData.h"

#include "DataHierarchyItem.h"
#include "DatasetsMimeData.h"

#include <iostream>
#include <QtCore>
#include <QtDebug>

#include <tuple>
#define _USE_MATH_DEFINES
#include <cmath>

Q_PLUGIN_METADATA(IID "nl.tudelft.SpectralViewPlugin")

using namespace mv;
using namespace mv::gui;

SpectralViewPlugin::SpectralViewPlugin(const PluginFactory* factory) :
    ViewPlugin(factory),
    _model(this),
    _selectionModel(&_model),
    _splitter(Qt::Horizontal, &getWidget()),
    _linePlotWidget(),
    _dropWidget(&_linePlotWidget),
    _mainToolbarAction(this, "Main Toolbar"),
    _settingsAction(*this),
    _angleMap(),
    _mapAngleImage(),
    _corMap(),
    _mapCorImage(),
    _angleDataset(),
    _corDataset(),
    _averageDataset(),
    _noLoadedClusters(),
    _clusterNames(),
    _prevSelection(),
    _viewSettingsAction(*this),
    _wavelengthsRGBAction(*this)
{
    setObjectName("Spectral Plot");

    getWidget().setContextMenuPolicy(Qt::CustomContextMenu);
    getWidget().setFocusPolicy(Qt::ClickFocus);

    _mainToolbarAction.setEnabled(false);

    _mainToolbarAction.addAction(&_viewSettingsAction, 2, GroupAction::Horizontal);
    _mainToolbarAction.addAction(&_wavelengthsRGBAction, 1, GroupAction::Horizontal);

}

SpectralViewPlugin::~SpectralViewPlugin() {
}

void SpectralViewPlugin::init()
{
    _dropWidget.setDropIndicatorWidget(new DropWidget::DropIndicatorWidget(&getWidget(), "No data loaded", "Drag an item from the data hierarchy and drop it here to visualize data..."));

    _linePlotWidget.setPage(":/lineplot/lineplot.html", "qrc:/lineplot/");

    auto mainLayout = new QHBoxLayout();

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    getWidget().setLayout(mainLayout);

    // Create main left widget
    auto mainWidget = new QWidget();

    // Create main widget layout
    auto mainWidgetLayout = new QVBoxLayout();

    // Configure main layout
    mainWidgetLayout->setContentsMargins(0, 0, 0, 0);
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

        const auto datasetsMimeData = dynamic_cast<const DatasetsMimeData*>(mimeData);

        if (datasetsMimeData == nullptr)
            return dropRegions;

        if (datasetsMimeData->getDatasets().count() > 1)
            return dropRegions;

        const auto dataset = datasetsMimeData->getDatasets().first();
        const auto datasetGuiName = dataset->text();
        const auto datasetId = dataset->getId();
        const auto dataType = dataset->getDataType();
        const auto dataTypes = DataTypes({ PointType, ClusterType });

        // Check if the data type can be dropped
        if (!dataTypes.contains(dataType)) {
            dropRegions << new DropWidget::DropRegion(this, "Incompatible data", "This type of data is not supported", "exclamation-circle", false);
        }

        // Get points dataset from the core
        auto candidateDataset = mv::data().getDataset(datasetId);

        // Points dataset is about to be dropped
        if (dataType == PointType) {

            auto noPointsCandiateData = candidateDataset.get<Points>()->getNumPoints();

            // Establish drop region description
            const auto description = QString("Visualize %1 as line plot").arg(datasetGuiName);

            try {

                if (!_points.isValid()) {
                    
                    // Load as point positions when no dataset is currently loaded
                    dropRegions << new DropWidget::DropRegion(this, "Point position", description, "map-marker-alt", true, [this, candidateDataset]() {
                        loadData({ candidateDataset });
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

                            DataHierarchyItems parents = candidateDataset->getDataHierarchyItem().getParents();
                            QString pointName = _points->getGuiName();

                            if (!_points->isFull() || _points->isDerivedData()) {
                                DataHierarchyItems pointParents = _points->getDataHierarchyItem().getParents();;
                                pointName = pointParents.at(0)->getDatasetReference()->getGuiName();
                            }

                            auto parent = candidateDataset->getParent();
                            // Load as point positions when no dataset is currently loaded
                            dropRegions << new DropWidget::DropRegion(this, "Point position", description, "map-marker-alt", true, [this, candidateDataset]() {
                                _points = candidateDataset.get<Points>();

                                initializeImageRGB();
                                _mainToolbarAction.setEnabled(true);
                                _model.removeAllEndmembers();
                                });

                            if (parent.get<Points>()->getNumPoints() == _points->getNumPoints() && parents.at(0)->getDatasetReference()->getGuiName() == pointName) {

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
                                _model.removeAllEndmembers();
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
            const auto description = QString("Visualize every cluster in %1 as one line").arg(candidateDataset->getGuiName());
            bool isNewCluster = true;

            if (_points.isValid()) {

                QString pointsGuid = _points->getId();

                if (!_points->isFull() || _points->isDerivedData()) {
                    DataHierarchyItems pointParents = _points->getDataHierarchyItem().getParents();
                    pointsGuid = pointParents.at(0)->getDataset<Points>()->getId();
                }

                DataHierarchyItems parents = candidateDataset->getDataHierarchyItem().getParents();
                const auto parent = parents.at(0)->getDataset<Points>();

                if (parent->getNumPoints() == _points->getNumPoints() && parent->getId() == pointsGuid) {

                    dropRegions << new DropWidget::DropRegion(this, "Endmembers", description, "map-marker-alt", true, [this, candidateDataset]() {
                        _clusterNames.push_back(candidateDataset->getGuiName());
                        _noLoadedClusters.push_back(candidateDataset.get<Clusters>()->getClusters().size());
                        addDataset(candidateDataset);

                        });
                }
                else {
                    dropRegions << new DropWidget::DropRegion(this, "Warning", "Clusters have to be a child of and come from a set of same size as the loaded points", "exclamation-circle", false);
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

    connect(&_linePlotWidget, SIGNAL(changeRGBWavelengths(float, int)), SLOT(changeRGBWavelengths(float, int)));

    // Respond when the name of the dataset in the dataset reference changes
    connect(&_points, &Dataset<Points>::guiNameChanged, this, [this, updateWindowTitle]() {

        // Only show the drop indicator when nothing is loaded in the dataset reference
        _dropWidget.setShowDropIndicator(_points->getGuiName().isEmpty());
        updateWindowTitle();
        });

    //_eventListener.setEventCore(Application::core());
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetAdded));
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataChanged));
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataSelectionChanged));
    _eventListener.registerDataEventByType(PointType, std::bind(&SpectralViewPlugin::onDataEvent, this, std::placeholders::_1));
    _eventListener.registerDataEventByType(ClusterType, std::bind(&SpectralViewPlugin::onDataEvent, this, std::placeholders::_1));

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

void SpectralViewPlugin::loadData(const Datasets& datasets) {

    // Only load data if we at least have one set
    if (datasets.isEmpty())
        return;
    
    if (datasets[0]->getDataType() != PointType)
        return;

     _points = datasets[0];
    initializeImageRGB();
    _mainToolbarAction.setEnabled(true);
    _model.removeAllEndmembers();
}

void SpectralViewPlugin::onDataEvent(mv::DatasetEvent* dataEvent)
{
    if (!_linePlotWidget.isVisible())
        return;

    // Get smart pointer to dataset that changed
    const auto& changedDataSet = dataEvent->getDataset();
    auto type = changedDataSet->getDataType();
    
    // Get GUI name of the dataset that changed
    const auto datasetGuiName = changedDataSet->getGuiName();
    const auto datasetGuid = changedDataSet->getId();
 
    switch (dataEvent->getType()) {

        // Event which gets triggered when a dataset is added to the system.
        case EventType::DatasetAdded:
        {
            _linePlotWidget.addDataOption(dataEvent->getDataset()->getGuiName());

            // Cast the data event to a data added event
            const auto dataAddedEvent = static_cast<DatasetAddedEvent*>(dataEvent);

            // Get the GUI name of the added points dataset and print to the console
            std::cout << datasetGuiName.toStdString() << " was added" << std::endl;            

            break;
        }

        // Event which gets triggered when the data contained in a dataset changes.
        case EventType::DatasetDataChanged:
        {
            // Cast the data event to a data changed event
            const auto dataChangedEvent = static_cast<DatasetDataChangedEvent*>(dataEvent);

            // Get the name of the points dataset of which the data changed and print to the console
            std::cout << datasetGuiName.toStdString() << " data changed" << std::endl;

            if (type == ClusterType) {
               for (int i = 0; i < _clusterNames.size(); i++) {
                    if (_clusterNames[i] == datasetGuiName) {
                        auto noClusters = changedDataSet.get<Clusters>()->getClusters().size();

                        if (_noLoadedClusters[i] == noClusters)
                            updateDataset(changedDataSet);
                        else if (_noLoadedClusters[i] > noClusters) {
                            _noLoadedClusters[i] = noClusters;
                            addDataset(changedDataSet);
                        }
                        else if (_noLoadedClusters[i] < noClusters) {
                            _noLoadedClusters[i] = noClusters;
                            addNewCluster(changedDataSet);
                        }
                        break;
                    }                    
                }
            }

            break;
        }

        // Points dataset selection has changed
        case EventType::DatasetDataSelectionChanged:
        {
            // Cast the data event to a data selection changed event
            const auto dataSelectionChangedEvent = static_cast<DatasetDataSelectionChangedEvent*>(dataEvent);

            // Get the selection set that changed
            const auto& selectionSet = changedDataSet->getSelection<Points>();

            if(_points.isValid() && _viewSettingsAction.getShowSelectionAction().isChecked())
                updateSelection(selectionSet);

            break;
        }

        default:
            break;
    }
}

void SpectralViewPlugin::addDataset(const Dataset<DatasetImpl>& dataset) {
    
    auto type = dataset->getDataType();
    
    if (type == PointType) {

        auto points = dataset.get<Points>();
        auto numPoints = points->getNumPoints();
        auto& indices = points->indices;

        if (indices.size() != 0) {
            for (unsigned int i = 0; i < numPoints; i++) {

                auto endmember = new Endmember(*this, dataset, -1);
                _model.addEndmember(endmember, -1);

                const auto& endmemberData = computeAverageSpectrum(_points, 1u, { indices[i] }, "endmember");
                endmember->setData(std::get<0>(endmemberData));
                //endmember->setIndices(indices);
            }
        }
        else {
            auto numDimensions = points->getNumDimensions();
            const auto& dimNames = points->getDimensionNames();

            std::vector<float> endmemberData(numDimensions);
            std::vector<float> confInterval(numDimensions);

            for (size_t i = 0; i < numPoints; i++) {
                
                for (unsigned int v = 0; v < numDimensions; v++) {
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

        auto& clusters = dataset.get<Clusters>()->getClusters();
        auto noClusters = clusters.length();

        auto numDimensions = _points->getNumDimensions();

        _model.saveEndmemberClusterVisibility(dataset->getId());
        // remove clusters of this dataset
        _model.removeEndmembers(dataset->getId());
        
        // add clsuters
        for (int i = 0; i < noClusters; i++) {

            auto& cluster = clusters[i];

            auto noPointsCluster = cluster.getNumberOfIndices();
            auto& indices = cluster.getIndices();

            auto& average = cluster.getMean();
            auto& std = cluster.getStandardDeviation();

            auto endmember = new Endmember(*this, dataset, i);

            _model.addEndmember(endmember, i);

            average.resize(numDimensions);
            std.resize(numDimensions);

            const auto& endmemberData = computeAverageSpectrum(_points, noPointsCluster, indices, "endmember");
            const auto& computedAvg = std::get<0>(endmemberData);
            const auto& computedStd = std::get<1>(endmemberData);

            for (unsigned int v = 0; v < numDimensions; v++) {
                average[v] = computedAvg[v];
                std[v] = computedStd[v];
            }
           // }
           /*
            else {
                qDebug() << "precomputed ";
                std::vector<float> confIntervalLeft(numDimensions);
                std::vector<float> confIntervalRight(numDimensions);
                auto dimNames = parent.get<Points>()->getDimensionNames();

                for (int v = 0; v < numDimensions; v++) {
                    confIntervalLeft[v] = average[v] - std[v];
                    confIntervalRight[v] = average[v] + std[v];
                }
                _linePlotWidget.setData(average, confIntervalLeft, confIntervalRight, dimNames, numDimensions, "endmember");
            }
            */
            
            endmember->setData(average);
            //endmember->setIndices(indices);
        }
        _model.updateEndmemberClusterVisibility(dataset->getId());
    }
}

void SpectralViewPlugin::addNewCluster(const Dataset<DatasetImpl>& dataset) {
    
    auto& clusters = dataset.get<Clusters>()->getClusters();
    auto noClusters = clusters.length();
    auto& lastCluster = clusters[noClusters - 1];

    auto& parent = _points;
    auto numDimensions = parent->getNumDimensions();

    auto& average = lastCluster.getMean();
    auto& std = lastCluster.getStandardDeviation();

    auto noPointsCluster = lastCluster.getNumberOfIndices();
    auto& indices = lastCluster.getIndices();

    auto endmember = new Endmember(*this, dataset, noClusters-1);
    _model.addEndmember(endmember, noClusters-1);

    average.resize(numDimensions);
    std.resize(numDimensions);

    const auto& endmemberData = computeAverageSpectrum(parent, noPointsCluster, indices, "endmember");
    const auto& computedAvg = std::get<0>(endmemberData);
    const auto& computedStd = std::get<1>(endmemberData);

    for (unsigned int v = 0; v < numDimensions; v++) {
        average[v] = computedAvg[v];
        std[v] = computedStd[v];
    }

    endmember->setData(average);
}

void SpectralViewPlugin::updateDataset(const Dataset<DatasetImpl>& dataset) {

    auto& clusters = dataset.get<Clusters>()->getClusters();
    auto noClusters = clusters.length();

    auto& parent = _points;
    auto numDimensions = parent->getNumDimensions();

    for (int i = 0; i < noClusters; i++) {

        auto& cluster = clusters[i];
        _model.updateClustersEndmember(dataset->getId(), i, cluster.getName(), cluster.getColor());
    }
    
}

void SpectralViewPlugin::addAverageDataset(const Dataset<DatasetImpl>& dataset) {

    auto type = dataset->getDataType();
    auto& parent = _points;

    if (type == PointType) {
        auto points = dataset.get<Points>();
        auto noPoints = points->getNumPoints();
        auto& indices = points->indices;

        if (indices.size() != 0) {

            auto endmember = new Endmember(*this, dataset, 0);
            _model.addEndmember(endmember, 0);

            const auto& endmemberData = computeAverageSpectrum(parent, noPoints, indices, "endmember");
            endmember->setData(std::get<0>(endmemberData));
        }
    }
}

void SpectralViewPlugin::initializeImageRGB() {

    QStringList dimensionNames;
    auto numDimensions = _points->getNumDimensions();
    const auto& dimNames = _points->getDimensionNames();

    // Populate dimension names
    if (dimNames.size() == _points->getNumDimensions()) {
        for (const auto& dimensionName : _points->getDimensionNames())
            dimensionNames << dimensionName;
    }
    else {
        for (std::uint32_t dimensionIndex = 0; dimensionIndex < _points->getNumDimensions(); dimensionIndex++)
            dimensionNames << QString("Dim %1").arg(QString::number(dimensionIndex));
    }

    _wavelengthsRGBAction.getRedWavelengthAction().setOptions(dimensionNames);
    _wavelengthsRGBAction.getGreenWavelengthAction().setOptions(dimensionNames);
    _wavelengthsRGBAction.getBlueWavelengthAction().setOptions(dimensionNames);

    float wavelengthR = 630;
    float wavelengthG = 532;
    float wavelengthB = 464;

    QString dimR;
    QString dimG;
    QString dimB;
    float step = dimNames.at(1).toFloat() - dimNames.at(0).toFloat();

    for (unsigned int v = 0; v < numDimensions; v++) {
        const auto& dimName = dimNames.at(v);
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

    _wavelengthsRGBAction.getRedWavelengthAction().setCurrentText(dimR);
    _wavelengthsRGBAction.getGreenWavelengthAction().setCurrentText(dimG);
    _wavelengthsRGBAction.getBlueWavelengthAction().setCurrentText(dimB);

    //_wavelengthsRGBAction.getRedWavelengthAction().setDefaultText(dimR);
    //_wavelengthsRGBAction.getGreenWavelengthAction().setDefaultText(dimG);
    //_wavelengthsRGBAction.getBlueWavelengthAction().setDefaultText(dimB);
}

void SpectralViewPlugin::changeRGBWavelengths(const float wavelength, int index) {

    QString newValue = QString::number(wavelength);

    if (index == 0) {
        _wavelengthsRGBAction.getRedWavelengthAction().setCurrentText(newValue);
    }
    else if (index == 1) {
        _wavelengthsRGBAction.getGreenWavelengthAction().setCurrentText(newValue);
    }
    else if (index == 2) {
        _wavelengthsRGBAction.getBlueWavelengthAction().setCurrentText(newValue);
    }

}

void SpectralViewPlugin::updateSelection(Dataset<Points> selection) {

    if (selection.isValid()) {
       
        const auto& selectedIndices = selection->indices;

        if (selectedIndices == _prevSelection)
            return;

        _prevSelection = selectedIndices;
        auto noSelectedPoints = selection->getSelectionSize();

        if (noSelectedPoints > 0) {
            computeAverageSpectrum(_points, noSelectedPoints, selectedIndices, "selection");
        }
        
    }
}

/*
void SpectralViewPlugin::setSelection(std::vector<unsigned int> indices) {
    
    if (_points.isValid()) {
        _points->setSelectionIndices(indices);
        Application::core()->notifyDatasetSelectionChanged(_points);
    }
}
*/

std::tuple<std::vector<float>, std::vector<float>> SpectralViewPlugin::computeAverageSpectrum(const Dataset<DatasetImpl>& source, unsigned int noPoints, const std::vector<unsigned int>& indices, const std::string& dataOrigin) {

    auto points = source.get<Points>();
    
    auto numDimensions = points->getNumDimensions();
    const auto& children = source->getChildren({ ImageType });

    std::vector<float> averageSpectrum(numDimensions);
    std::vector<float> standardDeviation(numDimensions);

    if (children.size() != 0) {
        auto imagesId = children[0].getDatasetId();
        const auto& images = mv::data().getDataset<Images>(imagesId);
        auto imageSize = images->getImageSize();
        int width = imageSize.width();
        int height = imageSize.height();

        std::vector<float> confIntervalLeft(numDimensions);
        std::vector<float> confIntervalRight(numDimensions);

        points->visitData([this, points, numDimensions, noPoints, &indices, &averageSpectrum, &standardDeviation, &confIntervalRight, &confIntervalLeft](auto pointData) {
            for (unsigned int i = 0; i < noPoints; i++) {
                const auto index = indices.at(i);
                const auto& spectrum = pointData[index];

                for (unsigned int v = 0; v < numDimensions; v++) {
                    averageSpectrum[v] += spectrum[v];
                }
            }

            for (unsigned int v = 0; v < numDimensions; v++) {
                averageSpectrum[v] = noPoints == 0 ? 0 : averageSpectrum[v] / noPoints;
            }

            if (noPoints > 1) {
                for (unsigned int i = 0; i < noPoints; i++) {
                    const auto index = indices.at(i);
                    const auto& spectrum = pointData[index];

                    for (unsigned int v = 0; v < numDimensions; v++) {
                        const auto value = spectrum[v];
                        const auto mean = averageSpectrum[v];
                        standardDeviation[v] += (value - mean) * (value - mean);
                    }
                }

                for (unsigned int v = 0; v < numDimensions; v++) {
                    const auto std = sqrt(standardDeviation[v] / noPoints);
                    const auto mean = averageSpectrum[v];
                    standardDeviation[v] = std;
                    confIntervalRight[v] = mean + std;
                    confIntervalLeft[v] = mean - std;
                }
            }
        });

        std::vector<QString> names;
        if (points->getDimensionNames().size() == points->getNumDimensions()) {
            names = points->getDimensionNames();
        }

        for (auto& name : names)
        {
            if (name.startsWith("0"))
                name.remove(0, 1);
        }

        _linePlotWidget.setData(averageSpectrum, confIntervalLeft, confIntervalRight, names, numDimensions, dataOrigin);
    }

    return { averageSpectrum, standardDeviation };
}

QString SpectralViewPlugin::getDatasetName() {
    if (_points.isValid()) {
        return _points->getGuiName();
    }
    else
        return "";
}

void SpectralViewPlugin::updateMap(QString endmemberName, std::vector<float> endmemberData, float thresholdAngle, int mapType, int algorithmType) {

    if (!_points.isValid()) {
        return;
    }

    if ( (algorithmType == 0 && !_angleMap.isValid()) || (algorithmType == 1 && !_corMap.isValid()) ) {

        const auto& children = _points->getChildren({ ImageType });
        auto imagesId = children[0].getDatasetId();
        const auto& images = mv::data().getDataset<Images>(imagesId);
        auto imageSize = images->getImageSize();
        int width = imageSize.width();
        int height = imageSize.height();
        auto numDimensions = _points->getNumDimensions();


        if (algorithmType == 0) {

            _angleMap = mv::data().createDerivedDataset("endmemberAngleMapPoints", _points);
            
            spectralAngleMapper(endmemberName, endmemberData, thresholdAngle, mapType);

            _mapAngleImage = mv::data().createDataset<Images>("Images", "images", mv::Dataset<mv::DatasetImpl>(*_angleMap));
            _mapAngleImage->setText("endmemberAngleMap");
            _mapAngleImage->setType(ImageData::Type::Stack);
            _mapAngleImage->setNumberOfImages(1);
            _mapAngleImage->setImageSize(QSize(width, height));
            _mapAngleImage->setNumberOfComponentsPerPixel(1);

            events().notifyDatasetDataChanged(_mapAngleImage);

        }
        else {

            _corMap = mv::data().createDerivedDataset("endmemberCorMapPoints", _points);

            spectralCorrelationMapper(endmemberName, endmemberData, thresholdAngle, mapType);

            _mapCorImage = mv::data().createDataset<Images>("Images", "images", mv::Dataset<mv::DatasetImpl>(*_corMap));
            _mapCorImage->setText("endmemberCorMap");
            _mapCorImage->setType(ImageData::Type::Stack);
            _mapCorImage->setNumberOfImages(1);
            _mapCorImage->setImageSize(QSize(width, height));
            _mapCorImage->setNumberOfComponentsPerPixel(1);

            events().notifyDatasetDataChanged(_mapCorImage);
        }
    }
    else {

        if (algorithmType == 0) {
            spectralAngleMapper(endmemberName, endmemberData, thresholdAngle, mapType);

            _mapAngleImage->setNumberOfImages(_angleMap->getNumDimensions());
            events().notifyDatasetDataChanged(_angleMap);
            events().notifyDatasetDataChanged(_mapAngleImage);
        }
        else {
            spectralCorrelationMapper(endmemberName, endmemberData, thresholdAngle, mapType);

            _mapCorImage->setNumberOfImages(_corMap->getNumDimensions());
            events().notifyDatasetDataChanged(_corMap);
            events().notifyDatasetDataChanged(_mapCorImage);
        }

       
    }
}

// implementation of spectral angle mapper and spectral correlation mapper
void SpectralViewPlugin::spectralAngleMapper(QString endmemberName, std::vector<float> endmemberData, float thresholdAngle, int mapType) {
    
    const auto& children = _points->getChildren({ ImageType });
    auto imagesId = children[0].getDatasetId();
    const auto& images = mv::data().getDataset<Images>(imagesId);
    auto imageSize = images->getImageSize();
    int width = imageSize.width();
    int height = imageSize.height();

    int imgDim = 1;
    bool existingEndmember = false;
    int currentDim = 0;

    //qDebug() << "Width: " << width;
    //qDebug() << "Height: " << height;

    if (_mapAngleImage.isValid()) {
        auto imgDimNames = _angleMap->getDimensionNames();
        imgDim = _angleMap->getNumDimensions();

        for (int i = 0; i < imgDim; i++) {
            if (imgDimNames[i] == endmemberName) {
                currentDim = i;
                existingEndmember = true;
                break;
            }
        }
        if (!existingEndmember) {
            currentDim = imgDim;
            imgDim++;
            imgDimNames.push_back(endmemberName);
            _angleMap->setDimensionNames(imgDimNames);
        }
    }

    _angleDataset.resize(width * height * imgDim);

    auto numDimensions = _points->getNumDimensions();
    auto noPoints = _points->getNumPoints();

   _mapAngleData.resize(width * height * imgDim);

    float referenceSum = 0;

    for (unsigned int v = 0; v < numDimensions; v++) {
        referenceSum += pow(endmemberData[v], 2);
    }

    referenceSum = sqrt(referenceSum);
    float angle;

    /*
    std::vector<std::uint32_t> globalIndices;
    _points->getGlobalIndices(globalIndices);

    Dataset<Points> parent;

    if (!_points->isFull())
        parent = _points->getParent<Points>();
        */
    for (int y = height -1; y >= 0; y--) {
        for (int x = width - 1; x >= 0; x--) {
            float sum = 0;
            float pointSum = 0;
            int64_t index = static_cast<int64_t>(y) * width + x;

        //    if (_points->isFull()) {
                for (unsigned int v = 0; v < numDimensions; v++) {
                    auto pointValue = _points->getValueAt(index * numDimensions + v);
               //     if (v == 0 || v == 1)
               //         qDebug() << pointValue;
                    sum += endmemberData[v] * pointValue;
                    pointSum += pow(pointValue, 2);
                }
       //     }
       /*     else {
                int index2 = 0;
                for (int g = 0; g < globalIndices.size(); g++) {
                    if (globalIndices[g] == index) {
                        index2 = g;
                        break;
                    }
                }
                for (int v = 0; v < numDimensions; v++) {
                    auto pointValue = _points->getValueAt(index2 * numDimensions + v);
                    //if (v == 0 || v == 1)
                    //    qDebug() << pointValue;
                    sum += endmemberData[v] * pointValue;
                    pointSum += pow(pointValue, 2);
                }
            }
            */
            pointSum = sqrt(pointSum);

            if (pointSum == 0 || referenceSum == 0) {
                angle = 10;
            }
            else {
                float value = sum / (referenceSum * pointSum);

                if (value > 1)
                    angle = 0;
                else if (value < 0)
                    angle = static_cast<float>( M_PI / 2 );
                else
                    angle = acos(value);
            }

            //auto it1 = _angleDataset.begin() + index * imgDim + currentDim;
            _angleDataset[index * imgDim + currentDim] = angle;

            // auto it2 = _mapAngleData.begin() + index * imgDim + currentDim;

            if (mapType == 0) {
                if (angle <= thresholdAngle) {
                    //_mapAngleData.insert(it2, 1);
                    _mapAngleData[index * imgDim + currentDim] = 1;
                }
                else {
                    // _mapAngleData.insert(it2, 0);
                    _mapAngleData[index * imgDim + currentDim] = 0;
                }
            }
            else if (mapType == 1) {
                if (angle <= thresholdAngle) {
                    //_mapAngleData.insert(it2, 1 - 2 * angle / M_PI);
                    _mapAngleData[index * imgDim + currentDim] = 1 - 2 * angle / M_PI;
                }
                else {
                    //_mapAngleData.insert(it2, 0);
                    _mapAngleData[index * imgDim + currentDim] = 0;
                }
            }
            else if (mapType == 2) {
                if (pointSum != 0) {
                    //_mapAngleData.insert(it2, 1 - 2 * angle / M_PI);
                    _mapAngleData[index * imgDim + currentDim] = 1 - 2 * angle / M_PI;
                }
                else {
                    //_mapAngleData.insert(it2, 0);
                    _mapAngleData[index * imgDim + currentDim] = 0;
                }
            }

            if (imgDim >= 2 && !existingEndmember) {
                for (int oldIndex = imgDim - 2; oldIndex >= 0; oldIndex--) {
                    _mapAngleData[index * imgDim + oldIndex] = _mapAngleData[index * (imgDim - 1) + oldIndex];
                    _angleDataset[index * imgDim + oldIndex] = _angleDataset[index * (imgDim - 1) + oldIndex];
                }
            }
        }
    }

    _angleMap->setData(_mapAngleData.data(), noPoints, imgDim);
    events().notifyDatasetDataChanged(_angleMap);

    if (imgDim == 1) {
        _angleMap->setDimensionNames({ endmemberName });
        events().notifyDatasetDataDimensionsChanged(_angleMap);
    }
}

void SpectralViewPlugin::spectralCorrelationMapper(QString endmemberName, std::vector<float> endmemberData, float threshold, int mapType) {

    const auto& children = _points->getChildren({ ImageType });
    auto imagesId = children[0].getDatasetId();
    const auto& images = mv::data().getDataset<Images>(imagesId);
    auto imageSize = images->getImageSize();
    int width = imageSize.width();
    int height = imageSize.height();

    int imgDim = 1;
    bool existingEndmember = false;
    int currentDim = 0;

    if (_mapCorImage.isValid()) {
        auto imgDimNames = _corMap->getDimensionNames();
        imgDim = _corMap->getNumDimensions();

        for (int i = 0; i < imgDim; i++) {
            if (imgDimNames[i] == endmemberName) {
                currentDim = i;
                existingEndmember = true;
                break;
            }
        }
        if (!existingEndmember) {
            currentDim = imgDim;
            imgDim++;
            imgDimNames.push_back(endmemberName);
            _corMap->setDimensionNames(imgDimNames);
        }

    }

    _corDataset.resize(width * height * imgDim);

    auto numDimensions = _points->getNumDimensions();
    auto noPoints = _points->getNumPoints();

    _mapCorData.resize(width * height * imgDim);

    float referenceSum = 0;
    float referenceMean = 0;

    computeAverageDataset(width, height, numDimensions);

    for (unsigned int v = 0; v < numDimensions; v++) {
        referenceMean += endmemberData[v];
    }

    referenceMean = referenceMean / numDimensions;

    for (unsigned int v = 0; v < numDimensions; v++) {
        referenceSum += pow(endmemberData[v] - referenceMean, 2);
    }
    
    referenceSum = sqrt(referenceSum);

    for (int y = height - 1; y >= 0; y--) {
        for (int x = width - 1; x >= 0; x--) {
            float sum = 0;
            float pointSum = 0;
            int64_t index = static_cast<int64_t>(y) * width + x;

            for (unsigned int v = 0; v < numDimensions; v++) {
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
            
            _corDataset[index * imgDim + currentDim] = value;

            if (mapType == 0) {

                if (value >= threshold)
                    _mapCorData[index * imgDim + currentDim] = 1;
                else
                    _mapCorData[index * imgDim + currentDim] = 0;
            }
            else if (mapType == 1) {
                if (value >= threshold) {
                    _mapCorData[index * imgDim + currentDim] = (value + 1) / 2;
                }
                else {
                    _mapCorData[index * imgDim + currentDim] = 0;
                }
            }
            else if (mapType == 2) {
                if (pointSum != 0) {
                    _mapCorData[index * imgDim + currentDim] = (value + 1) / 2;
                }
                else {
                    _mapCorData[index * imgDim + currentDim] = 0;
                }
            }

            if (imgDim >= 2 && !existingEndmember) {
                for (int oldIndex = imgDim - 2; oldIndex >= 0; oldIndex--) {
                    _mapCorData[index * imgDim + oldIndex] = _mapCorData[index * (imgDim - 1) + oldIndex];
                    _corDataset[index * imgDim + oldIndex] = _corDataset[index * (imgDim - 1) + oldIndex];
                }
            }
        }
    }

    _corMap->setData(_mapCorData.data(), noPoints, imgDim);
    events().notifyDatasetDataChanged(_corMap);

    if (imgDim == 1) {
        _corMap->setDimensionNames({ endmemberName });
        events().notifyDatasetDataDimensionsChanged(_corMap);
    }

}

void SpectralViewPlugin::updateThresholdAngle(QString endmemberName, float threshold, int mapType, int algorithmType) {

    if ( (algorithmType == 0 && _angleDataset.size() != 0) || (algorithmType == 1 && _corDataset.size() != 0) ) {

        const auto& children = _points->getChildren({ ImageType });
        auto imagesId = children[0].getDatasetId();
        const auto& images = mv::data().getDataset<Images>(imagesId);
        auto imageSize = images->getImageSize();
        int width = imageSize.width();
        int height = imageSize.height();

        auto numDimensions = _points->getNumDimensions();
        auto noPoints = _points->getNumPoints();

        int imgDim = 0;
        int currentDim = 0;
        std::vector<QString> imgDimNames;

        if (algorithmType == 0) {
            imgDimNames = _angleMap->getDimensionNames();
            imgDim = _angleMap->getNumDimensions();
        }
        else if (algorithmType == 1) {
            imgDimNames = _corMap->getDimensionNames();
            imgDim = _corMap->getNumDimensions();           
        }

        for (int i = 0; i < imgDim; i++)
            if (imgDimNames[i] == endmemberName)
                currentDim = i;

        float value = 0;

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {

                int64_t index = static_cast<int64_t>(y) * width + x;

                if (algorithmType == 0) {

                    value = _angleDataset[index * imgDim + currentDim];

                    if (mapType == 0) {
                        if (value <= threshold) {
                            _mapAngleData[index * imgDim + currentDim] = 1;
                        }
                        else {
                            _mapAngleData[index * imgDim + currentDim] = 0;
                        }
                    }
                    else if (mapType == 1) {
                        if (value <= threshold) {
                            _mapAngleData[index * imgDim + currentDim] = 1 - 2 * value / M_PI;
                        }
                        else {
                            _mapAngleData[index * imgDim + currentDim] = 0;
                        }
                    }
                    else if (mapType == 2) {
                        if (value > M_PI/2)
                            _mapAngleData[index * imgDim + currentDim] = 0;
                        else
                            _mapAngleData[index * imgDim + currentDim] = 1 - 2 * value / M_PI;
                    }
                }
                else if (algorithmType == 1) {

                    value = _corDataset[index * imgDim + currentDim];

                    if (mapType == 0) {
                        if (value >= threshold) {
                            _mapCorData[index * imgDim + currentDim] = 1;
                        }
                        else {
                            _mapCorData[index * imgDim + currentDim] = 0;
                        }
                    }
                    else if (mapType == 1) {
                        if (value >= threshold) {
                            _mapCorData[index * imgDim + currentDim] = (value + 1) / 2;
                        }
                        else {
                            _mapCorData[index * imgDim + currentDim] = 0;
                        }
                    }
                    else if (mapType == 2) {

                        if (value < -1)
                            _mapCorData[index * imgDim + currentDim] = 0;
                        else
                            _mapCorData[index * imgDim + currentDim] = (value + 1) / 2;
                    }
                }
            }
        }

        if (algorithmType == 0) {
            _angleMap->setData(_mapAngleData.data(), noPoints, imgDim);

            _mapAngleImage->setNumberOfImages(imgDim);
            events().notifyDatasetDataChanged(_angleMap);
            events().notifyDatasetDataChanged(_mapAngleImage);
        }
        else if (algorithmType == 1) {
            _corMap->setData(_mapCorData.data(), noPoints, imgDim);

            _mapCorImage->setNumberOfImages(imgDim);
            events().notifyDatasetDataChanged(_corMap);
            events().notifyDatasetDataChanged(_mapCorImage);
        }
    }
}

void SpectralViewPlugin::computeAverageDataset(int width, int height, int numDimensions) {

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


// =============================================================================
// Factory
// =============================================================================

QIcon SpectralViewPluginFactory::getIcon() const
{
    return Application::getIconFont("FontAwesome").getIcon("chart-line");
}

ViewPlugin* SpectralViewPluginFactory::produce()
{
    return new SpectralViewPlugin(this);
}

mv::DataTypes SpectralViewPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    supportedTypes.append(ClusterType);
    return supportedTypes;
}

mv::gui::PluginTriggerActions SpectralViewPluginFactory::getPluginTriggerActions(const mv::Datasets& datasets) const
{
    PluginTriggerActions pluginTriggerActions;

    const auto getInstance = [this]() -> SpectralViewPlugin* {
        return dynamic_cast<SpectralViewPlugin*>(plugins().requestViewPlugin(getKind()));
    };

    const auto numberOfDatasets = datasets.count();

    if (PluginFactory::areAllDatasetsOfTheSameType(datasets, PointType)) {
        if (numberOfDatasets >= 1) {
            if (datasets.first()->getDataType() == PointType) {
                auto pluginTriggerAction = new PluginTriggerAction(const_cast<SpectralViewPluginFactory*>(this), this, "Spectral Viewer", "Load dataset in spectral Viewer", getIcon(), [this, getInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
                    for (const auto& dataset : datasets)
                        getInstance()->loadData(Datasets({ dataset }));
                    });

                pluginTriggerActions << pluginTriggerAction;
            }
        }
    }

    return pluginTriggerActions;
}