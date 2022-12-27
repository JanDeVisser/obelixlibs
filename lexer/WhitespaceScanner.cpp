/*
 * Copyright (c) 2021, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

//
// Created by Jan de Visser on 2021-10-05.
//

#include <lexer/Tokenizer.h>

namespace Obelix {

WhitespaceScanner::WhitespaceScanner()
    : Scanner(20)
{
}

WhitespaceScanner::WhitespaceScanner(WhitespaceScanner::Config const& config)
    : Scanner(20)
    , m_config(config)
{
}

WhitespaceScanner::WhitespaceScanner(bool ignore_all_ws)
    : Scanner(20)
{
    if (ignore_all_ws) {
        m_config.newlines_are_spaces = true;
        m_config.ignore_spaces = true;
    }
}

void WhitespaceScanner::match(Tokenizer& tokenizer) {
    int ch;

    for (m_state = WhitespaceState::Init, ch = tokenizer.peek();
         ch && m_state != WhitespaceState::Done; ch = tokenizer.peek()) {
#if 0
        if ((ch == '\n' || ch == '\r') && !m_config.newlines_are_spaces) {
            if (m_state == WhitespaceState::Whitespace) {
                if (m_config.ignore_spaces) {
                    tokenizer.skip();
                } else {
                    tokenizer.accept(TokenCode::Whitespace);
                }
            }
            if (ch == '\r') {
                ch = tokenizer.peek(1);
                if (ch == '\n') {
                    tokenizer.discard();
                    tokenizer.push();
                } else {
                    tokenizer.push_as('\n');
                }
            } else {
                tokenizer.push();
            }
            if (m_config.ignore_newlines) {
                tokenizer.skip();
            } else {
                tokenizer.accept(TokenCode::NewLine);
            }
            m_state = WhitespaceState::Done;
            continue;
        }
#endif
        switch (m_state) {
            case WhitespaceState::Init:
                if (isspace(ch)) {
                    if (ch == '\r' || ch == '\n') {
                        if (!m_config.newlines_are_spaces) {
                            if (ch == '\r') {
                                ch = tokenizer.peek(1);
                                if (ch == '\n') {
                                    tokenizer.discard();
                                    tokenizer.push();
                                } else {
                                    tokenizer.push_as('\n');
                                }
                            } else {
                                tokenizer.push();
                            }
                            if (m_config.ignore_newlines) {
                                tokenizer.skip();
                            } else {
                                tokenizer.accept(TokenCode::NewLine);
                            }
                            m_state = WhitespaceState::Done;
                        } else {
                            tokenizer.push();
                            m_state = WhitespaceState::Whitespace;
                        }
                    } else {
                        tokenizer.push();
                        m_state = WhitespaceState::Whitespace;
                    }
                } else {
                    m_state = WhitespaceState::Done;
                }
                break;
            case WhitespaceState::Whitespace:
                if (isspace(ch)) {
                    if (ch == '\r' || ch == '\n') {
                        if (!m_config.newlines_are_spaces) {
                            if (m_config.ignore_spaces) {
                                tokenizer.skip();
                            } else {
                                tokenizer.accept(TokenCode::Whitespace);
                            }
                            if (ch == '\r') {
                                ch = tokenizer.peek(1);
                                if (ch == '\n') {
                                    tokenizer.discard();
                                    tokenizer.push();
                                } else {
                                    tokenizer.push_as('\n');
                                }
                            } else {
                                tokenizer.push();
                            }
                            if (m_config.ignore_newlines) {
                                tokenizer.skip();
                            } else {
                                tokenizer.accept(TokenCode::NewLine);
                            }
                            m_state = WhitespaceState::Done;
                        } else {
                            tokenizer.push();
                            m_state = WhitespaceState::Whitespace;
                        }
                    } else {
                        tokenizer.push();
                        m_state = WhitespaceState::Whitespace;
                    }
                } else {
                    if (m_config.ignore_spaces) {
                        tokenizer.skip();
                    } else {
                        tokenizer.accept(TokenCode::Whitespace);
                    }
                    m_state = WhitespaceState::Done;
                }
                break;
            default:
                break;
        }
    }
    if (!ch && m_state == WhitespaceState::Whitespace) {
        if (m_config.ignore_spaces) {
            tokenizer.skip();
        } else {
            tokenizer.accept(TokenCode::Whitespace);
        }
    }
}

}
