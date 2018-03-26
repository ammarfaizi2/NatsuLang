﻿#pragma once
#include <natMisc.h>
#include <natRefObj.h>
#include <unordered_set>
#include "Basic/SourceLocation.h"
#include "Basic/Identifier.h"
#include "AST/Declaration.h"
#include "AST/Type.h"
#include "AST/OperationTypes.h"
#include "CompilerAction.h"
#include "AST/Statement.h"

namespace NatsuLang
{
	namespace Expression
	{
		enum class UnaryOperationType;
	}

	namespace Declaration
	{
		class Declarator;
		using DeclaratorPtr = NatsuLib::natRefPointer<Declarator>;
	}

	namespace Diag
	{
		class DiagnosticsEngine;
	}

	namespace Identifier
	{
		class IdentifierInfo;
		using IdPtr = NatsuLib::natRefPointer<IdentifierInfo>;
	}

	namespace Syntax
	{
		class ResolveContext;
	}

	class Preprocessor;
	class ASTContext;
	struct ASTConsumer;
	class NestedNameSpecifier;
	class SourceManager;
}

namespace NatsuLang::Semantic
{
	enum class ScopeFlags : nuShort;
	class LookupResult;
	class Scope;

	class Sema
		: NatsuLib::nonmovable
	{
	public:
		enum class ExpressionEvaluationContext
		{
			Unevaluated,
			DiscardedStatement,
			ConstantEvaluated,
			PotentiallyEvaluated,
			PotentiallyEvaluatedIfUsed
		};

		enum class LookupNameType
		{
			LookupOrdinaryName,
			LookupTagName,
			LookupLabel,
			LookupMemberName,
			LookupModuleName,
			LookupAnyName
		};

		enum class Phase
		{
			Phase1,
			// 分析顶层及类内的声明符，获得名称，保留声明符，在声明符中缓存记号以便延迟分析类型及初始化器等
			Phase2 // 解析顶层的 CompilerAction 及对各保留的声明符进行分析，主要工作为解析类型为真实类型及解析初始化器等
		};

		using ModulePathType = std::vector<std::pair<NatsuLib::natRefPointer<Identifier::IdentifierInfo>, SourceLocation>>;

		Sema(Preprocessor& preprocessor, ASTContext& astContext, NatsuLib::natRefPointer<ASTConsumer> astConsumer);
		~Sema();

		Preprocessor& GetPreprocessor() const noexcept
		{
			return m_Preprocessor;
		}

		ASTContext& GetASTContext() const noexcept
		{
			return m_Context;
		}

		NatsuLib::natRefPointer<ASTConsumer> const& GetASTConsumer() const noexcept
		{
			return m_Consumer;
		}

		Diag::DiagnosticsEngine& GetDiagnosticsEngine() const noexcept
		{
			return m_Diag;
		}

		SourceManager& GetSourceManager() const noexcept
		{
			return m_SourceManager;
		}

		NatsuLib::natRefPointer<Scope> GetCurrentScope() const noexcept
		{
			return m_CurrentScope;
		}

		// 仅在解析声明符时恢复上下文时使用，不应该在其他地方使用
		void SetCurrentScope(NatsuLib::natRefPointer<Scope> value) noexcept
		{
			m_CurrentScope = std::move(value);
		}

		Phase GetCurrentPhase() const noexcept
		{
			return m_CurrentPhase;
		}

		void SetCurrentPhase(Phase value) noexcept
		{
			m_CurrentPhase = value;
		}

		std::vector<Declaration::DeclaratorPtr> const& GetCachedDeclarators() const noexcept;
		std::vector<Declaration::DeclaratorPtr> GetAndClearCachedDeclarators() noexcept;
		void ClearCachedDeclarations() noexcept;

		void ActOnPhaseDiverted();

		void PushDeclContext(NatsuLib::natRefPointer<Scope> const& scope, Declaration::DeclContext* dc);
		void PopDeclContext();

		// 仅在解析声明符时恢复上下文时使用，不应该在其他地方使用
		void SetDeclContext(Declaration::DeclPtr dc) noexcept;
		Declaration::DeclPtr GetDeclContext() const noexcept;

		void PushScope(ScopeFlags flags);
		void PopScope();

		void PushOnScopeChains(NatsuLib::natRefPointer<Declaration::NamedDecl> decl,
		                       NatsuLib::natRefPointer<Scope> const& scope, nBool addToContext = true);
		void RemoveFromScopeChains(NatsuLib::natRefPointer<Declaration::NamedDecl> const& decl,
		                           NatsuLib::natRefPointer<Scope> const& scope, nBool removeFromContext = true);

		NatsuLib::natRefPointer<CompilerActionNamespace> GetTopLevelActionNamespace() noexcept;

		void ActOnTranslationUnitScope(NatsuLib::natRefPointer<Scope> scope);

		NatsuLib::natRefPointer<Declaration::ModuleDecl> ActOnModuleDecl(NatsuLib::natRefPointer<Scope> scope,
		                                                                 SourceLocation startLoc, Identifier::IdPtr name);
		void ActOnStartModule(NatsuLib::natRefPointer<Scope> const& scope,
		                      NatsuLib::natRefPointer<Declaration::ModuleDecl> const& moduleDecl);
		void ActOnFinishModule();
		NatsuLib::natRefPointer<Declaration::Decl> ActOnModuleImport(SourceLocation startLoc, SourceLocation importLoc,
		                                                             ModulePathType const& path);

		Type::TypePtr LookupTypeName(NatsuLib::natRefPointer<Identifier::IdentifierInfo> const& id, SourceLocation nameLoc,
		                             NatsuLib::natRefPointer<Scope> scope,
		                             NatsuLib::natRefPointer<NestedNameSpecifier> const& nns);

		NatsuLib::natRefPointer<Declaration::AliasDecl> LookupAliasName(NatsuLib::natRefPointer<Identifier::IdentifierInfo> const& id, SourceLocation nameLoc,
			NatsuLib::natRefPointer<Scope> scope,
			NatsuLib::natRefPointer<NestedNameSpecifier> const& nns);

		Type::TypePtr BuildFunctionType(Type::TypePtr retType,
		                                NatsuLib::Linq<NatsuLib::Valued<Type::TypePtr>> const& paramType);
		Type::TypePtr CreateUnresolvedType(std::vector<Lex::Token> tokens);

		Declaration::DeclPtr ActOnStartOfFunctionDef(NatsuLib::natRefPointer<Scope> const& scope,
		                                             Declaration::DeclaratorPtr declarator);
		Declaration::DeclPtr ActOnStartOfFunctionDef(NatsuLib::natRefPointer<Scope> const& scope, Declaration::DeclPtr decl);
		Declaration::DeclPtr ActOnFinishFunctionBody(Declaration::DeclPtr decl, Statement::StmtPtr body);

		///	@brief	获取当前正在分析的函数声明
		///	@return	若当前并未在分析函数，则返回 nullptr，否则返回最近的一个函数声明
		NatsuLib::natRefPointer<Declaration::FunctionDecl> GetParsingFunction() const noexcept;

		Declaration::DeclPtr ActOnTag(NatsuLib::natRefPointer<Scope> const& scope, Type::TagType::TagTypeClass tagTypeClass,
		                              SourceLocation kwLoc, Specifier::Access accessSpecifier, Identifier::IdPtr name,
		                              SourceLocation nameLoc, Type::TypePtr underlyingType = nullptr);
		void ActOnTagStartDefinition(NatsuLib::natRefPointer<Scope> const& scope,
		                             NatsuLib::natRefPointer<Declaration::TagDecl> const& tagDecl);
		void ActOnTagFinishDefinition();

		nBool LookupName(LookupResult& result, NatsuLib::natRefPointer<Scope> scope) const;
		nBool LookupQualifiedName(LookupResult& result, Declaration::DeclContext* context) const;
		nBool LookupNestedName(LookupResult& result, NatsuLib::natRefPointer<Scope> scope,
		                       NatsuLib::natRefPointer<NestedNameSpecifier> const& nns);

		nBool LookupConstructors(LookupResult& result, NatsuLib::natRefPointer<Declaration::ClassDecl> const& classDecl);

		NatsuLib::natRefPointer<Declaration::LabelDecl> LookupOrCreateLabel(Identifier::IdPtr id, SourceLocation loc);

		Type::TypePtr ActOnArrayType(Type::TypePtr elementType, std::size_t size);
		Type::TypePtr ActOnPointerType(NatsuLib::natRefPointer<Scope> const& scope, Type::TypePtr pointeeType);

		NatsuLib::natRefPointer<Declaration::ParmVarDecl> ActOnParamDeclarator(
			NatsuLib::natRefPointer<Scope> const& scope, Declaration::DeclaratorPtr decl);
		NatsuLib::natRefPointer<Declaration::VarDecl> ActOnVariableDeclarator(NatsuLib::natRefPointer<Scope> const& scope,
		                                                                      Declaration::DeclaratorPtr decl,
		                                                                      Declaration::DeclContext* dc);
		NatsuLib::natRefPointer<Declaration::FieldDecl> ActOnFieldDeclarator(NatsuLib::natRefPointer<Scope> const& scope,
		                                                                     Declaration::DeclaratorPtr decl,
		                                                                     Declaration::DeclContext* dc);
		NatsuLib::natRefPointer<Declaration::FunctionDecl> ActOnFunctionDeclarator(
			NatsuLib::natRefPointer<Scope> const& scope, Declaration::DeclaratorPtr decl, Declaration::DeclContext* dc,
			Identifier::IdPtr asId = nullptr);
		NatsuLib::natRefPointer<Declaration::UnresolvedDecl> ActOnUnresolvedDeclarator(
			NatsuLib::natRefPointer<Scope> const& scope, Declaration::DeclaratorPtr decl, Declaration::DeclContext* dc);
		NatsuLib::natRefPointer<Declaration::UnresolvedDecl> ActOnCompilerActionIdentifierArgument(Identifier::IdPtr id);
		NatsuLib::natRefPointer<Declaration::NamedDecl> HandleDeclarator(NatsuLib::natRefPointer<Scope> scope,
		                                                                 Declaration::DeclaratorPtr decl,
		                                                                 Declaration::DeclPtr const& oldUnresolvedDeclPtr =
			                                                                 nullptr);

		NatsuLib::natRefPointer<Declaration::AliasDecl> ActOnAliasDeclaration(NatsuLib::natRefPointer<Scope> scope, SourceLocation loc, Identifier::IdPtr id, ASTNodePtr aliasAsAst);

		Statement::StmtPtr ActOnNullStmt(SourceLocation loc = {});
		Statement::StmtPtr ActOnDeclStmt(std::vector<Declaration::DeclPtr> decls, SourceLocation start, SourceLocation end);
		Statement::StmtPtr ActOnLabelStmt(SourceLocation labelLoc, NatsuLib::natRefPointer<Declaration::LabelDecl> labelDecl,
		                                  SourceLocation colonLoc, Statement::StmtPtr subStmt);
		Statement::StmtPtr ActOnCompoundStmt(std::vector<Statement::StmtPtr> stmtVec, SourceLocation begin,
		                                     SourceLocation end);
		Statement::StmtPtr ActOnIfStmt(SourceLocation ifLoc, Expression::ExprPtr condExpr, Statement::StmtPtr thenStmt,
		                               SourceLocation elseLoc, Statement::StmtPtr elseStmt);
		Statement::StmtPtr ActOnWhileStmt(SourceLocation loc, Expression::ExprPtr cond, Statement::StmtPtr body);
		Statement::StmtPtr ActOnForStmt(SourceLocation forLoc, SourceLocation leftParenLoc, Statement::StmtPtr init,
		                                Expression::ExprPtr cond, Expression::ExprPtr third, SourceLocation rightParenLoc,
		                                Statement::StmtPtr body);

		Statement::StmtPtr ActOnContinueStmt(SourceLocation loc, NatsuLib::natRefPointer<Scope> const& scope);
		Statement::StmtPtr ActOnBreakStmt(SourceLocation loc, NatsuLib::natRefPointer<Scope> const& scope);
		Statement::StmtPtr ActOnReturnStmt(SourceLocation loc, Expression::ExprPtr returnedExpr,
		                                   NatsuLib::natRefPointer<Scope> const& scope);

		Statement::StmtPtr ActOnExprStmt(Expression::ExprPtr expr);

		Expression::ExprPtr ActOnBooleanLiteral(Lex::Token const& token) const;
		Expression::ExprPtr ActOnNumericLiteral(Lex::Token const& token) const;
		Expression::ExprPtr ActOnCharLiteral(Lex::Token const& token) const;
		Expression::ExprPtr ActOnStringLiteral(Lex::Token const& token);

		Expression::ExprPtr ActOnConditionExpr(Expression::ExprPtr expr);

		Expression::ExprPtr ActOnThrow(NatsuLib::natRefPointer<Scope> const& scope, SourceLocation loc,
		                               Expression::ExprPtr expr);

		Expression::ExprPtr ActOnInitExpr(Type::TypePtr initType, SourceLocation leftBraceLoc,
		                                  std::vector<Expression::ExprPtr> initExprs, SourceLocation rightBraceLoc);

		Expression::ExprPtr ActOnIdExpr(NatsuLib::natRefPointer<Scope> const& scope,
		                                NatsuLib::natRefPointer<NestedNameSpecifier> const& nns, Identifier::IdPtr id,
		                                nBool hasTraillingLParen,
		                                NatsuLib::natRefPointer<Syntax::ResolveContext> const& resolveContext = nullptr);
		Expression::ExprPtr ActOnThis(SourceLocation loc);
		Expression::ExprPtr ActOnAsTypeExpr(NatsuLib::natRefPointer<Scope> const& scope, Expression::ExprPtr exprToCast,
		                                    Type::TypePtr type, SourceLocation loc);
		Expression::ExprPtr ActOnArraySubscriptExpr(NatsuLib::natRefPointer<Scope> const& scope, Expression::ExprPtr base,
		                                            SourceLocation lloc, Expression::ExprPtr index, SourceLocation rloc);
		Expression::ExprPtr ActOnCallExpr(NatsuLib::natRefPointer<Scope> const& scope, Expression::ExprPtr func,
		                                  SourceLocation lloc, NatsuLib::Linq<NatsuLib::Valued<Expression::ExprPtr>> argExprs,
		                                  SourceLocation rloc);
		Expression::ExprPtr ActOnMemberAccessExpr(NatsuLib::natRefPointer<Scope> const& scope, Expression::ExprPtr base,
		                                          SourceLocation periodLoc,
		                                          NatsuLib::natRefPointer<NestedNameSpecifier> const& nns,
		                                          Identifier::IdPtr id);
		Expression::ExprPtr ActOnUnaryOp(NatsuLib::natRefPointer<Scope> const& scope, SourceLocation loc,
		                                 Lex::TokenType tokenType, Expression::ExprPtr operand);
		Expression::ExprPtr ActOnPostfixUnaryOp(NatsuLib::natRefPointer<Scope> const& scope, SourceLocation loc,
		                                        Lex::TokenType tokenType, Expression::ExprPtr operand);
		Expression::ExprPtr ActOnBinaryOp(NatsuLib::natRefPointer<Scope> const& scope, SourceLocation loc,
		                                  Lex::TokenType tokenType, Expression::ExprPtr leftOperand,
		                                  Expression::ExprPtr rightOperand);
		Expression::ExprPtr BuildBuiltinBinaryOp(SourceLocation loc, Expression::BinaryOperationType binOpType,
		                                         Expression::ExprPtr leftOperand, Expression::ExprPtr rightOperand);
		// 条件操作符不可重载所以不需要scope信息
		Expression::ExprPtr ActOnConditionalOp(SourceLocation questionLoc, SourceLocation colonLoc,
		                                       Expression::ExprPtr condExpr, Expression::ExprPtr leftExpr,
		                                       Expression::ExprPtr rightExpr);

		Expression::ExprPtr BuildDeclarationNameExpr(NatsuLib::natRefPointer<NestedNameSpecifier> const& nns,
		                                             Identifier::IdPtr id,
		                                             NatsuLib::natRefPointer<Declaration::NamedDecl> decl);
		Expression::ExprPtr BuildDeclRefExpr(NatsuLib::natRefPointer<Declaration::ValueDecl> decl, Type::TypePtr type,
		                                     Identifier::IdPtr id, NatsuLib::natRefPointer<NestedNameSpecifier> const& nns);
		Expression::ExprPtr BuildMemberReferenceExpr(NatsuLib::natRefPointer<Scope> const& scope,
		                                             Expression::ExprPtr baseExpr, Type::TypePtr baseType,
		                                             SourceLocation opLoc,
		                                             NatsuLib::natRefPointer<NestedNameSpecifier> const& nns,
		                                             LookupResult& r);
		Expression::ExprPtr BuildFieldReferenceExpr(Expression::ExprPtr baseExpr, SourceLocation opLoc,
		                                            NatsuLib::natRefPointer<NestedNameSpecifier> const& nns,
		                                            NatsuLib::natRefPointer<Declaration::FieldDecl> field,
		                                            Identifier::IdPtr id);

		Expression::ExprPtr BuildConstructExpr(NatsuLib::natRefPointer<Type::ClassType> const& classType,
		                                       SourceLocation leftBraceLoc, std::vector<Expression::ExprPtr> initExprs,
		                                       SourceLocation rightBraceLoc);

		Expression::ExprPtr CreateBuiltinUnaryOp(SourceLocation opLoc, Expression::UnaryOperationType opCode,
		                                         Expression::ExprPtr operand);

		Type::TypePtr GetBuiltinBinaryOpType(Expression::ExprPtr& leftOperand, Expression::ExprPtr& rightOperand);
		Type::TypePtr UsualArithmeticConversions(Expression::ExprPtr& leftOperand, Expression::ExprPtr& rightOperand);

		Expression::ExprPtr ImpCastExprToType(Expression::ExprPtr expr, Type::TypePtr type, Expression::CastType castType);

		nBool CheckFunctionReturn(Statement::StmtEnumerable const& funcBody);

	private:
		static constexpr nStrView::CharType ConstructorName[] = ".Ctor";
		static constexpr nStrView::CharType DestructorName[] = ".Dtor";

		Preprocessor& m_Preprocessor;
		ASTContext& m_Context;
		NatsuLib::natRefPointer<ASTConsumer> m_Consumer;
		Diag::DiagnosticsEngine& m_Diag;
		SourceManager& m_SourceManager;

		Phase m_CurrentPhase;
		std::vector<Declaration::DeclaratorPtr> m_Declarators;

		NatsuLib::natRefPointer<Scope> m_TranslationUnitScope;
		NatsuLib::natRefPointer<Scope> m_CurrentScope;

		// m_CurrentDeclContext必须为nullptr或者可以转换到DeclContext*，不保存DeclContext*是为了保留对Decl的强引用
		Declaration::DeclPtr m_CurrentDeclContext;

		NatsuLib::natRefPointer<CompilerActionNamespace> m_TopLevelActionNamespace;

		void prewarming();

		Expression::CastType getCastType(Expression::ExprPtr const& operand, Type::TypePtr toType, nBool isImplicit);

		Type::TypePtr handleIntegerConversion(Expression::ExprPtr& leftOperand, Type::TypePtr leftOperandType,
		                                      Expression::ExprPtr& rightOperand, Type::TypePtr rightOperandType);
		Type::TypePtr handleFloatConversion(Expression::ExprPtr& leftOperand, Type::TypePtr leftOperandType,
		                                    Expression::ExprPtr& rightOperand, Type::TypePtr rightOperandType);
	};

	class LookupResult
	{
	public:
		enum class LookupResultType
		{
			NotFound,
			Found,
			FoundOverloaded,
			Ambiguous
		};

		enum class AmbiguousType
		{
			// TODO
		};

		LookupResult(Sema& sema, Identifier::IdPtr id, SourceLocation loc, Sema::LookupNameType lookupNameType);

		Identifier::IdPtr GetLookupId() const noexcept
		{
			return m_LookupId;
		}

		Sema::LookupNameType GetLookupType() const noexcept
		{
			return m_LookupNameType;
		}

		NatsuLib::Linq<NatsuLib::Valued<NatsuLib::natRefPointer<Declaration::NamedDecl>>> GetDecls() const noexcept;

		std::size_t GetDeclSize() const noexcept
		{
			return m_Decls.size();
		}

		nBool IsEmpty() const noexcept
		{
			return m_Decls.empty();
		}

		void AddDecl(NatsuLib::natRefPointer<Declaration::NamedDecl> decl);
		void AddDecl(NatsuLib::Linq<NatsuLib::Valued<NatsuLib::natRefPointer<Declaration::NamedDecl>>> decls);

		void ResolveResultType() noexcept;

		LookupResultType GetResultType() const noexcept
		{
			return m_Result;
		}

		AmbiguousType GetAmbiguousType() const noexcept
		{
			return m_AmbiguousType;
		}

		Type::TypePtr GetBaseObjectType() const noexcept
		{
			return m_BaseObjectType;
		}

		void SetBaseObjectType(Type::TypePtr value) noexcept
		{
			m_BaseObjectType = std::move(value);
		}

	private:
		// 查找参数
		Sema& m_Sema;
		Identifier::IdPtr m_LookupId;
		SourceLocation m_LookupLoc;
		Sema::LookupNameType m_LookupNameType;
		Declaration::IdentifierNamespace m_IDNS;

		// 查找结果
		LookupResultType m_Result;
		AmbiguousType m_AmbiguousType;
		std::unordered_set<NatsuLib::natRefPointer<Declaration::NamedDecl>> m_Decls;
		Type::TypePtr m_BaseObjectType;
	};
}
