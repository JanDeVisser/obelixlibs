/*
 * Copyright (c) 2023, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <lexer/Tokenizer.h>

namespace Obelix {

CustomScanner::CustomScanner(std::string name, CustomScanner::Match matcher, int priority)
    : Scanner(priority)
    , m_name(std::move(name))
    , m_match(std::move(matcher))
{
}

void CustomScanner::match(Tokenizer& tokenizer)
{
    assert(m_match != nullptr);
    m_match(tokenizer);
}

}
