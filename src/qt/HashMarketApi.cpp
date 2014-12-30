/* Author: Yash yash@speedovation.com Owner: poolstamp
 *
 * AIM : Update exchanges last prices for CHASH market when requested
 *
 * Overview screen will call refresh and connected using signal.
 *
 * On the overview page we are pulling current prices from the markets to show the current price for CHASH.
 *
 */
#include "HashMarketApi.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

#define ERROR_DISPLAY_MESSAGE "ApiError"

///FIXME: Important before going live
///I failed to find  global market which exists in all exchanges so I used diff market name for each exchange (which exists) seperately
///and needs to be changed when we go live


HashMarketApi::HashMarketApi(QObject *parent) :
    QObject(parent)
{
    nam = new QNetworkAccessManager(this);
    market = "XSP";
}

void HashMarketApi::refresh()
{
//    mintpal();
    bittrex();
    ccex();
    poloniex();
    cryptsy();
    betasharex();
}


/*
Mintpal
https://api.mintpal.com/v2/market/stats/CHASH/BTC
we want to pull the info from “last_price”
https://api.mintpal.com/v2/market/stats/AUR/BTC is an example as the market CHASH has not been created.
*/

void HashMarketApi::mintpal()
{
   ///TO BE Removed
//    market = "SUPER";

    QUrl url( QString("https://api.mintpal.com/v2/market/stats/%1/BTC").arg(market) );
    QNetworkReply* reply = nam->get(QNetworkRequest(url));

    QObject::connect(reply, SIGNAL(finished()), this, SLOT(mintpalFinished()));

}

void HashMarketApi::mintpalFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*> ( sender() );

    QJsonObject jsonObject = result(reply);
    QVariantMap data = jsonObject["data"].toObject().toVariantMap();

    if( jsonObject["status"].toString() != "success" )
    {
        qDebug() << ERROR_DISPLAY_MESSAGE;
        updateMarket("Mintpal",QString::number(0,'f',8 ));
        return;
    }

    emit updateMarket("Mintpal",data.value("last_price").toString());

}
/*
Bittrex
https://bittrex.com/api/v1.1/public/getmarketsummary?market=hash-btc
we need to pull the info from “Last”
https://bittrex.com/api/v1.1/public/getmarketsummary?market=btc-aur is an example as the market CHASH has not been created.
*/

void HashMarketApi::bittrex()
{
    ///TO BE Removed
//    market = "SUPER";

    QUrl url( QString("https://bittrex.com/api/v1.1/public/getmarketsummary?market=btc-%1").arg(market) );
    QNetworkReply* reply = nam->get(QNetworkRequest(url));

    QObject::connect(reply, SIGNAL(finished()), this, SLOT(bittrexFinished()));

}
void HashMarketApi::bittrexFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*> ( sender() );

    QJsonObject jsonObject = result(reply);

    if( jsonObject["success"].toBool() != true )
    {
        qDebug() << ERROR_DISPLAY_MESSAGE;
        //return ;
    }

   float r = jsonObject["result"].toArray().first().toObject().toVariantMap().value("Last").toFloat();

    emit updateMarket("Bittrex", QString::number(r,'f',8 ) );

}
/*
C-Cex
https://c-cex.com/t/hash-btc.json
shoule be https://c-cex.com/t/btc-hash.json

we need to pull the info from “lastbuy”
https://c-cex.com/t/btc-usd.json is an example as the market CHASH has not been created.
*/

void HashMarketApi::ccex()
{
    ///TO BE Removed
//    market = "usd";

    QUrl url( QString("https://c-cex.com/t/btc-%1.json").arg(market) );
    QNetworkReply* reply = nam->get(QNetworkRequest(url));

    QObject::connect(reply, SIGNAL(finished()),
                     this, SLOT(ccexFinished()));
}
void HashMarketApi::ccexFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*> ( sender() );

    QJsonObject jsonObject = result(reply);

    float r = jsonObject["ticker"].toObject().toVariantMap().value("lastprice").toFloat();

    emit updateMarket("C-Cex", QString::number(r,'f',8 ) );

}
/*
Poloniex
https://poloniex.com/public?command=returnTicker
we need to pull info from line BTC_CHASH
then need data from “last”
Poloniex does not have an API command to pull from just one market.
*/
void HashMarketApi::poloniex()
{
    QUrl url( QString("https://poloniex.com/public?command=returnTicker") );
    QNetworkReply* reply = nam->get(QNetworkRequest(url));

    QObject::connect(reply, SIGNAL(finished()),
                     this, SLOT(poloniexFinished()));
}
void HashMarketApi::poloniexFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*> ( sender() );
    QJsonObject jsonObject = result(reply);

    ///TO BE Removed
//    market = "AUR";

    float r = jsonObject[QString("BTC_%1").arg(market)].toObject().toVariantMap().value("last").toFloat();

    emit updateMarket("Poloniex", QString::number(r,'f',8 ) );

}
/*
Cryptsy
http://pubapi.cryptsy.com/api.php?method=singlemarketdata&marketid=127
we need to pull info from “lasttradeprice”
market id will change to our market after launch.   Market 127 can be used for now.
*/

void HashMarketApi::cryptsy()
{
    ///FIXME: update ID
    /// 127 for testing
    marketId = 127000;
    QUrl url( QString("http://pubapi.cryptsy.com/api.php?method=singlemarketdata&marketid=%1").arg(marketId) );
    QNetworkReply* reply = nam->get(QNetworkRequest(url));

    QObject::connect(reply, SIGNAL(finished()), this, SLOT(cryptsyFinished()));
}
void HashMarketApi::cryptsyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*> ( sender() );
    QJsonObject jsonObject = result(reply);

    QString str = jsonObject["return"].toObject()["markets"].toObject()["ZET"].toObject().toVariantMap().value("lasttradeprice").toString();

    if(str.isEmpty())
        str= QString::number(0,'f',8 ) ;

    emit updateMarket("Cryptsy", str );

}

QJsonObject HashMarketApi::result(QNetworkReply *reply )
{
    if(!reply)
    {
        qDebug() << "Reply object not found: ERROR";
        return QJsonObject();
    }

    QString strReply = QString(reply->readAll());

    QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
    return  jsonResponse.object();

}
/*
BetaShareX
https://api.betasharex.com/v1/market/chartdata/1/1DD
we need to pull info from “lasttradeprice”
market id will change to our market after launch.   Market 127 can be used for now.
*/
void HashMarketApi::betasharex()
{
    QUrl url( QString("https://api.betasharex.com/v1/market/stats/XSP/BTC") );
    QNetworkReply* reply = nam->get(QNetworkRequest(url));

    QObject::connect(reply, SIGNAL(finished()),
                     this, SLOT(betasharexFinished()));
}
void HashMarketApi::betasharexFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*> ( sender() );
    QJsonObject jsonObject = result(reply);

    ///TO BE Removed
//    market = "AUR";

    float r = jsonObject[QString("%1").arg(market)].toObject().toVariantMap().value("last_price").toFloat();

    emit updateMarket("BetaShareX", QString::number(r,'f',8 ) );

}
