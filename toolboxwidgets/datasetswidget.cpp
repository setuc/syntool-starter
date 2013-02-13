#include "datasetswidget.h"

DatasetsWidget::DatasetsWidget(QWidget *parent) :
    QWidget(parent)
{
    vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->setAlignment(Qt::AlignTop);
    vLayout->setSpacing(3);

    setObjectName("DatasetsWidget");
    setStyleSheet(QString("QWidget#DatasetsWidget {background-color: "
                  "rgb(%1, %2, %3);}")
                  .arg(parent->palette().background().color().red())
                  .arg(parent->palette().background().color().green())
                  .arg(parent->palette().background().color().blue()));

    showAllCheck = new QCheckBox(this);
    showAllCheck->setText(tr("Show all"));

    vLayout->addWidget(showAllCheck);

    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
}

void DatasetsWidget::setGranules(QHash<QString, Granule> *granulesHash)
{
    _granulesHash = granulesHash;
}

void DatasetsWidget::addDatasets(QList<qint32> displayedGranules)
{
    emit closeAllDatasetBoxWidgets();
    for (int i = 0; i < displayedGranules.size(); ++i) {
        if (!currentDatasets.contains(displayedGranules.at(i)))
        {
            currentDatasets.append(displayedGranules.at(i));

            DatasetBoxWidget* datasetBox = new DatasetBoxWidget
                          (_granulesHash->value(QString::number(displayedGranules.at(i))), this);
            datasetBox->setChecked(showAllCheck->isChecked());

            connect(this, &DatasetsWidget::closeDatasetForGranuleId,
                    datasetBox, &DatasetBoxWidget::closeGranuleId);

            vLayout->addWidget(datasetBox);
            qDebug() << displayedGranules.at(i);
        }
    }

    for (int i = 0; i < currentDatasets.size(); ++i) {
        if (!displayedGranules.contains(currentDatasets.at(i)))
        {
            qDebug() << "-" << currentDatasets.at(i);
            emit closeDatasetForGranuleId(currentDatasets.at(i));
            currentDatasets.removeAt(i);
        }
    }
}
