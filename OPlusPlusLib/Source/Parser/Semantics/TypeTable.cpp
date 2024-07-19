#include "TypeTable.h"

#include "../../Utils.hpp"

#include <assert.h>
#include <iostream>

namespace O
{
	Type::Type(const std::string& _typeName, TypeId _id, TypeKind _kind, std::vector<TypeId> _typeArguments)
		: typeName(_typeName), id(_id), kind(_kind), typeArguments(_typeArguments)
	{
	}

	std::string Type::GetName(const TypeTable* table) const
	{
		return GetName((TypeTable*)table);
	}
	std::string Type::GetName(TypeTable* table) const
	{
		// TODO: Cache the result if bad performance. 
		// I imagine it will be slow for very nested generic types
		if (kind == TypeKind::Primitive)
			return typeName;

		if (kind == TypeKind::Class)
			return typeName;

		std::stringstream name;
		/*name << TypeEntryTypeToString(kind) << "<";

		for (const auto& [typeId, type] : table->GetTypes())
		{
			name << 
		}*/
			return TypeEntryTypeToString(kind) + "<" + Join(typeArguments, std::string(", "), [&table](TypeId id) {
				return table->Lookup(id)->GetName(table); }) 
			+ ">";
	}

	TypeTable::TypeTable()
	{
		m_TableType = TypeTableType::Local;
		m_UpwardTypeTable = nullptr;
	}

	TypeTable::TypeTable(TypeTableType tableType, TypeTable* upwardTypeTable)
	{
		m_TableType = tableType;
		m_UpwardTypeTable = upwardTypeTable;

		if (tableType == TypeTableType::Global)
		{
			// Because the type table is used during parsing, we have to reset the next free id
			m_NextFreeTypeId = 0;
			InsertPrimitiveTypes();
		}
	}

	bool TypeTable::HasType(const std::string& typeName)
	{
		// First look in the current table
		if (m_Typenames.count(typeName) != 0)
			return true;

		// Otherwise look upward
		if (m_UpwardTypeTable)
			return m_UpwardTypeTable->HasType(typeName);

		assert(m_TableType == TypeTableType::Global);
		return false;
	}

	bool TypeTable::HasType(TypeId typeId)
	{
		// First look in the current table
		if (m_Types.count(typeId) == 1)
			return true;

		// Otherwise look upward
		if (m_UpwardTypeTable)
			return m_UpwardTypeTable->HasType(typeId);

		assert(m_TableType == TypeTableType::Global);
		return false;
	}

	bool TypeTable::HasCompleteType(const std::string& typeName)
	{
		// First look in the current table
		if (m_Typenames.count(typeName) != 0)
			return m_Types[m_Typenames[typeName]]->kind != TypeKind::Incomplete;

		// Otherwise look upward
		if (m_UpwardTypeTable)
			return m_UpwardTypeTable->HasCompleteType(typeName);

		assert(m_TableType == TypeTableType::Global);
		return false;
	}

	bool TypeTable::HasCompleteType(TypeId typeId)
	{
		// First look in the current table
		if (m_Types.count(typeId) != 0)
			return m_Types[typeId]->kind != TypeKind::Incomplete;

		// Otherwise look upward
		if (m_UpwardTypeTable)
			return m_UpwardTypeTable->HasCompleteType(typeId);

		assert(m_TableType == TypeTableType::Global);
		return false;
	}

	const Type* TypeTable::Lookup(const std::string& typeName)
	{
		// First look in the current table
		if (m_Typenames.count(typeName) != 0)
			return m_Types[m_Typenames[typeName]]; // TODO: Unify typenames and types

		// Otherwise look upward
		if (m_UpwardTypeTable)
			return m_UpwardTypeTable->Lookup(typeName);

		assert(m_TableType == TypeTableType::Global);
		abort();
		return nullptr;
	}

	const Type* TypeTable::Lookup(TypeId typeId)
	{
		// First look in the current table
		if (m_Types.count(typeId) != 0)
			return m_Types[typeId];

		// Otherwise look upward
		if (m_UpwardTypeTable)
			return m_UpwardTypeTable->Lookup(typeId);

		assert(m_TableType == TypeTableType::Global);
		abort();
		return nullptr;
	}

	std::vector<const Type*> TypeTable::Lookup(std::vector<TypeId> typeIds)
	{
		std::vector<const Type*> types;
		for (auto id : typeIds)
		{
			types.push_back(Lookup(id));
		}
		return  types;
	}

	const Type* TypeTable::LookupReference(TypeId typeId)
	{
		const std::string& name = Lookup(typeId)->GetName(this);
		return Lookup("Reference<" + name + ">");
	}

	const Type* TypeTable::Insert(const std::string& typeName, TypeKind type, bool insertReference)
	{
		assert(!HasCompleteType(typeName));

		uint16_t id = GetNextFreeTypeId();

		m_Typenames[typeName] = id;
		m_Types[id] = new Type(typeName, id, type);

		if (insertReference)
			InsertReferenceType(m_Types[id]);

		return m_Types[id];
	}
	const Type* TypeTable::InsertGeneric(TypeKind type, std::vector<const Type*> typeArguments, bool& existed, bool insertReference)
	{
		assert(!typeArguments.empty());
		if (type == TypeKind::Array)
			assert(typeArguments.size() == 1);
		// TODO: Validate the rest of generic types

		Type newType;
		newType.kind = type;
		for (auto& arg : typeArguments)
		{
			newType.typeArguments.push_back(arg->id);
		}

		std::string name = newType.GetName(this);

		existed = HasCompleteType(name);
		if (existed)
			return Lookup(name);

		// Go to the global table to insert generic types
		// This is to prevent the same type having different id's in different scopes
		TypeTable* global = this;
		while (global && global->m_TableType == TypeTableType::Local)
		{
			global = global->m_UpwardTypeTable;
		}

		Type* typeEntry = (Type*)global->Insert(name, type, insertReference);

		// Set the type arguments
		for (auto& argument : typeArguments)
		{
			typeEntry->typeArguments.push_back(argument->id);
		}

		return typeEntry;
	}
	const Type* TypeTable::InsertGeneric(TypeKind type, std::vector<const Type*> typeArguments, bool insertReference)
	{
		bool discard = false;
		return InsertGeneric(type, typeArguments, insertReference, discard);
	}

	const Type* TypeTable::InsertIncomplete()
	{
		std::string name = "X" + std::to_string(m_NextFreeTypeId);

		// Go to the global table to insert incomplete types
		// This is to prevent the same type having different id's in different scopes
		TypeTable* global = this;
		while (global && global->m_TableType == TypeTableType::Local)
		{
			global = global->m_UpwardTypeTable;
		}

		return global->Insert(name, TypeKind::Incomplete);
	}

	const Type* TypeTable::Replace(const Type* type, const Type* newType)
	{
		Type* t = (Type*)type;
		*t = *newType;
		
		return t;
	}

	void TypeTable::Replace(TypeId type, TypeId newType)
	{
		*LookupNonConst(type) = *Lookup(newType);
	}

	const Type* TypeTable::InsertArray(const Type* underlyingType, bool& existed)
	{
		return InsertGeneric(TypeKind::Array, { underlyingType }, existed, true);
	}
	const Type* TypeTable::InsertTuple(std::vector<const Type*> underlyingTypes)
	{
		return InsertGeneric(TypeKind::Tuple, underlyingTypes);
	}
	const Type* TypeTable::InsertFunction(std::vector<const Type*> argumentTypes, const Type* returnType)
	{
		std::vector<const Type*> typeArguments = argumentTypes;
		typeArguments.push_back(returnType);

		return InsertGeneric(TypeKind::Function, typeArguments);
	}
	const Type* TypeTable::InsertFunction(std::vector<const Type*> argumentTypesAndReturnType)
	{
		return InsertGeneric(TypeKind::Function, argumentTypesAndReturnType);
	}

	Type* TypeTable::LookupNonConst(TypeId typeId)
	{
		return (Type*)Lookup(typeId);
	}

	TypeId TypeTable::GetNextFreeTypeId()
	{
		return m_NextFreeTypeId++;
	}

	void TypeTable::AddTypeRelation(Type* type, TypeId relatedType, TypeRelation::ConversionType subtypeConversion, TypeRelation::ConversionType supertypeConversion)
	{
		assert(HasCompleteType(relatedType));

		AddTypeRelation(type, LookupNonConst(relatedType), subtypeConversion, supertypeConversion);
	}
	void TypeTable::AddTypeRelation(Type* type, Type* relatedType, TypeRelation::ConversionType subtypeConversion, TypeRelation::ConversionType supertypeConversion)
	{
		type->subtypes.push_back({ subtypeConversion, relatedType->id });
		relatedType->supertypes.push_back({ supertypeConversion, type->id });
	}

	std::optional<TypeRelation::ConversionType> TypeTable::GetFullSupertypeRelationTo(const Type* type, const Type* expectedSupertype)
	{
		for (auto& typeRelation : type->supertypes)
		{
			const Type* supertype = Lookup(typeRelation.relatedType);
			assert(supertype);

			if (supertype->id == expectedSupertype->id)
				return typeRelation.conversionType;

			// Continue searching upwards
			auto upwardTypeRelation = GetFullSupertypeRelationTo(supertype, expectedSupertype);
			if (upwardTypeRelation.has_value())
			{
				// If this relation and the above are both implicit, or explicit then dont modify it
				if (typeRelation.conversionType == upwardTypeRelation)
					return upwardTypeRelation.value();

				// Otherwise it is explicit somewhere in the chain, so the total relation is explicit
				return TypeRelation::Explicit;
			}
		}

		return {};
	}

	std::optional<TypeRelation::ConversionType> TypeTable::GetFullSubtypeRelationTo(const Type* type, const Type* expectedSubtype)
	{
		for (auto& typeRelation : type->subtypes)
		{
			const Type* subtype = Lookup(typeRelation.relatedType);
			assert(subtype);

			if (subtype->id == expectedSubtype->id)
				return typeRelation.conversionType;

			// Continue searching downwards
			auto downwardTypeRelation = GetFullSubtypeRelationTo(subtype, expectedSubtype);
			if (downwardTypeRelation.has_value())
			{
				// If this relation and the above are both implicit, or explicit then dont modify it
				if (typeRelation.conversionType == downwardTypeRelation)
					return downwardTypeRelation.value();

				// Otherwise it is explicit somewhere in the chain, so the total relation is explicit
				return TypeRelation::Explicit;
			}
		}

		return {};
	}

	std::optional<TypeRelation::ConversionType> TypeTable::GetFullTypeRelationTo(const Type* type, const Type* expectedType)
	{
		auto supertypeRelation = GetFullSupertypeRelationTo(type, expectedType);
		auto subtypeRelation = GetFullSubtypeRelationTo(type, expectedType);

		// Can't be both sub and super type at the same time
		if (subtypeRelation.has_value() && supertypeRelation.has_value())
			abort();

		if (subtypeRelation.has_value())
			return subtypeRelation;
		if (supertypeRelation.has_value())
			return supertypeRelation;
		
		return {};
	}

	bool TypeTable::IsTypeImplicitSubtypeOf(const Type* subtype, const Type* expectedSupertype)
	{
		if (AreTypesEquivalent(subtype, expectedSupertype))
			return true;

		for (auto& typeRelation : subtype->supertypes)
		{
			const Type* supertype = Lookup(typeRelation.relatedType);
			assert(supertype);

			if (supertype->id == expectedSupertype->id)
				return typeRelation.conversionType == TypeRelation::Implicit;

			// Continue searching upwards
			if (IsTypeImplicitSubtypeOf(supertype, expectedSupertype))
				return typeRelation.conversionType == TypeRelation::Implicit;
		}

		return false;
	}

	bool TypeTable::AreTypesEquivalent(const Type* a, const Type* b)
	{
		return a->id == b->id;
	}

	bool TypeTable::AreTypesEquivalent(TypeId a, TypeId b)
	{
		return Lookup(a)->id == Lookup(b)->id;
	}

	uint16_t TypeTable::GetHeightOfTypeRelation(const Type* type)
	{
		uint16_t highestRelation = 0;
		for (const TypeRelation& relation : type->subtypes)
		{
			auto typeEntry = Lookup(relation.relatedType);
			assert(typeEntry);

			uint16_t height = GetHeightOfTypeRelation(typeEntry) + 1;
			if (height >= highestRelation)
				highestRelation = height;
		}

		return highestRelation;
	}

	void TypeTable::Print(std::string padding)
	{
		for (auto& [_, entry] : m_Types)
		{
			std::cout << padding << "#" << entry->id << ": " << entry->GetName(this) << ", "
				<< TypeEntryTypeToString(entry->kind) << "\n";
		}
	}

	void TypeTable::InsertPrimitiveTypes()
	{
		// Insert the build in primitives to the typeEntry table 
		std::vector<std::string> typeKeywords = { "void", "int", "bool", "double", "string" };

		for (uint16_t i = 0; i < typeKeywords.size(); i++)
		{
			m_Typenames[typeKeywords[i]] = Insert(typeKeywords[i], TypeKind::Primitive, false)->id;
		}

		// Reference types
		for (uint16_t i = 0; i < typeKeywords.size(); i++)
		{
			InsertReferenceType(m_Types[i]);
			//m_Types[i] = { typeKeywords[i], i, TypeKind::Primitive };
		}

		// Insert relation for types
		// double -> int (explicit)
		// int -> double (implicit)
		AddTypeRelation(m_Types[PrimitiveValueTypes::Double], PrimitiveValueTypes::Integer, TypeRelation::Explicit, TypeRelation::Implicit);
		

		// temp
		// string -> int (explicit)
		// int -> string (implicit)
		//AddTypeRelation(m_Types[PrimitiveValueTypes::String], PrimitiveValueTypes::Integer, TypeRelation::Explicit, TypeRelation::Implicit);

		// int -> bool (explicit)
		// bool -> int (explicit)
		AddTypeRelation(m_Types[PrimitiveValueTypes::Integer], PrimitiveValueTypes::Bool, TypeRelation::Explicit, TypeRelation::Explicit);
	}

	const Type* TypeTable::InsertReferenceType(const Type* type)
	{
		Type* referenceType = (Type*)InsertGeneric(TypeKind::Reference, { type }, false);

		// Make it an implicit subtype of type

		// T -> T& (not possible)
		// T& -> T (implicit)
		referenceType->supertypes.push_back({ TypeRelation::Implicit, type->id });

		return referenceType;
	}

	TypeTable::~TypeTable()
	{
		std::cout << "TYPETABLE DESTRUCTOR\n";
	}

	TypeId TypeTable::m_NextFreeTypeId = 0;

}