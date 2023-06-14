/*
 * Copyright (c) 2023, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <lexer/BasicParser.h>

namespace Obelix {

ErrorOr<std::shared_ptr<PlainTextParser>,SystemError> PlainTextParser::create(std::string const& file_name, BufferLocator* locator)
{
    auto ret = std::make_shared<PlainTextParser>();
    TRY_RETURN(ret->read_file(file_name, locator));
    return ret;
}

PlainTextParser::PlainTextParser(StringBuffer const& src)
    : PlainTextParser()
{
    lexer().assign(src.str());
}

PlainTextParser::PlainTextParser()
    : BasicParser()
{
    lexer().add_scanner("plaintext", [](Tokenizer& tokenizer) {
        switch (int ch = tokenizer.peek()) {
        case '\n':
            tokenizer.push();
            tokenizer.accept(TokenCode::NewLine);
            break;
        case 0:
            break;
        default:
            do {
                tokenizer.push();
                ch = tokenizer.peek();
            } while (ch && ch != '\n');
            tokenizer.accept(TokenCode::Text);
            break;
        }
    });
}

}
