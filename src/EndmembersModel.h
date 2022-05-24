#pragma once

#include "event/EventListener.h"
#include "Endmember.h"

#include <QAbstractListModel>

class EndmembersModel : public QAbstractListModel, public hdps::EventListener
{
public:

    /**  Columns */
    enum Column {
        Visible,        /** Visibility of the layer */
        Color,          /** Color of the layer */
        Name,           /** Name of the layer */
        //ImageWidth,     /** Width of the image(s) */
        //ImageHeight,    /** Height of the image(s) */
        //Scale,          /** Layer scale */
        //Opacity,        /** Layer opacity */

        Last = Name,
        Count = Last + 1
    };

public:

    /**
     * Constructor
     * @param parent Pointer to parent object
     */
    explicit EndmembersModel(QObject* parent);

    /** Destructor */
    ~EndmembersModel();

    /**
     * Returns the number of rows in the model given the parent model index
     * @param parent Parent model index
     * @return Number of rows in the model given the parent model index
     */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * Returns the number of columns in the model given the parent model index
     * @param parent Parent model index
     * @return Number of columns in the model given the parent model index
     */
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * Returns the model index for the given row, column and parent model index
     * @param row Row
     * @param column Column
     * @param parent Parent model index
     * @return Model index
     */
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    /**
     * Returns the data for the given model index and data role
     * @param index Model index
     * @param role Data role
     * @return Data in variant form
     */
    QVariant data(const QModelIndex& index, int role) const override;

    /**
     * Sets the data value for the given model index and data role
     * @param index Model index
     * @param value Data value in variant form
     * @param role Data role
     * @return Whether the data was properly set or not
     */
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    /**
     * Get header data
     * @param section Section
     * @param orientation Orientation
     * @param role Data role
     * @return Header
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /**
     * Get item flags
     * @param index Model index
     * @return Item flags
     */
    Qt::ItemFlags flags(const QModelIndex& index) const override;

public: // Endmember operations

    /**
     * Add an endmember to the model
     * @param endmember Pointer to endmember
     * @param decision index for colour/name selection
     */
    void addEndmember(Endmember* endmember, int decisionIndex);

    /**
    * Save all endmembers to a .txt file
    */
    void saveEndmembers(QString name);
    void saveEndmemberClusterVisibility(QString datasetGuid);
    void updateEndmemberClusterVisibility(QString datasetGuid);

    void updateEndmember(QString datasetGuid, int index, QString name, QColor color);
    /**
     * Remove an endmember from the model by row index
     * @param row Row index of the layer
     */
    void removeEndmember(const std::uint32_t& row);

    /**
     * Remove endmembers created from given dataset from the model
     * @param datasetGuid
     */
    void removeEndmembers(QString datasetGuid);

    /**
     * Remove a layer from the model by images dataset globally unique identifier
     * @param datasetId Globally unique identifier of the images dataset
     */
    void removeEndmember(const QString& datasetId);

    /**
     * Remove a layer from the model by model index of the layer to remove
     * @param layerModelIndex Layer model index
     */
    void removeEndmember(const QModelIndex& layerModelIndex);

    /**
     * Duplicates a layer  by model index
     * @param layerModelIndex Layer model index of the layer to duplicate
     */
  //  void duplicateLayer(const QModelIndex& layerModelIndex);

    /**
     * Move a layer in the model by amount
     * @param layerModelIndex Layer model index of the layer to move
     * @param amount Amount of layers to move up/down
     */
   // void moveLayer(const QModelIndex& layerModelIndex, const std::int32_t& amount = 1);

    /** Get the layers data */
    //QVector<Layer*>& getLayers();

    /**
     * Get pointer to layer by dataset globally unique identifier
     * @param datasetId Globally unique identifier of the images dataset
     * @param Reference to layer
     */
    //Layer& getLayerByDatasetId(const QString& datasetId);

//protected:

    /**
     * Get color icon for the layer
     * @param color Input color
     * @return Icon
     */
    QIcon getColorIcon(const QColor& color) const;

protected:
    QVector<Endmember*>                   _endmembers;        /** Endmembers data */
    std::unordered_map<QString, bool>       _clusterVisibility;
};