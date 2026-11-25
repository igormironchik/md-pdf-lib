/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QObject>
#include <QWebEnginePage>

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
class Diagram : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    //! Start converting of Mermaid diagram into SVG.
    void convert(const QString &mermaid);
    //! Converting finished.
    void finished(const QString &svg);

public:
    Diagram(const QString &fontName,
            const QString &monoFontName,
            int fontSize,
            QObject *parent = nullptr,
            bool async = false);
    ~Diagram() override;

    //! \return Base font size.
    int fontSize() const;

private Q_SLOTS:
    void onStart(const QString &mermaid);

private:
    Q_DISABLE_COPY(Diagram)

    WebPage *m_page = nullptr;
    bool m_loaded = false;
    QString m_fontName;
    QString m_monoFontName;
    int m_fontSize = 16;
}; // class Diagram

} /* namespace MdPdf */
