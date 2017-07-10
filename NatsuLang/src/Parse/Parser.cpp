#include "Parse/Parser.h"
#include "Sema/Sema.h"

using namespace NatsuLang::Syntax;
using namespace NatsuLang::Token;

Parser::Parser(Preprocessor& preprocessor, Semantic::Sema& sema)
	: m_Preprocessor{ preprocessor }, m_DiagnosticsEngine{ preprocessor.GetDiag() }, m_Sema{ sema }, m_ParenCount{}, m_BracketCount{}, m_BraceCount{}
{
	m_CurrentToken.SetType(TokenType::Eof);
}

Parser::~Parser()
{
}

NatsuLang::Preprocessor& Parser::GetPreprocessor() const noexcept
{
	return m_Preprocessor;
}

NatsuLang::Diag::DiagnosticsEngine& Parser::GetDiagnosticsEngine() const noexcept
{
	return m_DiagnosticsEngine;
}

nBool Parser::ParseTopLevelDecl(std::unordered_set<NatsuLib::natRefPointer<Declaration::Decl>>& decls)
{
	if (m_CurrentToken.Is(TokenType::Eof))
	{
		ConsumeToken();
	}

	switch (m_CurrentToken.GetType())
	{
	case TokenType::Kw_import:
		break;
	default:
		break;
	}
}

std::vector<NatsuLib::natRefPointer<NatsuLang::Declaration::Decl>> Parser::ParseModuleImport()
{
	assert(m_CurrentToken.Is(Token::TokenType::Kw_import));
	ConsumeToken();
	auto startLoc = m_PrevTokenLocation;

	std::vector<std::pair<NatsuLib::natRefPointer<Identifier::IdentifierInfo>, SourceLocation>> path;
	if (!ParseModuleName(path))
	{
		return {};
	}
}

nBool Parser::ParseModuleName(std::vector<std::pair<NatsuLib::natRefPointer<Identifier::IdentifierInfo>, SourceLocation>>& path)
{
	while (true)
	{
		if (!m_CurrentToken.Is(TokenType::Identifier))
		{
			m_DiagnosticsEngine.Report(Diag::DiagnosticsEngine::DiagID::ErrModuleExpectedIdentifier, m_CurrentToken.GetLocation());
			SkipUntil({ TokenType::Semi });
			return false;
		}

		path.emplace_back(m_CurrentToken.GetIdentifierInfo(), m_CurrentToken.GetLocation());
		ConsumeToken();

		if (!m_CurrentToken.Is(TokenType::Period))
		{
			return true;
		}

		ConsumeToken();
	}
}

nBool Parser::SkipUntil(std::initializer_list<Token::TokenType> list, nBool dontConsume)
{
	// 特例，如果调用者只是想跳到文件结尾，我们不需要再另外判断其他信息
	if (list.size() == 1 && *list.begin() == TokenType::Eof)
	{
		while (!m_CurrentToken.Is(TokenType::Eof))
		{
			ConsumeAnyToken();
		}
		return true;
	}

	while (true)
	{
		const auto currentType = m_CurrentToken.GetType();
		for (auto type : list)
		{
			if (currentType == type)
			{
				if (!dontConsume)
				{
					ConsumeToken();
				}

				return true;
			}
		}

		if (currentType == TokenType::Eof)
		{
			return false;
		}

		ConsumeAnyToken();
	}
}
