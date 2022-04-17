#include "EndmembersModel.h"

#include <Application.h>
#include <util/Exception.h>
#include <event/Event.h>
#include <PointData.h>
#include "ClusterData.h"
#include <ImageData/ImageData.h>
#include "LineplotPlugin.h"

#include <QMessageBox>
#include <QPainter>
#include <QFileDialog>

#include <stdexcept>
#include <iomanip>


using namespace hdps;

EndmembersModel::EndmembersModel(QObject* parent) :
    QAbstractListModel(parent),
    EventListener(),
    _endmembers()
{
    setEventCore(Application::core());

    // Register for events for points datasets
    registerDataEventByType(PointType, [this](DataEvent* dataEvent) {

        switch (dataEvent->getType())
        {
        case EventType::DataRemoved:
        {
            removeEndmember(dataEvent->getDataset()->getGuid());
            break;
        }

        default:
            break;
        }
        });

    // Register for events for images datasets
    registerDataEventByType(ImageType, [this](DataEvent* dataEvent) {

        switch (dataEvent->getType())
        {
        case EventType::DataRemoved:
        {
            removeEndmember(dataEvent->getDataset()->getGuid());
            break;
        }

        default:
            break;
        }
        });
}

EndmembersModel::~EndmembersModel()
{
    // Remove all endmembers
    for (std::int32_t row = 0; rowCount(); row++)
        removeEndmember(row);
}

void EndmembersModel::addEndmember(Endmember* endmember) {
    
    try
    {
        int noEndmembers = _endmembers.length();

        // Insert the endmember action at the beginning
        beginInsertRows(QModelIndex(), 0, 0);
        {
            // Insert the endmember at the end (endmember will be added after all other endmembers)
            _endmembers.insert(noEndmembers, endmember);

            // Inform views that the endmember visibility has changed when it is changed in the action
            connect(&endmember->getGeneralAction().getVisibleAction(), &ToggleAction::toggled, this, [this, endmember](bool toggled) {
                const auto changedCell = index(_endmembers.indexOf(endmember), Column::Name);
                emit dataChanged(changedCell, changedCell.siblingAtColumn(Column::Last));
                endmember->updateVisibility(toggled, _endmembers.indexOf(endmember));                
                });

            // Inform views that the endmember color has changed when it is changed in the action
            connect(&endmember->getGeneralAction().getColorAction(), &ColorAction::colorChanged, this, [this, endmember](const QColor& color) {
                const auto changedCell = index(_endmembers.indexOf(endmember), Column::Color);
                emit dataChanged(changedCell, changedCell);
                endmember->sendColor(color, _endmembers.indexOf(endmember));
                });

            // Inform views that the endmember name has changed when it is changed in the action
            connect(&endmember->getGeneralAction().getNameAction(), &StringAction::stringChanged, this, [this, endmember]() {
                const auto changedCell = index(_endmembers.indexOf(endmember), Column::Name);
                emit dataChanged(changedCell, changedCell);
                });

            connect(&endmember->getMapAction().getComputeAction(), &TriggerAction::triggered, this, [this, endmember]() {
                
                auto endmemberData = endmember->getData();
                auto angle = endmember->getMapAction().getAngleAction().getValue();
                qDebug() << angle;
                auto mapType = endmember->getMapAction().getMapTypeAction().getCurrentIndex();
                auto algorithm = endmember->getMapAction().getAlgorithmAction().getCurrentIndex();
                //auto automatic = endmember->getMapAction().getComputeAction().isChecked();

                endmember->updateAngle(endmemberData, angle, mapType, algorithm);
                });

            connect(&endmember->getMapAction().getMapTypeAction(), &OptionAction::currentIndexChanged, this, [this, endmember](int index) {
                
                if (index == 0) {
                    endmember->getMapAction().getAngleAction().setEnabled(true);
                }
                else if (index == 1) {
                    endmember->getMapAction().getAngleAction().setEnabled(false);
                }

                });

            // Inform views that the endmember angle has changed when it is changed in the action
            //connect(&endmember->getMapAction().getAngleAction(), &DecimalAction::valueChanged, this, [this, endmember](float value) {
            //    endmember->updateAngle(endmember->getData(), value);
            //    });
        }
        endInsertRows();   
    }
    catch (std::exception& e)
    {
        exceptionMessageBox("Unable to add endmember to the endmembers model", e);
    }
    catch (...) {
        exceptionMessageBox("Unable to add endmember to the endmembers model");
    }
}

void EndmembersModel::setEndmemberProperties(Endmember* endmember, int decisionIndex) {

    int noEndmembers = _endmembers.length() - 1;

    auto dataset = endmember->getDataset();
    auto type = dataset->getDataType();
    QColor endmemberColor;
    QVector<Cluster> clusters;

    if (type == PointType) {
        // set name with avg
        if (decisionIndex == 0) {
            auto currentName = endmember->getGeneralAction().getNameAction().getString();
            endmember->getGeneralAction().getNameAction().setString(currentName + " average");
        }

        endmemberColor = endmember->getGeneralAction().getColorAction().getColor();

    }
    else if (type == ClusterType) {
        clusters = dataset.get<Clusters>()->getClusters();
        auto noClusters = clusters.length();

        endmemberColor = clusters[decisionIndex].getColor();
        auto endmemberName = clusters[decisionIndex].getName();
        endmember->getGeneralAction().getColorAction().setColor(endmemberColor);
        endmember->getGeneralAction().getNameAction().setString(endmemberName);
    }

    endmember->sendColor(endmemberColor, noEndmembers);

}

void EndmembersModel::saveEndmembers(QString name) {

    auto parent = _endmembers[0]->getDataset()->getParent();
    auto points = parent.get<Points>();
    auto dimNames = points->getDimensionNames();
    auto noDim = points->getNumDimensions();

    std::vector<float> dimensions(noDim);

    for (int v = 0; v < noDim; v++) {
        dimensions[v] = dimNames[v].toFloat();
    }

    // Open dialog for saving location
    QString fileName = QFileDialog::getSaveFileName(nullptr, 
        tr("Save Selected Endmembers"), "",
        tr("Txt files (*.txt); "));

    if (fileName.isEmpty()) {
        return;
    }
    else {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(nullptr, tr("Unable to open file"),
                file.errorString());
            return;
        }

        QTextStream stream(&file);
        stream << "Endmembers for " << name << " dataset" << endl;
        stream << "Column 1: Wavelength" << endl;
        
        stream.setRealNumberNotation(QTextStream::FixedNotation);
        stream.setRealNumberPrecision(6);

        int noEndmembers = _endmembers.length();
        int columnNo = 2;
        int lastIndex = 0;

        for (int i = 0; i < noEndmembers; i++) {
            // set the right name
            auto visible = _endmembers[i]->getGeneralAction().getVisibleAction().isChecked();
            if (visible) {
                stream << "Column: " << columnNo << ": " << "name" << endl;
                columnNo++;
                lastIndex = i;
            }
        }

        for (int v = 0; v < noDim; v++) {

            stream << "  " << dimNames[v];

            for (int i = 0; i < lastIndex; i++) {
   
                auto visible = _endmembers[i]->getGeneralAction().getVisibleAction().isChecked();
                if (visible) {
                    // resample before saving
                    auto resampledEndmember = _endmembers[i]->resample(dimensions);

                    stream << "  " << _endmembers[i]->getData().at(v);
                    //stream << "  " << resampledEndmember.at(v);

                }
            }

            auto resampledEndmember = _endmembers[lastIndex]->resample(dimensions);

            stream << "  " << _endmembers[lastIndex]->getData().at(v) << endl;
            //stream << "  " << resampledEndmember.at(v) << endl;

        }
    }
}

int EndmembersModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    return _endmembers.count();
}

int EndmembersModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    return Column::Count;
}

QModelIndex EndmembersModel::index(int row, int column, const QModelIndex& parent /*= QModelIndex()*/) const
{
    return createIndex(row, column, static_cast<void*>(_endmembers.at(row)));
}


QVariant EndmembersModel::data(const QModelIndex& index, int role) const
{
    auto endmember = static_cast<Endmember*>(index.internalPointer());

    switch (role) {
    case Qt::DisplayRole:
    {
        switch (static_cast<Column>(index.column())) {
        case Column::Visible:
            break;

        case Column::Color:
            return "";

        case Column::Name:
            return data(index, Qt::EditRole).toString();

        default:
            break;
        }

        break;
    }

    case Qt::EditRole:
    {
        switch (static_cast<Column>(index.column())) {
        case Column::Visible:
            return endmember->getGeneralAction().getVisibleAction().isChecked();

        case Column::Color:
            return endmember->getGeneralAction().getColorAction().getColor();

        case Column::Name:
            return endmember->getGeneralAction().getNameAction().getString();

        default:
            break;
        }

        break;
    }

    case Qt::CheckStateRole:
    {
        switch (static_cast<Column>(index.column())) {
        case Column::Visible:
            return endmember->getGeneralAction().getVisibleAction().isChecked() ? Qt::Checked : Qt::Unchecked;

        case Column::Color:
        case Column::Name:
            break;

        default:
            break;
        }

        break;
    }

    case Qt::DecorationRole:
    {
        switch (static_cast<Column>(index.column())) {
        case Column::Visible:
            break;

            
        case Column::Color:
        {
            if (endmember->getGeneralAction().getVisibleAction().isChecked()) {
                return getColorIcon(endmember->getGeneralAction().getColorAction().getColor());
            }
            else {
                const auto color = endmember->getGeneralAction().getColorAction().getColor();
                return getColorIcon(QColor::fromHsl(color.hue(), 0, color.lightness()));
            }
        }
        
        case Column::Name:
            break;

        default:
            break;
        }

        break;
    }

    case Qt::ForegroundRole:
    {
        switch (static_cast<Column>(index.column())) {
        case Column::Visible:
        case Column::Color:
        case Column::Name:

        default:
            break;
        }
    }
    }

    return QVariant();
}


bool EndmembersModel::setData(const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/)
{
    auto endmember = static_cast<Endmember*>(index.internalPointer());

    switch (role) {
    case Qt::DisplayRole:
        break;

    case Qt::EditRole:
    {
        switch (static_cast<Column>(index.column())) {
        case Column::Visible:
            break;

        case Column::Color:
        {
            endmember->getGeneralAction().getColorAction().setColor(value.value<QColor>());
            break;
        }

        case Column::Name:
        {
            endmember->getGeneralAction().getNameAction().setString(value.toString());
            break;
        }

            break;

        default:
            break;
        }

        break;
    }

    case Qt::CheckStateRole:
    {
        switch (static_cast<Column>(index.column())) {
        case Column::Visible:
        {
            endmember->getGeneralAction().getVisibleAction().setChecked(value.toBool());
            break;
        }

        case Column::Color:
        case Column::Name:
            break;

        default:
            break;
        }

        break;
    }
    }

    emit dataChanged(index, index);

    return true;
}

QVariant EndmembersModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    const auto iconSize = QSize(14, 14);

    if (orientation == Qt::Horizontal) {
        switch (role)
        {
        case Qt::DisplayRole:
        {
            switch (static_cast<Column>(section))
            {
            case Column::Visible:
            case Column::Color:
                return "";

            case Column::Name:
                return "Name";

            default:
                break;
            }

            break;
        }

        case Qt::ToolTipRole:
        {
            switch (static_cast<Column>(section))
            {
            case Column::Visible:
                return "Whether the layer is visible or not";

            case Column::Color:
                return "Color of the layer";

            case Column::Name:
                return "Name of the layer";

            default:
                break;
            }

            break;
        }

        case Qt::DecorationRole:
        {
            switch (static_cast<Column>(section))
            {
            case Column::Visible:
                break;//return Application::getIconFont("FontAwesome").getIcon("eye", iconSize);

            case Column::Color:
                break;//return Application::getIconFont("FontAwesome").getIcon("palette", iconSize);

            case Column::Name:
                break;

            default:
                break;
            }

            break;
        }

        case Qt::TextAlignmentRole:
        {
            switch (static_cast<Column>(section)) {
            case Column::Visible:
                return Qt::AlignCenter;

            case Column::Color:
            case Column::Name:
                break;

            default:
                break;
            }

            break;
        }

        default:
            break;
        }
    }

    return QVariant();
}

void EndmembersModel::removeEndmember(const std::uint32_t& row)
{
    try
    {
        // Get pointer to layer which needs to be removed
        auto removeEndmember = _endmembers[row];
        removeEndmember->sendEndmemberRemoved(row);
        qDebug() << "Remove endmember:" << removeEndmember->getGeneralAction().getNameAction().getString();

        // Remove the row from the model
        beginRemoveRows(QModelIndex(), row, row);
        {
            // Remove the layer from the list
            _endmembers.removeAt(row);

            // Remove the layer physically
            delete removeEndmember;
        }
        endRemoveRows();
    }
    catch (std::exception& e)
    {
        exceptionMessageBox("Unable to remove layer from the layers model", e);
    }
    catch (...) {
        exceptionMessageBox("Unable to remove layer from the layers model");
    }
}

void EndmembersModel::removeEndmembers(QString datasetGuid) {
    
    int noEndmembers = _endmembers.length();

    Dataset<DatasetImpl> endmemberDataset;
    int noClusters = 0;
    int startIndex = 0;

    if (noEndmembers > 0) {
        endmemberDataset = _endmembers[0]->getDataset();
    }

    for (int i = 0; i < noEndmembers; i++) {
        
        if (endmemberDataset->getGuid() == datasetGuid) {
            startIndex = i;
            break;
        }
    }

    for (int i = 0; i < noEndmembers; i++) {

        if (endmemberDataset->getGuid() == datasetGuid) {
            noClusters++;
        }
    }

    for (int i = 0; i < noClusters; i++) {
        removeEndmember(startIndex);
    }
}

void EndmembersModel::removeEndmember(const QModelIndex& endmemberModelIndex)
{
    removeEndmember(endmemberModelIndex.row());
}

void EndmembersModel::removeEndmember(const QString& datasetId)
{
    try
    {
        if (datasetId.isEmpty())
            throw std::runtime_error("Dataset UID is empty");

        for (const auto& endmember : _endmembers) {
            //if (endmember->getImagesDatasetId() == datasetId)
                removeEndmember(_endmembers.indexOf(endmember));
        }
    }
    catch (std::exception& e)
    {
        exceptionMessageBox("Unable to remove layer from the layers model", e);
    }
    catch (...) {
        exceptionMessageBox("Unable to remove layer from the layers model");
    }
}

Qt::ItemFlags EndmembersModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto itemFlags = Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | QAbstractItemModel::flags(index);

    auto layer = static_cast<Endmember*>(index.internalPointer());

    if (index.column() == Column::Visible) {
        itemFlags |= Qt::ItemIsUserCheckable;
    }
    else {
        if (index.column() == Column::Name)
            itemFlags |= Qt::ItemIsEditable;
    }

    return itemFlags;
}

QIcon EndmembersModel::getColorIcon(const QColor& color) const
{
    QPixmap pixmap(QSize(13, 13));

    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);

    const auto radius = 3;

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(color));
    painter.drawRoundedRect(0, 0, pixmap.width(), pixmap.height(), radius, radius);

    return QIcon(pixmap);
}