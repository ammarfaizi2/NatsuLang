#pragma once
#include "TypeBase.h"
#include <natLinq.h>

namespace NatsuLang::Declaration
{
	class TagDecl;
	class RecordDecl;
	class EnumDecl;
}

namespace NatsuLang::Expression
{
	class Expr;
}

namespace NatsuLang::Type
{
	class ParenType
		: public Type
	{
	public:
		explicit ParenType(TypePtr innerType)
			: Type{ Paren }, m_InnerType{ std::move(innerType) }
		{
		}

		~ParenType();

		TypePtr GetInnerType() const noexcept
		{
			return m_InnerType;
		}

		std::size_t GetHashCode() const noexcept override;
		nBool EqualTo(TypePtr const& other) const noexcept override;

	private:
		const TypePtr m_InnerType;
	};

	class ArrayType
		: public Type
	{
	public:
		ArrayType(TypePtr elementType, std::size_t arraySize)
			: Type{ Array }, m_ElementType{ std::move(elementType) }, m_ArraySize{ arraySize }
		{
		}

		~ArrayType();

		TypePtr GetElementType() const noexcept
		{
			return m_ElementType;
		}

		std::size_t GetSize() const noexcept
		{
			return m_ArraySize;
		}

		std::size_t GetHashCode() const noexcept override;
		nBool EqualTo(TypePtr const& other) const noexcept override;

	private:
		TypePtr m_ElementType;
		std::size_t m_ArraySize;
	};

	class FunctionType
		: Type
	{
	public:
		FunctionType(NatsuLib::Linq<TypePtr> const& params, TypePtr resultType)
			: Type{ Function }, m_ParameterTypes{ params.begin(), params.end() }, m_ResultType{ std::move(resultType) }
		{
		}

		~FunctionType();

		TypePtr GetResultType() const noexcept
		{
			return m_ResultType;
		}

		NatsuLib::Linq<TypePtr> GetParameterTypes() const noexcept;
		std::size_t GetParameterCount() const noexcept;

		std::size_t GetHashCode() const noexcept override;
		nBool EqualTo(TypePtr const& other) const noexcept override;

	private:
		std::vector<TypePtr> m_ParameterTypes;
		TypePtr m_ResultType;
	};

	class TypeOfType
		: public Type
	{
	public:
		TypeOfType(NatsuLib::natRefPointer<Expression::Expr> expr, TypePtr underlyingType)
			: Type{ TypeOf }, m_Expr{ std::move(expr) }, m_UnderlyingType{ std::move(underlyingType) }
		{
		}

		~TypeOfType();

		NatsuLib::natRefPointer<Expression::Expr> GetExpr() const noexcept
		{
			return m_Expr;
		}

		TypePtr GetUnderlyingType() const noexcept
		{
			return m_UnderlyingType;
		}

		std::size_t GetHashCode() const noexcept override;
		nBool EqualTo(TypePtr const& other) const noexcept override;

	private:
		NatsuLib::natRefPointer<Expression::Expr> m_Expr;
		TypePtr m_UnderlyingType;
	};

	class TagType
		: public Type
	{
	protected:
		TagType(TypeClass typeClass, NatsuLib::natRefPointer<Declaration::TagDecl> decl)
			: Type{ typeClass }, m_Decl{ std::move(decl) }
		{
		}

	public:
		enum class TagTypeClass
		{
			Class,
			Enum
		};

		~TagType();

		NatsuLib::natRefPointer<Declaration::TagDecl> GetDecl() const noexcept
		{
			return m_Decl;
		}

		std::size_t GetHashCode() const noexcept override;
		nBool EqualTo(TypePtr const& other) const noexcept override;

	private:
		NatsuLib::natRefPointer<Declaration::TagDecl> m_Decl;
	};

	class RecordType
		: public TagType
	{
	public:
		explicit RecordType(NatsuLib::natRefPointer<Declaration::RecordDecl> recordDecl)
			: TagType{ Record, recordDecl }
		{
		}

		RecordType(TypeClass typeClass, NatsuLib::natRefPointer<Declaration::RecordDecl> recordDecl)
			: TagType{ typeClass, recordDecl }
		{
		}

		~RecordType();

		nBool EqualTo(TypePtr const& other) const noexcept override;
	};

	class EnumType
		: public TagType
	{
	public:
		explicit EnumType(NatsuLib::natRefPointer<Declaration::EnumDecl> decl)
			: TagType{ Enum, decl }
		{
		}

		~EnumType();

		nBool EqualTo(TypePtr const& other) const noexcept override;
	};

	class DeducedType
		: public Type
	{
	public:
		DeducedType(TypeClass typeClass, TypePtr deducedAsType)
			: Type{ typeClass }, m_DeducedAsType{ std::move(deducedAsType) }
		{
		}

		~DeducedType();

		TypePtr GetDeducedAsType() const noexcept
		{
			return m_DeducedAsType;
		}

		std::size_t GetHashCode() const noexcept override;
		nBool EqualTo(TypePtr const& other) const noexcept override;

	private:
		TypePtr m_DeducedAsType;
	};

	class AutoType
		: public DeducedType
	{
	public:
		explicit AutoType(TypePtr deducedAsType)
			: DeducedType{ Auto, std::move(deducedAsType) }
		{
		}

		~AutoType();

		nBool EqualTo(TypePtr const& other) const noexcept override;
	};
}
