/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QObject>
#include <QPromise>
#include <QQueue>
#include <QWebEnginePage>

// C++ include.
#include <utility>

//! Init Qt resources.
void initMdPdfSharedResources();

namespace MdPdf
{

//
// WebPage
//

//! Web page.
class WebPage : public QWebEnginePage
{
public:
    explicit WebPage(QObject *parent);
    ~WebPage() override = default;

protected:
    bool acceptNavigationRequest(const QUrl &url,
                                 NavigationType type,
                                 bool isMainFrame) override;
}; // class WebPage

//
// Diagram
//

//! Helper to convert Mermaid diagram into SVG.
//! This object must live in GUI thread.
class Diagram : public QObject
{
    Q_OBJECT

public:
    Diagram(const QString &fontName,
            const QString &monoFontName,
            int fontSize,
            QObject *parent = nullptr);
    ~Diagram() override;

    //! \return Base font size.
    int fontSize() const;

    //! Convert Mermaid diagram into SVG.
    Q_INVOKABLE void convert(const QString &mermaid,
                             QSharedPointer<QPromise<QString>> promise);

private Q_SLOTS:
    void onPageLoaded(bool res);

private:
    Q_DISABLE_COPY(Diagram)

    QQueue<std::pair<QString, QSharedPointer<QPromise<QString>>>> m_queue;
    WebPage *m_page = nullptr;
    bool m_loaded = false;
    bool m_error = false;
    const QString m_fontName;
    const QString m_monoFontName;
    const int m_fontSize = 16;
}; // class Diagram

} /* namespace MdPdf */
