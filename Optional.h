// @2024 (IHarzI) Maslianka Zakhar
// Implementation(pretty simple, yet useful) of Optional/Optional Pair

#pragma once

// Define HARZ_Optional_ASSERT(cond) for your custom assert 
#ifndef HARZ_Optional_ASSERT
#include <cassert>
#define HARZ_Optional_ASSERT(cond) assert(cond)
#endif // !HRZ_Optional

#include <type_traits>
#include <utility>

namespace harz
{

	namespace detailOptionalImplementation
	{
		template<class T>
		struct OptionalHelpler
		{
      // Last byte is always metadata
			static constexpr unsigned int MetadataIndex() { return sizeof(T) + 1 - 1; };
			static constexpr unsigned int OptionalSize() { return sizeof(T) > 0 ? sizeof(T) + 1 : 1; };
		};

		template<class A, class B>
		struct OptionalPairHelpler
		{
			static constexpr unsigned int OptionalSize() { 
				return sizeof(A) > sizeof(B) ? sizeof(A) > 0 ? sizeof(A) + 1 : 1 : sizeof(B) > 0 ? sizeof(B) + 1 : 1;
			};
      // Last byte is always metadata
			static constexpr unsigned int MetadataIndex() { return OptionalSize() - 1; };
		};

		enum MetadataFlags : unsigned char
		{
			ValueUnset = 0,
			ValueSet = 1
		};

	}
#define HelperOptional detailOptionalImplementation::OptionalHelpler
#define HelperOptionalFlags detailOptionalImplementation::MetadataFlags

	template <class T>
	struct Optional
	{
		static_assert(!std::is_same<T, void>::value, "Do not use void as template argument type for optional!");
		Optional(T& Value) { *(T*)&OptionalValue = Value; ResetMetadataFlag(HelperOptionalFlags::ValueSet); };
		Optional(T&& Value) { ResetMetadataFlag(HelperOptionalFlags::ValueUnset);  EmplaceMove(std::move(Value)); };

		template<typename ...Args>
		Optional(Args&... args)
		{
			ResetMetadataFlag(HelperOptionalFlags::ValueUnset);
			Emplace(args...);
			SetMetadataFlag(HelperOptionalFlags::ValueSet);
		}

		Optional(Optional<T>& Other) { ResetMetadataFlag(HelperOptionalFlags::ValueUnset); if (Other.IsSet()) SetValue(*(T*)&(Other.OptionalValue)); };
		Optional(Optional<T>&& Other) {
			unsigned char OtherFlags = Other.OptionalValue[HelperOptional<T>::MetadataIndex()];
			if (Other.IsSet())
			{
				EmplaceMove(std::move((*(T*)Other.OptionalValue)));
				Other.OptionalValue[HelperOptional<T>::MetadataIndex()] = HelperOptionalFlags::ValueUnset;
			};
			ResetMetadataFlag(OtherFlags);
		};
		Optional() { ResetMetadataFlag(HelperOptionalFlags::ValueUnset); };
		~Optional() { if (IsSet()) (*(T*)&OptionalValue).~T(); };
		Optional& operator=(Optional<T>& Other)
		{
			// Set new value
			if (Other.IsSet())
				SetValue(*(T*)&(Other.OptionalValue));
			return *this;
		};
	
		Optional& operator=(Optional<T>&& Other)
		{
			unsigned char OtherFlags = Other.OptionalValue[HelperOptional<T>::MetadataIndex()];
			if (Other.IsSet())
			{
				EmplaceMove(std::move((*(T*)Other.OptionalValue)));
				Other.OptionalValue[HelperOptional<T>::MetadataIndex()] = HelperOptionalFlags::ValueUnset;
			};
			ResetMetadataFlag(OtherFlags);

			return *this;
		};

		bool IsSet() const { return OptionalValue[HelperOptional<T>::MetadataIndex()] & HelperOptionalFlags::ValueSet; };
		void DefaultInitialize()
		{
			HARZ_Optional_ASSERT(!IsSet() && "Do not call DefaultInitialize on optional with value!");
			if (!IsSet())
				new((void*)&OptionalValue) T();

			SetMetadataFlag(HelperOptionalFlags::ValueSet);
		};
		void SetValue(T Value)
		{
			if (!IsSet())
				new((void*)&OptionalValue) T(Value);
			else
				*(T*)&OptionalValue = Value;
	
			SetMetadataFlag(HelperOptionalFlags::ValueSet);
		};
		const T& GetValue() const { HARZ_Optional_ASSERT(IsSet()); return *(T*)&OptionalValue; };
		T& GetValue() { HARZ_Optional_ASSERT(IsSet()); return *(T*)&OptionalValue; };
	
		template<typename ...Args>
		void Emplace(Args&&... args) {
			new((void*)&OptionalValue) T(std::forward<Args>(args)...);
			SetMetadataFlag(HelperOptionalFlags::ValueSet);
		};
	
		void EmplaceMove(T&& Other) {
			if (IsSet())
				*(T*)&OptionalValue = std::move(Other);
			else
				Emplace(std::move(Other));
	
			SetMetadataFlag(HelperOptionalFlags::ValueSet);
		};
	private:
		void SetMetadataFlag(unsigned char flag) { OptionalValue[HelperOptional<T>::MetadataIndex()] |= flag; };
		void ResetMetadataFlag(unsigned char flag) { OptionalValue[HelperOptional<T>::MetadataIndex()] = flag; };
		// last is metadata
		unsigned char OptionalValue[HelperOptional<T>::OptionalSize()];
	};

#define HelperOptionalPair detailOptionalImplementation::OptionalPairHelpler

	template <class A, class B>
	struct OptionalPair
	{
		static_assert(!std::is_same<A, void>::value || !std::is_same<B, void>::value, "Do not use void as template argument type for optional!");
		enum metadataType : unsigned char
		{
			NONE = 0,
			TypeA = 32,
			TypeB = 64
		};

		OptionalPair(A& Value) { SetValueA(Value); ResetMetadataFlag(HelperOptionalFlags::ValueSet | metadataType::TypeA); };
		OptionalPair(B& Value) { SetValueB(Value); ResetMetadataFlag(HelperOptionalFlags::ValueSet | metadataType::TypeB); };
		OptionalPair(A&& Value) { ResetMetadataFlag(HelperOptionalFlags::ValueSet | metadataType::TypeA);  EmplaceMoveA(std::move(Value)); };
		OptionalPair(B&& Value) { ResetMetadataFlag(HelperOptionalFlags::ValueSet | metadataType::TypeB);  EmplaceMoveB(std::move(Value)); };

		OptionalPair(OptionalPair& Other) 
		{ 
			ResetMetadataFlag(HelperOptionalFlags::ValueUnset); 
			if (Other.IsSet())
			{
				SetValueByTypeIndex(Other.GetTypeIndex(), &Other.OptionalValue);
			};
		};

		template<typename OptClass, typename ...Args>
		OptionalPair(Args&... args)
		{
			EmplaceByTypeIndex(GetIndexForType<OptClass>());

			SetMetadataFlag(HelperOptionalFlags::ValueSet | GetIndexForType<GetIndexForType>());
		}

		OptionalPair(OptionalPair&& Other) {
			unsigned char OtherFlags = Other.OptionalValue[HelperOptionalPair<A,B>::MetadataIndex()];
			if (Other.IsSet())
			{
				EmplaceMoveByTypeIndex(Other.GetTypeIndex(), &Other.OptionalValue);
				Other.OptionalValue[HelperOptionalPair<A,B>::MetadataIndex()] = HelperOptionalFlags::ValueUnset;
			};
			ResetMetadataFlag(OtherFlags);
		};

		OptionalPair() { ResetMetadataFlag(HelperOptionalFlags::ValueUnset); };
		~OptionalPair() { if (IsSet()) DestroyValueByTypeIndex(GetTypeIndex()); };

		OptionalPair& operator=(OptionalPair& Other)
		{
			if (IsSet())
			{
				DestroyValueByTypeIndex(GetTypeIndex());
				ResetMetadataFlag(HelperOptionalFlags::ValueUnset);
			};
			// Set new value
			if (Other.IsSet())
			{
				SetValueByTypeIndex(Other.GetTypeIndex(), &Other.OptionalValue);
				SetMetadataFlag(HelperOptionalFlags::ValueSet | Other.GetTypeIndex());
			};
			return *this;
		};

		OptionalPair& operator=(OptionalPair&& Other)
		{
			if (IsSet())
			{
				DestroyValueByTypeIndex(GetTypeIndex());
				ResetMetadataFlag(HelperOptionalFlags::ValueUnset);
			};
			// Set new value
			unsigned char OtherFlags = Other.OptionalValue[HelperOptionalPair<A,B>::MetadataIndex()];
			if (Other.IsSet())
			{
				EmplaceMoveByTypeIndex(Other.GetTypeIndex(), &Other.OptionalValue);
				Other.OptionalValue[HelperOptionalPair<A,B>::MetadataIndex()] = HelperOptionalFlags::ValueUnset;
			};
			ResetMetadataFlag(OtherFlags);
			return *this;
		};

		bool IsSet() const { return OptionalValue[HelperOptionalPair<A,B>::MetadataIndex()] & HelperOptionalFlags::ValueSet; };

		template<typename OptClass>
		bool ContainsType()
		{
			if (IsSet())
			{
				if (std::is_same<OptClass, A>::value)
				{
					return IsTypeA();
				}
				else if (std::is_same<OptClass, B>::value)
				{
					return IsTypeB();
				}
			}
			return false;
		};

		template<typename OptClass>
		metadataType GetIndexForType()
		{
			if (IsSet())
			{
				if (std::is_same<OptClass, A>::value)
				{
					return TypeA;
				}
				else if (std::is_same<OptClass, B>::value)
				{
					return TypeB;
				}
			}
			return NONE;
		};

		template<typename OptClass>
		void DefaultInitialize()
		{
			HARZ_Optional_ASSERT(!IsSet() && "Do not call DefaultInitialize on optional with value!");
			if (!IsSet())
			{
				ResetMetadataFlag(HelperOptionalFlags::ValueUnset);
				if (std::is_same<OptClass, A>::value)
				{
					new((void*)&OptionalValue) A();
					SetMetadataFlag(metadataType::TypeA);
				}
				else if (std::is_same<OptClass, B>::value)
				{
					new((void*)&OptionalValue) B();
					SetMetadataFlag(metadataType::TypeB);
				}
				else
					HARZ_Optional_ASSERT(false && "invalidTypeId");
			};
			SetMetadataFlag(HelperOptionalFlags::ValueSet);
		};

		void SetValueA(A Value)
		{
			if (IsSet())
			{
				DestroyValueByTypeIndex(GetTypeIndex());
			}
			new((void*)&OptionalValue) A(Value);

			ResetMetadataFlag(HelperOptionalFlags::ValueSet | metadataType::TypeA);
		};

		void SetValueB(B Value)
		{
			if (IsSet())
			{
				DestroyValueByTypeIndex(GetTypeIndex());
			}
			new((void*)&OptionalValue) B(Value);
			
			ResetMetadataFlag(HelperOptionalFlags::ValueSet | metadataType::TypeB);
		};

		const A& GetValueA() const { HARZ_Optional_ASSERT(IsSet() && IsTypeA()); return *(A*)&OptionalValue; };
		A& GetValueA() { HARZ_Optional_ASSERT(IsSet() && IsTypeA()); return *(A*)&OptionalValue; };

		const B& GetValueB() const { HARZ_Optional_ASSERT(IsSet() && IsTypeB()); return *(B*)&OptionalValue; };
		B& GetValueB() { HARZ_Optional_ASSERT(IsSet() && IsTypeB()); return *(B*)&OptionalValue; };

		template<typename OptClass, typename ...Args>
		void Emplace(Args&&... args) {
			if (IsSet())
			{
				DestroyValueByTypeIndex(GetTypeIndex());
				ResetMetadataFlag(HelperOptionalFlags::ValueUnset);
			}

			if (std::is_same<OptClass, A>::value)
			{
				EmplaceByTypeIndex(metadataType::TypeA, args...);
					SetMetadataFlag(metadataType::TypeA);
			}
			else if (std::is_same<OptClass, B>::value)
			{
				EmplaceByTypeIndex(metadataType::TypeB, args...);
				SetMetadataFlag(metadataType::TypeB);
			}
			else
			{
				HARZ_Optional_ASSERT(false && "Wrong type to Emplace");
			}
			SetMetadataFlag(HelperOptionalFlags::ValueSet);
		};

		void EmplaceMoveA(A&& Other) {
			if (IsSet())
			{
				DestroyValueByTypeIndex(GetTypeIndex());
				EmplaceMoveByTypeIndex(metadataType::TypeA, &Other);
			}
			else
				EmplaceByTypeIndex(metadataType::TypeA, std::move(Other));

			SetMetadataFlag(HelperOptionalFlags::ValueSet | metadataType::TypeA);
		};

		void EmplaceMoveB(B&& Other) {
			if (IsSet())
			{
				DestroyValueByTypeIndex(GetTypeIndex());
				EmplaceMoveByTypeIndex(metadataType::TypeB, &Other);
			}
			else
				EmplaceByTypeIndex(metadataType::TypeB, std::move(Other));

			SetMetadataFlag(HelperOptionalFlags::ValueSet | metadataType::TypeB);
		};

		bool IsTypeA() const { return GetTypeIndex() == metadataType::TypeA; };
		bool IsTypeB() const { return GetTypeIndex() == metadataType::TypeB; };

	private:
		void SetValueByTypeIndex(metadataType typeId, void* ValueAddress)
		{
			switch (typeId)
			{
			case TypeA:
				SetValue(*(A*)ValueAddress);
				break;
			case TypeB:
				SetValue(*(B*)ValueAddress);
				break;
			default:
				HARZ_Optional_ASSERT(false && "invalidTypeId");
				break;
			};
		};

		void DestroyValueByTypeIndex(metadataType typeId)
		{
			if (IsSet())
			{
				switch (typeId)
				{
				case TypeA:
					(*(A*)&OptionalValue).~A();
					break;
				case TypeB:
					(*(B*)&OptionalValue).~B();
					break;
				default:
					HARZ_Optional_ASSERT(false && "invalidTypeId");
					break;
				};
			};
		};

		template<typename ...Args>
		void EmplaceByTypeIndex(metadataType typeId, Args&&... args) {
			switch (typeId)
			{
			case TypeA:
				new((void*)&OptionalValue) A(std::forward<Args>(args)...);
				break;
			case TypeB:
				new((void*)&OptionalValue) B(std::forward<Args>(args)...);
				break;
			default:
				HARZ_Optional_ASSERT(false && "invalidTypeId");
				break;
			};
		};

		void EmplaceMoveByTypeIndex(metadataType typeId, void* ValueAddress) {
			switch (typeId)
			{
			case TypeA:
				*(A*)&OptionalValue = std::move(*(A*)ValueAddress);
				break;
			case TypeB:
				*(B*)&OptionalValue = std::move(*(B*)ValueAddress);
				break;
			default:
				HARZ_Optional_ASSERT(false && "invalidTypeId");
				break;
			};
		};

		metadataType GetTypeIndex() const
		{
			if (!IsSet())
				return NONE;
			return OptionalValue[HelperOptionalPair<A,B>::MetadataIndex()] & metadataType::TypeA ? metadataType::TypeA : metadataType::TypeB;
		};

		void SetMetadataFlag(unsigned char flag) { OptionalValue[HelperOptionalPair<A,B>::MetadataIndex()] |= flag; };
		void ResetMetadataFlag(unsigned char flag) { OptionalValue[HelperOptionalPair<A,B>::MetadataIndex()] = flag; };
		unsigned char OptionalValue[HelperOptionalPair<A,B>::OptionalSize()];
	};

#undef OptionalPairSizeCheck
#undef HelperOptional
#undef HelperOptionalPair
#undef HelperOptionalFlags
};
