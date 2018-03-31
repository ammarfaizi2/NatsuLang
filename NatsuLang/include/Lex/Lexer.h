﻿#pragma once
#include <natException.h>
#include "Basic/Token.h"

namespace NatsuLang
{
	class Preprocessor;

	namespace Lex
	{
		DeclareException(LexerException, NatsuLib::natException, "Exception generated by lexer.");

		class Lexer
			: public NatsuLib::natRefObjImpl<Lexer>, public NatsuLib::nonmovable
		{
		public:
			explicit Lexer(nStrView buffer, Preprocessor& preprocessor);

			nBool Lex(Token& result);

			nuInt GetFileID() const noexcept;
			void SetFileID(nuInt value) noexcept;

			void EnableCodeCompletion(nBool value) noexcept
			{
				m_CodeCompletionEnabled = value;
			}

			nBool IsCodeCompletionEnabled() const noexcept
			{
				return m_CodeCompletionEnabled;
			}

		private:
			using Iterator = nStrView::const_iterator;
			using CharType = nStrView::CharType;

			Preprocessor& m_Preprocessor;

			nBool m_CodeCompletionEnabled;

			SourceLocation m_CurLoc;
			nStrView m_Buffer;
			// 当前处理的指针，指向下一次被处理的字符
			Iterator m_Current;

			nBool skipWhitespace(Token& result, Iterator cur);
			nBool skipLineComment(Token& result, Iterator cur);
			nBool skipBlockComment(Token& result, Iterator cur);

			nBool lexNumericLiteral(Token& result, Iterator cur);
			nBool lexIdentifier(Token& result, Iterator cur);
			nBool lexCharLiteral(Token& result, Iterator cur);
			nBool lexStringLiteral(Token& result, Iterator cur);

		public:
			class Memento
			{
				friend class Lexer;

				constexpr Memento(SourceLocation curLoc, Iterator current) noexcept
					: m_CurLoc{ curLoc }, m_Current{ current }
				{
				}

				SourceLocation m_CurLoc;
				Iterator m_Current;
			};

			Memento SaveToMemento() const noexcept;
			void RestoreFromMemento(Memento memento) noexcept;
		};
	}
}
