#pragma once
#include <variant>
#include <optional>
#include <natRefObj.h>

namespace NatsuLang::Identifier
{
	class IdentifierInfo;
}

namespace NatsuLang::Token
{
	enum class TokenType
	{
#define TOK(X) X,
#include "TokenDef.h"
		TokenNumber,
	};

	constexpr bool IsLiteral(TokenType tokenType) noexcept
	{
		return tokenType == TokenType::NumericLiteral || tokenType == TokenType::CharLiteral || tokenType == TokenType::StringLiteral;
	}

	const char* GetTokenName(TokenType tokenType) noexcept;
	const char* GetPunctuatorName(TokenType tokenType) noexcept;
	const char* GetKeywordName(TokenType tokenType) noexcept;

	class Token final
	{
	public:
		constexpr explicit Token(TokenType tokenType = TokenType::Unknown) noexcept
			: m_Type{ tokenType }
		{
		}

		void Reset() noexcept
		{
			m_Type = TokenType::Unknown;
		}

		TokenType GetType() const noexcept
		{
			return m_Type;
		}

		void SetType(TokenType tokenType) noexcept
		{
			m_Type = tokenType;
		}

		nuInt GetLength() const noexcept;

		void SetLength(nuInt value)
		{
			m_Info.emplace<2>(value);
		}

		NatsuLib::natRefPointer<Identifier::IdentifierInfo> GetIdentifierInfo() noexcept
		{
			return m_Info.index() == 0 ? std::get<0>(m_Info) : nullptr;
		}

		void SetIdentifierInfo(NatsuLib::natRefPointer<Identifier::IdentifierInfo> identifierInfo)
		{
			m_Info.emplace<0>(std::move(identifierInfo));
		}

		std::optional<nStrView> GetLiteralContent() noexcept
		{
			if (m_Info.index() == 1)
			{
				return std::get<1>(m_Info);
			}

			return {};
		}

		void SetLiteralContent(nStrView const& str)
		{
			m_Info.emplace<1>(str);
		}

	private:
		TokenType m_Type;
		std::variant<NatsuLib::natRefPointer<Identifier::IdentifierInfo>, nStrView, nuInt> m_Info;
	};
}
