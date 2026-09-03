/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// shared include.
#include "utils.h"
#include "emoji_parser.h"
#include "emphasis.h"
#include "syntax.h"

// Qt include.
#include <QtResource>

// md4qt inclide.
#include <md4qt/src/asterisk_emphasis_parser.h>
#include <md4qt/src/atx_heading_parser.h>
#include <md4qt/src/autolink_parser.h>
#include <md4qt/src/blockquote_parser.h>
#include <md4qt/src/emphasis_parser.h>
#include <md4qt/src/fenced_code_parser.h>
#include <md4qt/src/footnote_parser.h>
#include <md4qt/src/gfm_autolink_parser.h>
#include <md4qt/src/hard_line_break_parser.h>
#include <md4qt/src/html_parser.h>
#include <md4qt/src/indented_code_parser.h>
#include <md4qt/src/inline_code_parser.h>
#include <md4qt/src/inline_html_parser.h>
#include <md4qt/src/inline_math_parser.h>
#include <md4qt/src/link_image_parser.h>
#include <md4qt/src/list_parser.h>
#include <md4qt/src/paragraph_parser.h>
#include <md4qt/src/setext_heading_parser.h>
#include <md4qt/src/strikethrough_emphasis_parser.h>
#include <md4qt/src/table_parser.h>
#include <md4qt/src/thematic_break_parser.h>
#include <md4qt/src/underline_emphasis_parser.h>
#include <md4qt/src/yaml_parser.h>

namespace MdPdf
{

//
// Utf8String
//

Utf8String::Utf8String(const QByteArray &a)
    : data(a)
{
}

Utf8String::Utf8String(const char *s)
    : data(s)
{
}

Utf8String::operator const char *() const
{
    return data.data();
}

Utf8String::operator std::string_view() const
{
    return data.data();
}

void initSharedResources()
{
    Q_INIT_RESOURCE(resources);
    Q_INIT_RESOURCE(latex);
    Syntax::init();
}

Word::Word(const QString &word,
           bool rtl,
           bool onNewLine,
           const SkFont *font)
    : m_word(word)
    , m_rtl(rtl)
    , m_onNewLine(onNewLine)
    , m_font(font)
{
}

bool isRightToLeft(const QChar &ch)
{
    switch (ch.direction()) {
    case QChar::DirAL:
    case QChar::DirAN:
    case QChar::DirR:
        return true;

    default:
        return false;
    }
}

QVector<Word> splitString(const QString &str,
                          bool skipSpaces,
                          sk_sp<SkUnicode> unicode,
                          bool rtl)
{
    static const QString s_spaceString = QStringLiteral(" ");

    QVector<Word> res;

    if (str.isEmpty()) {
        return res;
    }

    const auto uint16 = reinterpret_cast<const uint16_t *>(str.utf16());
    const auto char16 = reinterpret_cast<const char16_t *>(str.utf16());

    auto bidiIter = unicode->makeBidiIterator(uint16,
                                              str.size(),
                                              rtl ? SkBidiIterator::Direction::kRTL : SkBidiIterator::Direction::kLTR);

    if (!bidiIter) {
        return res;
    }

    auto iter = unicode->makeBreakIterator(SkUnicode::BreakType::kWords);

    if (!iter) {
        return res;
    }

    iter->setText(char16, str.size());

    do {
        const auto startPos = iter->current();
        const auto endPos = iter->next();

        if (iter->isDone()) {
            break;
        }

        const QString word = QString::fromUtf16(&char16[startPos], endPos - startPos);

        if (word.simplified().isEmpty()) {
            if (!skipSpaces && ((!res.isEmpty() && res.back().m_word != s_spaceString) || res.isEmpty())) {
                res.append({s_spaceString, false, false, nullptr});
            }
        } else {
            res.append({word, bidiIter->getLevelAt(startPos) % 2 != 0, false, nullptr});
        }
    } while (true);

    return res;
}

void orderWords(QVector<Word> &text,
                bool rtl)
{
    qsizetype start = -1;
    qsizetype end = -1;

    auto reverseItems = [](qsizetype start, qsizetype end, QVector<Word> &data) {
        if (start > -1 && end > start) {
            while (end - start > 0) {
                data.swapItemsAt(start, end);
                ++start;
                --end;
            }
        }
    };

    for (qsizetype i = 0; i < text.size(); ++i) {
        if (text[i].m_word != QStringLiteral(" ")) {
            if (text[i].m_rtl != rtl) {
                if (start == -1) {
                    start = i;
                    end = i;
                } else {
                    end = i;
                }
            } else {
                reverseItems(start, end, text);

                start = -1;
                end = -1;
            }
        }
    }

    reverseItems(start, end, text);
}

void setPlugins(MD::Parser &parser,
                const PluginsCfg &cfg,
                bool enableEmoji)
{
    MD::Parser::InlineParsers inlineParsers;

    MD::Parser::appendInlineParser<MD::InlineCodeParser>(inlineParsers);

    auto linkParser = MD::Parser::appendInlineParser<MD::LinkImageParser>(inlineParsers);

    MD::Parser::appendInlineParser<MD::AutolinkParser>(inlineParsers);
    MD::Parser::appendInlineParser<MD::InlineHtmlParser>(inlineParsers);
    MD::Parser::appendInlineParser<MD::InlineMathParser>(inlineParsers);
    MD::Parser::appendInlineParser<MD::AsteriskEmphasisParser>(inlineParsers);
    MD::Parser::appendInlineParser<MD::UnderlineEmphasisParser>(inlineParsers);
    MD::Parser::appendInlineParser<MD::StrikethroughEmphasisParser>(inlineParsers);

    if (enableEmoji) {
        MD::Parser::appendInlineParser<EmojiParser>(inlineParsers);
    }

    if (cfg.m_sup.m_on) {
        inlineParsers.append(QSharedPointer<SupEmphasisParser>::create(cfg.m_sup.m_delimiter));
    }

    if (cfg.m_sub.m_on) {
        inlineParsers.append(QSharedPointer<SubEmphasisParser>::create(cfg.m_sub.m_delimiter));
    }

    if (cfg.m_mark.m_on) {
        inlineParsers.append(QSharedPointer<HighlightEmphasisParser>::create(cfg.m_mark.m_delimiter));
    }

    inlineParsers.append(QSharedPointer<MD::GfmAutolinkParser>::create(linkParser));

    MD::Parser::appendInlineParser<MD::HardLineBreakParser>(inlineParsers);

    MD::Parser::BlockParsers blockParsers;

    if (cfg.m_yamlEnabled) {
        MD::Parser::appendBlockParser<MD::YAMLParser>(blockParsers, &parser);
    }

    MD::Parser::appendBlockParser<MD::BlockquoteParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::SetextHeadingParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::ThematicBreakParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::ListParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::ATXHeadingParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::FencedCodeParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::HTMLParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::IndentedCodeParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::FootnoteParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::TableParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::ParagraphParser>(blockParsers, &parser);

    parser.setBlockParsers(blockParsers);
    parser.setInlineParsers(inlineParsers);
}

} /* namespace MdPdf */
