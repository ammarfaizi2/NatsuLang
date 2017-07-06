#pragma once
#include "Token.h"

namespace NatsuLang
{
	class Preprocessor;

	namespace Lex
	{
		DeclareException(LexerException, NatsuLib::natException, "Exception generated by lexer."_nv);

		class Lexer
			: public NatsuLib::nonmovable
		{
		public:
			explicit Lexer(nStrView buffer, Preprocessor& preprocessor);

			nBool Lex(Token::Token& result);

		private:
			using Iterator = nStrView::const_iterator;
			using CharType = nStrView::CharType;

			Preprocessor& m_Preprocessor;

			nStrView m_Buffer;
			// 当前处理的指针，指向下一次被处理的字符
			Iterator m_Current;

			nBool skipWhitespace(Token::Token& result, Iterator cur);
			nBool skipLineComment(Token::Token& result, Iterator cur);
			nBool skipBlockComment(Token::Token& result, Iterator cur);

			nBool lexNumericLiteral(Token::Token& result, Iterator cur);
			nBool lexIdentifier(Token::Token& result, Iterator cur);
			nBool lexCharLiteral(Token::Token& result, Iterator cur);
			nBool lexStringLiteral(Token::Token& result, Iterator cur);
		};
	}
}
