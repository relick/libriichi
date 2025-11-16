#pragma once

#include "Base.hpp"
#include "Containers.hpp"

#include "range/v3/view/concat.hpp"

#include <cassert>
#include <concepts>
#include <iterator>
#include <limits>
#include <ranges>
#include <utility>

namespace Riichi::Utils
{

//------------------------------------------------------------------------------
namespace IterInterfaceTypes
{

template<typename I> concept PtrArrowDefined = requires( I i ) { i.operator->(); };

template<typename T> struct AddConstT { using Type = T const; };
template<typename T> struct AddConstT<T&> { using Type = T const&; };
template<typename T> struct AddConstT<T*> { using Type = T const*; };
template<typename T> struct AddConstT<T const&> { using Type = T const&; };
template<typename T> struct AddConstT<T const*> { using Type = T const*; };
template<typename T> using AddConst = typename AddConstT<T>::Type;

template<typename Iter> struct Ref { using Type = std::iter_reference_t<Iter>; };

template<typename Iter> struct Ptr { using Type = std::add_pointer_t<typename Ref<Iter>::Type>; };
template<typename Iter> requires PtrArrowDefined<Iter> struct Ptr<Iter> { using Type = decltype( std::declval<Iter&>().operator->() ); };
template<typename Iter> requires ( requires { typename Iter::pointer; } && !PtrArrowDefined<Iter> ) struct Ptr<Iter> { using Type = typename Iter::pointer; };

template<typename Iter> struct Value { using Type = std::remove_cvref_t<typename Ref<Iter>::Type>; };
template<typename Iter> requires requires { typename Iter::value_type; } struct Value<Iter> { using Type = typename Iter::value_type; };

template<typename Iter> struct Diff { using Type = ptrdiff_t; };
template<typename Iter> requires requires { typename std::iter_difference_t<Iter>; } struct Diff<Iter> { using Type = std::iter_difference_t<Iter>; };

}

namespace IterInterfaceConcepts
{

template<typename I> concept Incrementable = requires( I i ) { { ++i } -> std::same_as<I&>; };
template<typename I> concept Decrementable = requires( I i ) { { --i } -> std::same_as<I&>; };
template<typename I> concept EqualityComparable = requires( I const& i, I const& j ) { { i == j } -> std::convertible_to<bool>; };
template<typename I> concept LessThanComparable = requires( I const& i, I const& j ) { { i < j } -> std::convertible_to<bool>; };
template<typename I> concept DifferenceAble = requires( I const& i, I const& j ) { { j - i } -> std::convertible_to<typename IterInterfaceTypes::Diff<I>::Type>; };
template<typename I> concept OffsetAble = requires( I i, typename IterInterfaceTypes::Diff<I>::Type const n ) { { i += n } -> std::same_as<I&>; };
template<typename I> concept AddressAble = std::is_lvalue_reference_v<typename IterInterfaceTypes::Ref<I>::Type>
&& std::same_as<typename IterInterfaceTypes::Value<I>::Type, std::remove_cvref_t<typename IterInterfaceTypes::Ref<I>::Type>>
&& std::same_as<decltype( std::addressof( *std::declval<I&>() ) ), typename IterInterfaceTypes::Ptr<I>::Type>;

template<typename I> concept BasicInput = requires( I i ) { { *i } -> std::same_as<typename IterInterfaceTypes::Ref<I>::Type>; } && std::semiregular<I> && Incrementable<I>;
template<typename I> concept BasicForward = BasicInput<I> && EqualityComparable<I>;
template<typename I> concept BasicBidirectional = BasicForward<I> && Decrementable<I>;
template<typename I> concept BasicRandomAccess = BasicBidirectional<I> && LessThanComparable<I> && DifferenceAble<I> && OffsetAble<I>;
template<typename I> concept BasicContiguous = BasicRandomAccess<I> && AddressAble<I>;

}


//------------------------------------------------------------------------------
// Automatically build iterators types by wrapping with this type or ConstIteratorInterface.
// Must implement the following:
// base& ++operator()
// reference operator*() const
// 
// Can also implement (only) the following:
// bool operator==(base const&) const
// base& --operator()
// bool operator<(base const&) const
// difference_type operator-(base const&) const
// base& operator+=(difference_type)
// 
// May also manually implement operator-> but it will be automatically implemented if not (and if possible).
// 
// All other expected iterator operations will be automatically generated from the above,
// when provided the prerequisite operations.
// The types and iterator tag required for iterator_traits will be automatically generated and selected
// as appropriate.
//------------------------------------------------------------------------------
template<IterInterfaceConcepts::BasicInput BaseIter, template<typename> typename Constify = std::type_identity_t>
struct IteratorInterface
	: protected BaseIter
{
	// Add all required types, including the tag
	using value_type = typename IterInterfaceTypes::Value<BaseIter>::Type;
	using difference_type = typename IterInterfaceTypes::Diff<BaseIter>::Type;
	using pointer = Constify<typename IterInterfaceTypes::Ptr<BaseIter>::Type>;
	using reference = Constify<typename IterInterfaceTypes::Ref<BaseIter>::Type>;
	using iterator_category = std::conditional_t<
		IterInterfaceConcepts::BasicContiguous<BaseIter>,
		std::contiguous_iterator_tag,
		std::conditional_t<
			IterInterfaceConcepts::BasicRandomAccess<BaseIter>,
			std::random_access_iterator_tag,
			std::conditional_t<
				IterInterfaceConcepts::BasicBidirectional<BaseIter>,
				std::bidirectional_iterator_tag,
				std::conditional_t<
					IterInterfaceConcepts::BasicForward<BaseIter>,
					std::forward_iterator_tag,
					std::input_iterator_tag
				>
			>
		>
	>;

	// Inherit the constructors directly
	using BaseIter::BaseIter;

	// --- Start building std::input_iterator

	// Wrapping what we know we've been given
	constexpr reference operator*() const
	{
		return static_cast< reference >( BaseIter::operator*() );
	}
	constexpr IteratorInterface& operator++()
	{
		BaseIter::operator++(); return *this;
	}

	// Add post-increment
	constexpr IteratorInterface operator++( int ) const { IteratorInterface tmp; ++( *this ); return tmp; }

	// --- Finish building std::input_iterator

	// --- Start building std::forward_iterator

	// Wrap equality, allowing comparison between const and non-const iterators
	template<template<typename> typename OtherConstify>
	constexpr bool operator==( IteratorInterface<BaseIter, OtherConstify> const& i_other ) const
		requires IterInterfaceConcepts::EqualityComparable<BaseIter>
	{
		return static_cast< BaseIter const& >( *this ) == static_cast< BaseIter const& >( i_other );
	}
	constexpr bool operator==( IteratorInterface const& i_other ) const
		requires IterInterfaceConcepts::EqualityComparable<BaseIter>
	{
		return operator==<Constify>( i_other );
	}

	// --- Finish building std::forward_iterator

	// --- Start building std::bidirectional_iterator

	// Add decrement
	constexpr IteratorInterface& operator--()
		requires IterInterfaceConcepts::Decrementable<BaseIter>
	{
		BaseIter::operator--(); return *this;
	}
	constexpr IteratorInterface operator--( int ) const
		requires IterInterfaceConcepts::Decrementable<BaseIter>
	{
		IteratorInterface tmp; --( *this ); return tmp;
	}

	// --- Finish building std::bidirectional_iterator

	// --- Start building std::random_access_iterator

	// Add less than
	template<template<typename> typename OtherConstify>
	constexpr bool operator<( IteratorInterface<BaseIter, OtherConstify> const& i_other ) const
		requires IterInterfaceConcepts::LessThanComparable<BaseIter>
	{
		return static_cast< bool >( static_cast< BaseIter const& >( *this ) < static_cast< BaseIter const& >( i_other ) );
	}
	constexpr bool operator<( IteratorInterface const& i_other ) const
		requires IterInterfaceConcepts::LessThanComparable<BaseIter>
	{
		return operator< <Constify>( i_other );
	}

	// Add the total ordering
	template<template<typename> typename OtherConstify>
	constexpr std::strong_ordering operator<=>( IteratorInterface<BaseIter, OtherConstify> const& i_other ) const
		requires IterInterfaceConcepts::EqualityComparable<BaseIter>&& IterInterfaceConcepts::LessThanComparable<BaseIter>
	{
		return *this == i_other ? std::strong_ordering::equal : ( *this < i_other ? std::strong_ordering::less : std::strong_ordering::greater );
	}
	constexpr std::strong_ordering operator<=>( BaseIter const& i_other ) const
		requires IterInterfaceConcepts::EqualityComparable<BaseIter>&& IterInterfaceConcepts::LessThanComparable<BaseIter>
	{
		return operator<=> <Constify>( i_other );
	}

	// Wrap the difference op
	template<template<typename> typename OtherConstify>
	constexpr difference_type operator-( IteratorInterface<BaseIter, OtherConstify> const& i_other ) const
		requires IterInterfaceConcepts::DifferenceAble<BaseIter>
	{
		return static_cast< difference_type >( static_cast< BaseIter const& >( *this ) - static_cast< BaseIter const& >( i_other ) );
	}
	constexpr difference_type operator-( IteratorInterface const& i_other ) const
		requires IterInterfaceConcepts::DifferenceAble<BaseIter>
	{
		return operator-<Constify>( i_other );
	}

	// Wrap the offset op
	constexpr IteratorInterface& operator+=( difference_type i_other )
		requires IterInterfaceConcepts::OffsetAble<BaseIter>
	{
		BaseIter::operator+=( i_other ); return *this;
	}

	// Make all the derivable offset ops
	constexpr IteratorInterface& operator-=( difference_type i_offset )
		requires IterInterfaceConcepts::OffsetAble<BaseIter>
	{
		return *this += -i_offset;
	}

	constexpr IteratorInterface operator+( difference_type i_offset ) const
		requires IterInterfaceConcepts::OffsetAble<BaseIter>
	{
		IteratorInterface tmp; tmp += i_offset; return tmp;
	}

	constexpr IteratorInterface operator-( difference_type i_offset ) const
		requires IterInterfaceConcepts::OffsetAble<BaseIter>
	{
		IteratorInterface tmp; tmp -= i_offset; return tmp;
	}

	friend constexpr IteratorInterface operator+( difference_type i_offset, IteratorInterface const& i_iter )
		requires IterInterfaceConcepts::OffsetAble<BaseIter>
	{
		return i_iter + i_offset;
	}

	constexpr reference operator[]( difference_type i_offset ) const
		requires IterInterfaceConcepts::OffsetAble<BaseIter>
	{
		return *( *this + i_offset );
	}

	// --- Finish building std::random_access_iterator

	// --- Start building std::contiguous_iterator

	using element_type = value_type;

	constexpr pointer operator->() const
		requires IterInterfaceTypes::PtrArrowDefined<BaseIter>
	{
		return static_cast< pointer >( BaseIter::operator->() );
	}
	constexpr pointer operator->() const
		requires ( !IterInterfaceTypes::PtrArrowDefined<BaseIter>&& IterInterfaceConcepts::AddressAble<BaseIter> )
	{
		return static_cast< pointer >( std::addressof( **this ) );
	}

	// --- Finish building std::contiguous_iterator

};

template<typename Iter>
using ConstIteratorInterface = IteratorInterface<Iter, IterInterfaceTypes::AddConst>;

//------------------------------------------------------------------------------
template<std::move_constructible T>
	requires std::is_object_v<T>
class MaybeSingleView
	: public std::ranges::view_interface<MaybeSingleView<T>>
{
	Option<T> m_value;

public:
	MaybeSingleView() = default;

	constexpr explicit MaybeSingleView( T const& t ) requires std::copy_constructible<T>
		: m_value{ t }
	{}
	constexpr explicit MaybeSingleView( Option<T> const& t ) requires std::copy_constructible<T>
		: m_value{ t }
	{}
	constexpr explicit MaybeSingleView( T&& t )
		: m_value{ std::move( t ) }
	{}
	constexpr explicit MaybeSingleView( Option<T>&& t )
		: m_value{ std::move( t ) }
	{}
	template<typename... Args> requires std::constructible_from<T, Args...>
	constexpr explicit MaybeSingleView( std::in_place_t, Args&&... i_args )
		: m_value{ std::in_place_t{}, std::forward< Args >( i_args )... }
	{}

	constexpr bool empty() const { return !m_value.has_value(); }
	constexpr size_t size() const { return empty() ? 0 : 1; }

	struct BaseIter
	{
		MaybeSingleView* m_parent{ nullptr };
		bool m_pastValue{ false };

		BaseIter() = default;
		BaseIter( MaybeSingleView* i_parent, bool i_pastValue = false )
			: m_parent{ i_parent }
			, m_pastValue{ i_pastValue || i_parent->empty() }
		{}

		T& operator*() const { return *( m_parent->m_value ); }

		bool operator==( BaseIter const& i_other ) const
		{
			return m_parent == i_other.m_parent && m_pastValue == i_other.m_pastValue;
		}
		bool operator<( BaseIter const& i_other ) const
		{
			return m_parent == i_other.m_parent && !m_pastValue && i_other.m_pastValue;
		}

		BaseIter& operator++() { m_pastValue = true; return *this; }
		BaseIter& operator--() { m_pastValue = m_parent->empty(); return *this; }

		BaseIter& operator+=( ptrdiff_t i_offset )
		{
			if ( i_offset >= 1 )
			{
				return ++( *this );
			}
			else if ( i_offset == 0 )
			{
				return *this;
			}
			else
			{
				return --( *this );
			}
		}

		ptrdiff_t operator-( BaseIter const& i_other ) const
		{
			if ( !m_parent->empty() && m_parent == i_other.m_parent )
			{
				if ( m_pastValue && !i_other.m_pastValue )
				{
					return 1;
				}
				else if ( !m_pastValue && i_other.m_pastValue )
				{
					return -1;
				}
			}
			return 0;
		}
	};

	using Iter = IteratorInterface<BaseIter>;
	using ConstIter = ConstIteratorInterface<BaseIter>;

	constexpr Iter begin() { return { this }; }
	constexpr ConstIter begin() const { return { const_cast< MaybeSingleView* >( this ) }; }

	constexpr Iter end() { return { this, true }; }
	constexpr ConstIter end() const { return { const_cast< MaybeSingleView* >( this ), true }; }
};

template<typename T>
MaybeSingleView( T ) -> MaybeSingleView<T>;
template<typename T>
MaybeSingleView( Option<T> ) -> MaybeSingleView<T>;

//------------------------------------------------------------------------------
template<std::ranges::range R>
using RawPtrOfRangeElement = decltype( std::to_address( *std::declval<std::ranges::iterator_t<R>&>() ) );

struct DerefConstOnly{};

template<std::ranges::viewable_range R, typename P = RawPtrOfRangeElement<R> >
	requires std::is_pointer_v<P>
class DerefView
	: public std::ranges::view_interface<DerefView<R>>
{
	R m_range;

public:
	constexpr explicit DerefView( R&& i_range )
		: m_range{ std::move( i_range ) }
	{}
	constexpr explicit DerefView( DerefConstOnly, R&& i_range )
		: m_range{ std::move( i_range ) }
	{}

	using BaseIterator = std::ranges::iterator_t<R>;
	using PtrType = P;
	using DerefValueType = std::pointer_traits<PtrType>::element_type;

	struct Iter
		: BaseIterator
	{
		Iter( BaseIterator iter ) : BaseIterator{ iter } {}

		// Just override the dereferencing
		constexpr DerefValueType& operator*() const { return *BaseIterator::operator*(); }
		constexpr DerefValueType* operator->() const { return std::to_address( BaseIterator::operator*() ); }
		constexpr DerefValueType& operator[]( std::ptrdiff_t i_offset ) const { return *BaseIterator::operator[]( i_offset ); }
	};

	using BaseConstIterator = decltype( std::ranges::cbegin( std::declval<R&>() ) );

	struct ConstIter
		: BaseConstIterator
	{
		ConstIter( BaseConstIterator iter ) : BaseConstIterator{ iter } {}

		// Just override the dereferencing
		constexpr DerefValueType const& operator*() const { return *BaseConstIterator::operator*(); }
		constexpr DerefValueType const* operator->() const { return std::to_address( BaseIterator::operator*() ); }
		constexpr DerefValueType const& operator[]( std::ptrdiff_t i_offset ) const { return *BaseIterator::operator[]( i_offset ); }
	};

	constexpr Iter begin() { return Iter{ m_range.begin() }; }
	constexpr ConstIter begin() const { return ConstIter{ m_range.begin() }; }

	constexpr auto end() { return m_range.end(); }
	constexpr auto end() const { return m_range.end(); }
};

template<typename R>
DerefView( R&& ) -> DerefView<std::views::all_t<R>>;

template<typename R>
DerefView( DerefConstOnly, R&& ) -> DerefView<
	std::views::all_t<R>,
	std::add_pointer_t<
		std::add_const_t<
			std::remove_pointer_t<
				RawPtrOfRangeElement<std::views::all_t<R>>
			>
		>
	>
>;

//------------------------------------------------------------------------------
template<typename R, template<typename> typename T_IterT = std::ranges::iterator_t>
using RangeRefType = decltype( *std::declval<T_IterT<R>&>() );

template<typename R>
using RangeValueType = std::remove_cvref_t<RangeRefType<R>>;

template<typename R, typename V>
concept RangeWithValueType = std::ranges::range<R> && std::same_as<RangeValueType<R>, V>;

template<typename T, RangeWithValueType<T>... Rs>
auto ConcatRanges( Rs&&... i_ranges )
{
	return ranges::concat_view( std::views::all( std::forward<Rs>( i_ranges ) )... );
}

}

namespace Riichi
{

template<bool t_ConstOnly>
struct DerefT
	: std::ranges::range_adaptor_closure<DerefT<t_ConstOnly>>
{
	template<std::ranges::range R>
	constexpr auto operator()( R&& i_rng ) const
	{
		if constexpr ( t_ConstOnly )
		{
			return Utils::DerefView{ Utils::DerefConstOnly{}, std::forward<R>( i_rng ) };
		}
		else
		{
			return Utils::DerefView{ std::forward<R>( i_rng ) };
		}
	}
};

inline constexpr DerefT<false> Deref{};
inline constexpr DerefT<true> DerefConst{};

}