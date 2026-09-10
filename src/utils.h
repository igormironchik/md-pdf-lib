/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// md-pdf-lib include.
#include "renderer_aux.h"

// Qt include.
#include <QString>
#include <QVector>

// md4qt include.
#include <md4qt/src/parser.h>

// Skia include.
#include <modules/skunicode/include/SkUnicode.h>

namespace MdPdf
{

//! Init shared resources, like *.qrc.
void initSharedResources();

//! \return Is a given character RTL one?
bool isRightToLeft(const QChar &ch);

//! Auxiliary struct for splitted words.
struct Word {
    Word() = default;
    Word(const QString &word,
         bool rtl,
         bool onNewLine,
         const Font *font);

    QString m_word;
    bool m_rtl = false;
    bool m_onNewLine = false;
    const Font *m_font = nullptr;
}; // struct Word

//! Makes Utf8String from QString.
inline Utf8String createUtf8String(const QString &text)
{
    return {text.toUtf8()};
}

/*!
 * \brief Split string by spaces.
 * \param str String.
 * \param skipSpaces If false in returned vector will be spaces too.
 * \return Vector of words with flag indicates RTL.
 */
QVector<Word> splitString(const QString &str,
                          bool skipSpaces,
                          sk_sp<SkUnicode> unicode,
                          bool rtl);

//! \return Whether the given symbol is a DirCS.
inline bool isSeparator(const QChar &ch)
{
    return (ch.direction() == QChar::DirCS);
}

//! Order words for painting with Qt with RTL, LTR rules.
void orderWords(QVector<Word> &text,
                bool rtl);

//
// EmphasisPluginCfg
//

//! Configuration of emphasis plugin
struct EmphasisPluginCfg {
    //! Delimiter.
    QChar m_delimiter;
    //! Is on?
    bool m_on = false;
}; // struct EmphasisPluginCfg

inline bool operator!=(const EmphasisPluginCfg &c1,
                       const EmphasisPluginCfg &c2)
{
    return (c1.m_delimiter != c2.m_delimiter || c1.m_on != c2.m_on);
}

//
// PluginsCfg
//

//! Configuration of plugins.
struct PluginsCfg {
    //! Configuration of superscript plugin.
    EmphasisPluginCfg m_sup;
    //! Configuration of subscrip plugin.
    EmphasisPluginCfg m_sub;
    //! Configuration of mark plugin.
    EmphasisPluginCfg m_mark;
    //! YAML header ON/OFF.
    bool m_yamlEnabled = false;
}; // struct PluginsCfg

inline bool operator!=(const PluginsCfg &c1,
                       const PluginsCfg &c2)
{
    return (
        c1.m_sup != c2.m_sup || c1.m_sub != c2.m_sub || c1.m_mark != c2.m_mark || c1.m_yamlEnabled != c2.m_yamlEnabled);
}

//! Set plugins to parser.
void setPlugins(MD::Parser &parser,
                const PluginsCfg &cfg,
                bool enableEmoji = false);

} /* namespace MdPdf */
