#include "productswidget.h"
#include <QSizePolicy>

ProductsWidget::ProductsWidget(ConfigData *configData, QWidget *parent):
    QWidget()
{
    _configData = configData;
    productsHash = new QHash<QString, Product>;
    productsIdName = new QHash<qint32, QString>;

    gLayout = new QGridLayout(this);
//    gLayout->setContentsMargins(0,2,0,0);
    gLayout->setSpacing(8);
//    qCritical() << gLayout->spacing();
//    gLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    QHBoxLayout* infoLayout = new QHBoxLayout;
    productsLbl = new QLabel(tr("Products list:"), this);
    viewProductInfo = new QPushButton(this);
    viewProductInfo->setIcon(QIcon(":/icons/info.png"));
    viewProductInfo->setIconSize(QSize(16,16));
    viewProductInfo->setFixedSize(24,24);
    viewProductInfo->setToolTip(tr("View information about selected product"));
    viewProductInfo->setDisabled(true);
    connect(viewProductInfo, &QPushButton::clicked, this, &ProductsWidget::slotProductInfo);

    infoLayout->addWidget(productsLbl);
    infoLayout->addWidget(viewProductInfo);

    comboProducts = new QComboBox(this);
    comboProducts->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    connect(comboProducts, SIGNAL(currentIndexChanged(int)), this, SLOT(currentProductChanged(int)));

    productImageLbl = new QLabel(this);
    productImageLbl->hide();
    productImagePixmap = new QPixmap;

    reloadProductsButton = new QPushButton(tr("Reload list"), this);
    connect(reloadProductsButton, &QPushButton::clicked, this, &ProductsWidget::reloadProductsList);
    reloadProductsButton->hide();

    gLayout->addLayout(infoLayout, 0,0, 1, 2);
//    gLayout->addWidget(productsLbl);
    gLayout->addWidget(comboProducts, 1, 0, 1, 2);
    gLayout->addWidget(productImageLbl, 2, 0, 2, 2);
    gLayout->addWidget(reloadProductsButton);

    // Create request Url
    serverName = configData->serverName;
    cacheDir = configData->cacheDir;

    urlProducts = QUrl(serverName + "/api/products");
    urlGranules = QUrl(serverName + "/api/granules");

    reloadProductsList();

    // select parameters
    parametersLbl = new QLabel(tr("Select parameter: "), this);
    comboParameters = new QComboBox(this);
    comboParameters->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    comboParameters->setDisabled(true);

    gLayout->addWidget(parametersLbl, 0, 2, 1, 2);
    gLayout->addWidget(comboParameters, 1, 2, 1, 2);

    // add button
    addProductToFavoritesButton = new QPushButton(this);
    addProductToFavoritesButton->setContentsMargins(0,0,0,0);
    addProductToFavoritesButton->setFixedSize(24,24);

    addProductToFavoritesButton->setIconSize(QSize(16, 16));
    addProductToFavoritesButton->setFocusPolicy(Qt::NoFocus);
    addProductToFavoritesButton->setIcon(QIcon(":/icons/favorites.png"));
    addProductToFavoritesButton->setToolTip(tr("Add to favorites"));
    addProductToFavoritesButton->setCheckable(true);

    addProductToFavoritesButton->setDisabled(true);
    connect(addProductToFavoritesButton, &QPushButton::clicked, this, &ProductsWidget::addProductToFavorites);

    addProductLabel = new QPushButton(tr("Add product"), this);
    addProductLabel->setDisabled(true);
    connect(addProductLabel, SIGNAL(clicked()), this, SLOT(addProduct()));

    QHBoxLayout* bottomLayout = new QHBoxLayout(this);
    bottomLayout->addWidget(addProductToFavoritesButton);
    bottomLayout->addWidget(addProductLabel);

    gLayout->addLayout(bottomLayout, 2, 2, 1, 2);

    QFrame* hLine1 = new QFrame();
    hLine1->setFrameShape(QFrame::HLine);
    hLine1->setFrameShadow(QFrame::Sunken);
    gLayout->addWidget(hLine1, 4, 0, 1, 4);

// Add widgets for select Area

    QHBoxLayout* selectAreaLayout = new QHBoxLayout;
    QLabel* AreaLbl = new QLabel(tr("Select Area:"), this);
    AreaLbl->setContentsMargins(0,2,0,0);

    leftTopButton = new QPushButton(this);
    leftTopButton->setContentsMargins(0,0,0,0);
    leftTopButton->setFixedSize(24,24);
    leftTopButton->setIconSize(QSize(16, 16));
    leftTopButton->setFocusPolicy(Qt::NoFocus);

    leftTopButton->setIcon(QIcon(":/icons/left_top.png"));
    leftTopButton->setToolTip(tr("Overlay an image"));
    leftTopButton->setCheckable(true);
    connect(leftTopButton, &QPushButton::clicked, this, &ProductsWidget::setCheckedButton);

    selectAreaLayout->addWidget(AreaLbl);
    selectAreaLayout->addWidget(leftTopButton);

    gLayout->addLayout(selectAreaLayout, 5, 0, 1, 2);

    North = new InputBox(tr("North: "), this);
    North->setValidator("double");
    North->setText("90.00");
    North->setDisabled(true);

    South = new InputBox(tr("South: "), this);
    South->setValidator("double");
    South->setText("-90.00");
    South->setDisabled(true);

    West = new InputBox(tr("West: "), this);
    West->setValidator("double");
    West->setText("-180.00");
    West->setDisabled(true);

    East = new InputBox(tr("East: "), this);
    East->setValidator("double");
    East->setText("180.00");
    East->setDisabled(true);

    gLayout->addWidget(North, 6, 0, 2, 2);
    gLayout->addWidget(East, 6, 2, 2, 2);
    gLayout->addWidget(South, 8, 0, 2, 2);
    gLayout->addWidget(West, 8, 2, 2, 2);

// Set style

    setObjectName("ProductsWidget");
    setStyleSheet(QString("QWidget#ProductsWidget {background-color: "
                  "rgb(%1, %2, %3);}")
                  .arg(parent->palette().background().color().red())
                  .arg(parent->palette().background().color().green())
                  .arg(parent->palette().background().color().blue()));
}

// create Product structure from xml (from QDomElement)
Product createProductFromXml(QDomElement domElement)
{
    Product newProduct;

    // Get main fields
    newProduct.Name = domElement.firstChildElement("Name").text();
    newProduct.NaiadProductId = domElement.firstChildElement("NaiadProductId").text();
    newProduct.Id = domElement.firstChildElement("Id").text().toInt();
    newProduct.Description = domElement.firstChildElement("Description").text();
    newProduct.ProductionInterval = domElement.firstChildElement("ProductionInterval").text();
    newProduct.ImageUrl = domElement.firstChildElement("ImageUrl").text();
    newProduct.ProcessingLevels = domElement.firstChildElement("ProcessingLevels").text();
    newProduct.IsGlobalCoverage = domElement.firstChildElement("IsGlobalCoverage").text().toLower().startsWith("f") ? false : true;

    if (domElement.firstChildElement("IsTiled").text().toLower() == "true")
        newProduct.IsTiled = true;
    else
        newProduct.IsTiled = false;

    if (domElement.firstChildElement("TotalGranulesCount").text().isEmpty())
        newProduct.TotalGranulesCount = 0;
    else
        newProduct.TotalGranulesCount = \
                        domElement.firstChildElement("TotalGranulesCount").text().toInt();

    // get start and end date
    newProduct.StartDate = QDateTime::fromString(domElement.firstChildElement("StartDate").text(),
                                                 Qt::ISODate);
    newProduct.EndDate = QDateTime::fromString(domElement.firstChildElement("EndDate").text(),
                                               Qt::ISODate);

    // Get parameters
    QDomElement productParameters = domElement.firstChildElement("Parameters");
    QDomElement productParameter = productParameters.firstChildElement("Parameter");

    while ( !productParameter.isNull() )
    {
        newProduct.productsParameters[productParameter.firstChildElement("Name").text()] = \
                                      productParameter.firstChildElement("Id").text().toInt();
        productParameter = productParameter.nextSiblingElement("Parameter");
    }

    return newProduct;
}

// get xml response with products
void ProductsWidget::slotReadyReadProductList()
{
    reloadProductsButton->hide();

    QStringList productsList;
    productsList << "--"+tr("not selected")+"--";
    QNetworkReply *reply=qobject_cast<QNetworkReply*>(sender());

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error() == QNetworkReply::NoError)
    {
        qDebug() << "status code: " << statusCode;

        switch (statusCode)
        {
            case 200:
            {
                QByteArray bytes = reply->readAll();

                QDomDocument mDocument;
                QString errorMsg;
                int errorLine;
                int errorColumn;
                if (!mDocument.setContent(bytes, false, &errorMsg,
                                          &errorLine, &errorColumn))
                {
                    qWarning() << tr("Error parse XML");
                    if (errorLine > 1)
                    {
                        currentRequest = bytes;
                        return;
                    }
                    else
                    {
                        currentRequest = currentRequest + bytes;
                        bytes = currentRequest;
                        if (!mDocument.setContent(bytes, false,
                                                  &errorMsg, &errorLine, &errorColumn))
                        {
                                qWarning() << tr("Error parse XML");
                                qDebug() << errorMsg;
                                qDebug() << errorLine;
                                qDebug() << errorColumn;
                                return;
                        }
                    }
                }
                currentRequest.clear();

                QDomElement  mElement = mDocument.documentElement().firstChildElement("Product");
                while ( !mElement.isNull() )
                {
                    Product newProduct = createProductFromXml(mElement);


                    QDir().mkdir(cacheDir);
                    QString imagePath = QString(cacheDir+"/%1.jpg").arg(newProduct.NaiadProductId);

                    // download product image
                    if (!newProduct.ImageUrl.isEmpty())
                    {
                        if ( ! QFile::exists(imagePath) ) {
                            DownloadImage* downloadImage = new DownloadImage;
                            downloadImage->setImageUrl(serverName+newProduct.ImageUrl,
                                                       imagePath);
                            downloadImage->run();
                        }
                    }

                    QDomElement productID = mElement.firstChildElement("NaiadProductId");
                    productsHash->insert(productID.text(), newProduct);
                    productsIdName->insert(newProduct.Id, newProduct.NaiadProductId);

                    // create parametersList <parameterName, parameterId>
                    QHash<QString, int>::const_iterator k = newProduct.productsParameters.constBegin();
                    while ( k != newProduct.productsParameters.constEnd() )
                    {
                        if (!parametersList.contains(k.key()))
                            parametersList[k.key()] = k.value();
                        ++k;
                    }

                    productsList << productID.text();

                    mElement = mElement.nextSiblingElement("Product");
                }
                emit productsHashSignal(productsHash, productsIdName);
            }
        }
    }

    comboProducts->clear();
    comboProducts->addItems(productsList);
    addSavedProducts(true);
}

void ProductsWidget::getErrorProductList(QNetworkReply::NetworkError)
{
    reloadProductsButton->show();
    comboProducts->clear();
    comboProducts->addItem(tr("Connection Error"));
}

void ProductsWidget::reloadProductsList()
{
    QNetworkRequest request;
    request.setUrl(urlProducts);
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Accept-Language", "en-US");
    request.setRawHeader("Accept-Charset", "utf-8;q=0.7,*;q=0.7");
    request.setRawHeader("Content-Type", "text/xml");

    networkManager = new QNetworkAccessManager (this);
    QNetworkReply* reply = networkManager->get(request);
    qDebug() << request.url();
    connect(reply, &QNetworkReply::readyRead, this, &ProductsWidget::slotReadyReadProductList);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(getErrorProductList(QNetworkReply::NetworkError)));
}

void ProductsWidget::currentProductChanged(int index)
{
    bool enabledFlag;
    if (index == 0)
    {
        enabledFlag = false;
    }
    else
    {
        enabledFlag = true;
    }

    // if product not selected -> set disabled all elements
    if (enabledFlag != North->isEnabled())
    {
        North->setEnabled(enabledFlag);
        South->setEnabled(enabledFlag);
        West->setEnabled(enabledFlag);
        East->setEnabled(enabledFlag);

        comboParameters->setEnabled(enabledFlag);
        addProductLabel->setEnabled(enabledFlag);
        viewProductInfo->setEnabled(enabledFlag);

        addProductToFavoritesButton->setEnabled(enabledFlag);
    }

    // checked favorites products
    qint32 currentProductId = productsHash->value(comboProducts->currentText()).Id;
    if (enabledFlag)
        addProductToFavoritesButton->setChecked(_configData->favoriteProducts->contains(currentProductId));

    comboParameters->clear();
    if (enabledFlag)
    {
        // add new parameters in comboParameters
        comboParameters->addItems( \
                    productsHash->value(comboProducts->currentText()).productsParameters.keys());
        comboParameters->model()->sort(0);

        // set product image to productImageLbl
        if (QFile::exists(QString(cacheDir+"/%1.jpg").\
                          arg(productsHash->value(comboProducts->currentText()).NaiadProductId)))
        {
            productImagePixmap->load(QString(cacheDir+"/%1.jpg").\
                                 arg(productsHash->value(comboProducts->currentText()).NaiadProductId));
            *productImagePixmap = productImagePixmap->scaled(116,116);

            productImageLbl->setPixmap(*productImagePixmap);
            productImageLbl->show();
        }
        else {
            productImageLbl->hide();
        }
    }
    else
    {
        productImageLbl->hide();
    }
}

void ProductsWidget::setSelectedProducts(QHash<QString, selectedProduct> *selectedProductsValue,
                                         QHash<QString, Granule>* granulesHashValue)
{
    selectedProducts = selectedProductsValue;
    granulesHash = granulesHashValue;
}

void ProductsWidget::setObjectsPointer(TimeLine *timeLine)
{
    timeLinePointer = timeLine;
}

// add new product
void ProductsWidget::addProduct(ProductType::Type productType, QString productName)
{
    hide();

    selectedProduct newSelectedProduct;

    if (productName.isEmpty())
        newSelectedProduct.productName = comboProducts->currentText();
    else
        newSelectedProduct.productName = productName;

    newSelectedProduct.parameter = comboParameters->currentText();
    newSelectedProduct.north = North->text().toFloat();
    newSelectedProduct.east  = East->text().toFloat();
    newSelectedProduct.south = South->text().toFloat();
    newSelectedProduct.west  = West->text().toFloat();
    newSelectedProduct.id    = productsIdName->key(newSelectedProduct.productName);

    if (selectedProducts->keys().contains(newSelectedProduct.productName))
    {
        return;
//        selectedProducts->remove(newSelectedProduct.productName);
//        emit productDeleted(newSelectedProduct.productName);
    }

    selectedProducts->insert(newSelectedProduct.productName, newSelectedProduct);
    _configData->selectedProducts->append(newSelectedProduct.id);
    getGranulesForNewProduct();

    emit productAdded(newSelectedProduct.productName, productsIdName->key(newSelectedProduct.productName),
                      productType);

    qDebug() << selectedProducts->keys();
}

void ProductsWidget::addSavedProducts(bool favoritesOnly)
{
    foreach (qint32 n, *_configData->favoriteProducts)
        addProduct(ProductType::Favorite, productsIdName->value(n));

    if (!favoritesOnly)
    {
        foreach (qint32 n, *_configData->selectedProducts)
            addProduct(ProductType::Product, productsIdName->value(n));
    }
}

void ProductsWidget::addProductToFavorites(bool value)
{
    hide();
    qint32 currentKey = productsIdName->key(comboProducts->currentText());
    if (value && !_configData->favoriteProducts->contains(currentKey))
    {
        _configData->favoriteProducts->append(currentKey);
        addProduct(ProductType::Favorite);
    }
    else if(!value && _configData->favoriteProducts->contains(currentKey))
    {
        _configData->favoriteProducts->removeAll(currentKey);
        emit productDeleted(comboProducts->currentText());
    }

    // save favorites
    QSettings *settings = new QSettings(_configData->configFile, QSettings::IniFormat);

    QStringList favoriteProductsStringList;
    foreach(int n, *_configData->favoriteProducts) favoriteProductsStringList << QString::number(n);
    settings->setValue("other/favorite_products", favoriteProductsStringList);

    settings->sync();
    delete settings;
}

void ProductsWidget::getNewGranules(int scale)
{
    QHash<QString, selectedProduct>::const_iterator k = selectedProducts->constBegin();
    while ( k != selectedProducts->constEnd())
    {
        // create request
        QNetworkRequest request;
        QString filter = "?";

        filter += "productId="+QString::number(productsHash->value(k.key()).Id);
        filter += "&date=" + \
                timeLinePointer->control_.currentDate.date().toString("yyyy-MM-dd");
        filter += "&range=" + QString::number(24/scale);

        // get coords and set area filter
        QString north = North->text();
        QString east = East->text();
        QString south = South->text();
        QString west = West->text();

        if (productsHash->value(k.key()).IsTiled)
        {
            filter += "&area=(" + QString("%1,%2,%3,%4)").arg(north, east, south, west);
        }

        request.setUrl(QUrl(urlGranules.scheme() + "://" + urlGranules.host() + urlGranules.path() + filter));

        request.setRawHeader("Content-Type", "text/xml");

        GetGranules* getGranules = new GetGranules();

        connect(getGranules, SIGNAL(timeLineRepaint()), timeLinePointer, SLOT(update()));
//        connect (getGranules, SIGNAL(finished()), getGranules, SLOT(deleteLater()));
        getGranules->setSelectedProducts(selectedProducts, granulesHash);
        getGranules->setParameters(request, "getNewGranules");
        getGranules->run();
//        getGranules->deleteLater();

        ++k;
    }
}

void ProductsWidget::getGranulesForNewProduct()
{
    QNetworkRequest request;

    //create filter
    QString filter = "?";

    filter += "productId="+QString::number(productsHash->value(comboProducts->currentText()).Id);
    filter += "&date=" + \
            timeLinePointer->control_.currentDate.date().toString("yyyy-MM-dd");
    filter += "&range=20";

    // get coords and set area filter
    QString north = North->text();
    QString east = East->text();
    QString south = South->text();
    QString west = West->text();

    if (productsHash->value(comboProducts->currentText()).IsTiled)
    {
        filter += "&area=(" + QString("%1,%2,%3,%4)").arg(north, east, south, west);
    }

    request.setUrl(QUrl(urlGranules.scheme() + "://" + urlGranules.host() + urlGranules.path() + filter));
    request.setRawHeader("Content-Type", "text/xml");

    qDebug() << request.url();

    GetGranules* getGranules = new GetGranules();
    connect(getGranules, SIGNAL(timeLineRepaint()), timeLinePointer, SLOT(update()));
    connect(getGranules, SIGNAL(moveTimeLine(QDateTime)), timeLinePointer, SLOT(moveTimeLine(QDateTime)));
//    connect (getGranules, SIGNAL(finished()), getGranules, SLOT(deleteLater()));
    getGranules->setSelectedProducts(selectedProducts, granulesHash);
    getGranules->setParameters(request, "getGranulesForNewProduct");
    getGranules->run();
//    getGranules->deleteLater();
}

void ProductsWidget::slotProductInfo()
{
    ProductInfoWidget* productInfo = new ProductInfoWidget;
    productInfo->setProduct(productsHash->value(comboProducts->currentText()), productImagePixmap);
    productInfo->show();
}

void ProductsWidget::removeProduct(QString productId)
{
    //remove product from selectedProducts
    qDebug() << "removeProduct";
    if (selectedProducts->keys().contains(productId))
    {
        qDebug() << tr("Removal product") << productId;
        selectedProducts->remove(productId);
        _configData->selectedProducts->removeAll(productsIdName->key(productId));
    }

    // remobe granules from granulesHash (only for removed product)
    QList<QString> granuleIdlist;

    QHash<QString, Granule>::const_iterator k = granulesHash->constBegin();
    while ( k != granulesHash->constEnd())
    {
        if (productsHash->value(productId).Id == k.value().productId)
            granuleIdlist.append(k.key());
        ++k;
    }

    QList<QString>::iterator cur = granuleIdlist.begin();
    QList<QString>::iterator last = granuleIdlist.end();

    while ( cur != last)
    {
//        qDebug() << "REMOVE GRANULES: " << granuleIdlist.at(0);
        granulesHash->remove(granuleIdlist.takeFirst());
        cur = granuleIdlist.begin();
        last = granuleIdlist.end();
    };

    if (_configData->favoriteProducts->contains(productsHash->value(productId).Id))
    {
        _configData->favoriteProducts->removeAll(productsHash->value(productId).Id);
        if (comboProducts->currentText() == productId)
            addProductToFavoritesButton->setChecked(false);
    }
}

void ProductsWidget::updateButtons()
{
    if (_configData->favoriteProducts->contains(productsHash->value(comboProducts->currentText()).Id))
        addProductToFavoritesButton->setChecked(true);
    else
        addProductToFavoritesButton->setChecked(false);
}

void ProductsWidget::areaCoordsSlot(GeoCoords pos1, GeoCoords pos2)
{
    qreal north = ((pos1.lat > pos2.lat) ? pos1.lat : pos2.lat)*180/M_PI;
    qreal west = ((pos1.lon < pos2.lon) ? pos1.lon : pos2.lon)*180/M_PI;
    qreal south = ((pos1.lat < pos2.lat) ? pos1.lat : pos2.lat)*180/M_PI;
    qreal east = ((pos1.lon > pos2.lon) ? pos1.lon : pos2.lon)*180/M_PI;

    North->setText(QString::number(north));
    West->setText(QString::number(west));
    South->setText(QString::number(south));
    East->setText(QString::number(east));
}

void ProductsWidget::setCheckedButton(bool value)
{
    if (value)
    {
        leftTopButton->setChecked(false);
        CursorMode::Mode curMode = CursorMode::GetAreaCoords;
        emit setCursorModeSignal(curMode);
    }
    else
    {
        leftTopButton->setChecked(false);
        emit setCursorModeSignal(CursorMode::Move);
    }
}

void ProductsWidget::setCursorModeSlot(CursorMode::Mode value)
{
    leftTopButton->setChecked(false);
    if (value == CursorMode::GetAreaCoords)
        leftTopButton->setChecked(true);
}
