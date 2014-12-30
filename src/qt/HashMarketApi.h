#ifndef HASHMARKETAPI_H
#define HASHMARKETAPI_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
class QUrl;

class HashMarketApi : public QObject
{
        Q_OBJECT
    public:
        explicit HashMarketApi(QObject *parent = 0);

        void refresh();

    signals:
        void updateMarket(const QString &market,const QString &price);

    public slots:
        void mintpalFinished();
        void bittrexFinished();
        void ccexFinished();
        void poloniexFinished();
        void cryptsyFinished();
        void betasharexFinished();


    private:

        void mintpal();
        void bittrex();
        void ccex();
        void poloniex();
        void cryptsy();
        void betasharex();


        QNetworkAccessManager *nam;
        QJsonObject result(QNetworkReply *reply );
        QString market;
        int marketId;


};

#endif // HASHMARKETAPI_H
