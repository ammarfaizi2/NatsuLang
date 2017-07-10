#include <Lex/Lexer.h>
#include <Lex/Preprocessor.h>
#include <Basic/Diagnostic.h>
#include <Basic/FileManager.h>
#include <Basic/SourceManager.h>
#include <natConsole.h>

using namespace NatsuLib;
using namespace NatsuLang;
using namespace Lex;
using namespace Diag;

class IDMap
	: public natRefObjImpl<IDMap, Misc::TextProvider<DiagnosticsEngine::DiagID>>
{
public:
	nString GetText(DiagnosticsEngine::DiagID id) override
	{
		switch (id)
		{
		case DiagnosticsEngine::DiagID::ErrUndefinedIdentifier:
			return u8"Undefined identifier {0}.";
		case DiagnosticsEngine::DiagID::ErrMultiCharInLiteral:
			return u8"Multiple char in literal.";
		case DiagnosticsEngine::DiagID::ErrUnexpectEOF:
			return u8"Unexpect eof.";
		default:
			return u8"(Error text)";
		}
	}
};

class OutputDiagConsumer
	: public natRefObjImpl<OutputDiagConsumer, DiagnosticConsumer>
{
public:
	explicit OutputDiagConsumer(natConsole& console)
		: m_Console{ console }
	{
	}

	void HandleDiagnostic(DiagnosticsEngine::Level /*level*/, DiagnosticsEngine::Diagnostic const& diag) override
	{
		m_Console.WriteLineErr(diag.GetDiagMessage());
	}

private:
	natConsole& m_Console;
};

int main()
{
	natConsole console;
	DiagnosticsEngine diag{ make_ref<IDMap>(), make_ref<OutputDiagConsumer>(console) };
	FileManager fileManager{};
	SourceManager sourceManager{ diag, fileManager };
	Preprocessor pp{ diag, sourceManager };
	Lexer lexer{ "abc", pp };

	Token::Token token;
	lexer.Lex(token);

	system("pause");
}
