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
    connect(m_page, &QWebEnginePage::loadFinished, this, [this](bool ok) {
        m_loaded = ok;
    });

    m_page->load(QUrl(QStringLiteral("qrc:/js/index.html")));
}

Diagram::~Diagram() = default;

int Diagram::fontSize() const
{
    return m_fontSize;
}

QString Diagram::convert(const QString &mermaid)
{
    if (!m_loaded) {
        QEventLoop loadLoop;
        auto conn = connect(m_page, &QWebEnginePage::loadFinished, &loadLoop, &QEventLoop::quit);
        loadLoop.exec();
        disconnect(conn);
    }

    const QString jsonArray = QString::fromUtf8(QJsonDocument(QJsonArray{mermaid}).toJson(QJsonDocument::Compact));

    const QString safeJsString = jsonArray.mid(1, jsonArray.length() - 2);

    QEventLoop loop;

    QString svg;

    m_page->runJavaScript(QStringLiteral("renderMermaid(%1, '%2', '%3', %4)")
                              .arg(safeJsString)
                              .arg(m_fontName)
                              .arg(m_monoFontName)
                              .arg(m_fontSize),
                          [&loop, &svg](const QVariant &v) {
                              svg = v.toString();
                              loop.quit();
                          });

    loop.exec();

    return svg;
}

} /* namespace MdPdf */
