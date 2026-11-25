/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-pdf-lib include.
#include "diagram.h"

// Qt include.
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QWebEngineSettings>

void initMdPdfSharedResources()
{
    Q_INIT_RESOURCE(resources);
}

namespace MdPdf
{

//
// WebPage
//

WebPage::WebPage(QObject *parent)
    : QWebEnginePage(parent)
{
    settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
}

bool WebPage::acceptNavigationRequest(const QUrl &url,
                                      QWebEnginePage::NavigationType /*type*/,
                                      bool /*isMainFrame*/)
{
    if (url.scheme() == QStringLiteral("qrc") || url.scheme() == QStringLiteral("data")) {
        return true;
    }

    return false;
}

//
// Diagram
//

Diagram::Diagram(const QString &fontName,
                 const QString &monoFontName,
                 int fontSize,
                 QObject *parent)
    : QObject(parent)
    , m_page(new WebPage(this))
    , m_fontName(fontName)
    , m_monoFontName(monoFontName)
    , m_fontSize(fontSize)
{
    connect(m_page, &QWebEnginePage::loadFinished, this, &Diagram::onPageLoaded);

    m_page->load(QUrl(QStringLiteral("qrc:/js/index.html")));
}

Diagram::~Diagram() = default;

int Diagram::fontSize() const
{
    return m_fontSize;
}

void Diagram::convert(const QString &mermaid,
                      QSharedPointer<QPromise<QString>> promise)
{
    if (m_error) {
        if (promise) {
            promise->start();
            promise->addResult(QString());
            promise->finish();
        }

        return;
    }

    if (!m_loaded) {
        if (promise) {
            m_queue.enqueue(std::make_pair(mermaid, promise));
        }

        return;
    }

    std::pair<QString, QSharedPointer<QPromise<QString>>> mmd;

    if (!m_queue.isEmpty()) {
        mmd = m_queue.dequeue();
    } else {
        if (promise) {
            mmd = std::make_pair(mermaid, promise);
        } else {
            return;
        }
    }

    mmd.second->start();

    const QString jsonArray = QString::fromUtf8(QJsonDocument(QJsonArray{mmd.first}).toJson(QJsonDocument::Compact));

    const QString safeJsString = jsonArray.mid(1, jsonArray.length() - 2);

    m_page->runJavaScript(QStringLiteral("renderMermaid(%1, '%2', '%3', %4)")
                              .arg(safeJsString)
                              .arg(m_fontName)
                              .arg(m_monoFontName)
                              .arg(m_fontSize),
                          [p = mmd.second](const QVariant &v) {
                              p->addResult(v.toString());
                              p->finish();
                          });
}

void Diagram::onPageLoaded(bool res)
{
    if (res) {
        m_loaded = true;

        while (!m_queue.isEmpty()) {
            convert(QString(), nullptr);
        }
    } else {
        m_error = true;
    }
}

} /* namespace MdPdf */
