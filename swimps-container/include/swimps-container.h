#pragma once

#include <cstddef>
#include <cassert>
#include <array>
#include <type_traits>

namespace swimps::container {
    //!
    //! \brief  Encapsulates a buffer + its size.
    //!         Much like std::span, except it lets you "consume" some of the buffer.
    //!         Owns none of the memory given to it via pointers.
    //!
    //! \tparam  T  The type of the buffer (so for a char array or pointer, T should be char).
    //!
    template <typename T>
    class Span final {
    public:
        //!
        //! \brief  Creates a span covering the entire buffer.
        //!
        //! \tparam  N  The size of the buffer (intended to be inferred).
        //!
        //! \param[in]  buffer  The array to cover the contents over.
        //!
        template <size_t N>
        constexpr Span(T (&buffer)[N]) noexcept
            : Span(&buffer[0], N) { }

        //!
        //! \brief  Creates a span over an integral.
        //!
        //! \tparam  Integral  The type to create a span over.
        //!
        //! \note  Only enabled when T is char because of strict aliasing.
        //!
        template <typename Integral>
        constexpr Span(Integral& data) noexcept
            requires std::integral<Integral>
                  && std::same_as<char, std::remove_const_t<T>>
            : Span(reinterpret_cast<T*>(&data), sizeof(Integral) / sizeof(T)) { }

        //!
        //! \brief  Creates a span convering the contents of an std::array.
        //!
        //! \tparam  The size parameter of the std::array.
        //!
        //! \param[in]  array  The std::array.
        //!
        template <size_t N>
        constexpr Span(std::array<std::remove_const_t<T>, N>& array) noexcept
            : Span(array.data(), array.size()) { }

        //!
        //! \brief  Creates a span starting at the buffer provided, with the number of elements specified.
        //!
        //! \param[in]  buffer          The starting address of the span.
        //! \param[in]  sizeInElements  The number of elements in the span.
        //!
        constexpr Span(
            T* const buffer,
            const size_t sizeInElements) noexcept
            : m_bufferStart(buffer),
              m_bufferCurrent(buffer),
              m_originalSize(sizeInElements),
              m_remainingElements(sizeInElements) {
            assert(buffer != nullptr);
        }

        //!
        //! \brief  Destroys the span. Does *not* destroy the underlying buffer.
        //!         Not virtual as the class is final.
        //!
        ~Span() = default;

        //!
        //! \brief  Copy-constructor.
        //!
        //! \param[in]  other  The span to copy from.
        //!
        constexpr Span(const Span& other) = default;

        //!
        //! \brief  Copy-assignment operator.
        //!
        //! \param[in]  other  The span to copy from.
        //!
        //! \returns  A reference to the assigned-to instance.
        //!
        constexpr Span& operator= (const Span& other) = default;

        //!
        //! \brief  Move-constructor.
        //!
        //! \param[in]  other  The span to move from.
        //!
        //! \note  The parameter is left in an unspecified state. Do not rely upon it.  
        //!
        constexpr Span(Span&& other) = default;

        //!
        //! \brief  Move-assignment operator.
        //!
        //! \param[in]  other  The span to move from.
        //!
        //! \returns  The moved-to instance.
        //!
        //! \note  The parameter is left in an unspecified state. Do not rely upon it.  
        //!
        constexpr Span& operator= (Span&& other) = default;

        //!
        //! \brief  Pre-increment operator.
        //!
        //! \returns  The same span, but with one element "consumed."
        //!           Essentially, the buffer moves along by one with the size adjusted accordingly.
        //!
        Span& operator++ () noexcept {
            // TODO: add option to enable these assertions in release mode?
            assert(m_remainingElements != 0);

            ++m_bufferCurrent;
            --m_remainingElements;

            return *this;
        }

        //!
        //! \brief  Post-increment operator.
        //!
        //! \param[in]  Unused.
        //!
        //! \returns  A copy of the previous span, before the increment occured.
        //!
        //! \see  operator++()
        //!
        Span operator++ (int) noexcept {
            const auto oldValue = *this;
            operator++();
            return oldValue;
        }

        //!
        //! \brief  Pre-decrement operator.
        //!
        //! \returns  The same span, but with one element "unconsumed."
        //!           Essentially, the buffer moves back by one with the size adjusted accordingly.
        //!
        Span& operator-- () noexcept {
            assert(m_bufferCurrent != m_bufferStart);

            --m_bufferCurrent;
            ++m_remainingElements;

            return *this;
        }

        //!
        //! \brief  Post-decrement.
        //!
        //! \param[in]  Unused.
        //!
        //! \returns  A copy of the previous span, before the decrement occured.
        //!
        Span operator-- (int) noexcept {
            const auto oldValue = *this;
            operator--();
            return oldValue;
        }

        //!
        //! \brief  Addition-assignment operator for spans.
        //!
        //! \param[in]  offset  What to add to this span.
        //!
        //! \returns  The modified span with the offset increased by the requested amount.
        //!
        constexpr Span& operator+= (const size_t offset) noexcept {
            // Make sure remaining elements isn't going to go below 0.
            assert(m_remainingElements >= offset);
            m_remainingElements -= offset;
            m_bufferCurrent += offset;
            return *this;
        }

        //!
        //! \brief  Subtraction-assignment operator for spans.
        //!
        //! \param[in]  offset  What to subtract from this span.
        //!
        //! \returns  The modified span with the offset decreased by the requested amount.
        //!
        constexpr Span& operator-= (const size_t offset) noexcept {
            m_remainingElements += offset;
            m_bufferCurrent -= offset;
            assert(m_bufferCurrent >= m_bufferStart);
            return *this;
        }

        //!
        //! \brief  Non-const index operator.
        //!
        //! \param[in]  index  Which index to access.
        //!
        //! \returns  A non-const reference to the element at the index requested.
        //!
        constexpr T& operator[] (const size_t index) noexcept {
            assert(m_remainingElements >= index);
            return *(m_bufferCurrent + index);
        }

        //!
        //! \brief  Const index operator.
        //!
        //! \param[in]  index  Which index to access.
        //!
        //! \returns  A const reference to the element at the index requested.
        //!
        constexpr const T& operator[] (const size_t index) const noexcept {
            return const_cast<Span*>(this)->operator[](index);
        }

        //!
        //! \brief  Gets the number of elements left in the span.
        //!
        //! \returns  The number of elements left in the span.
        //!
        //! \note  This includes the current element.
        //!
        constexpr size_t current_size() const noexcept {
            return m_remainingElements;
        }

        //!
        //! \brief  Gets the number of elements specified at construction.
        //!
        //! \returns  The size in elements specified upon construction.
        //!
        constexpr size_t original_size() const noexcept {
            return m_originalSize;
        }

    private:
        //! Where the buffer started.
        const T* m_bufferStart;

        //! Where the buffer is now (adjusted for offset).
        T* m_bufferCurrent;

        //! How big the span was originally (in elements).
        const size_t m_originalSize;

        //! How many elements are left in the span (adjusted for offset).
        size_t m_remainingElements;
    };

    //!
    //! \brief  Equality operator for spans. 
    //!
    //! \tparam  T  The type of the buffer covered by the span.
    //!
    //! \param[in]  lhs  The left operand.
    //! \param[in]  rhs  The right operand.
    //!
    //! \returns  Whether the spans are equivalent, which means:
    //!           - They point to the same memory.
    //!           - They have the same number of elements left.
    //!           - They originally had the same number of elements.
    //!
    template <typename T>
    constexpr bool operator== (const Span<T>& lhs, const Span<T>& rhs) noexcept {
        return &lhs[0] == &rhs[0]
            && lhs.original_size() == rhs.original_size()
            && lhs.current_size() == rhs.current_size();
    }

    //!
    //! \brief  Addition operator for spans.
    //!
    //! \tparam  T  The type of the buffer covered by the span.
    //!
    //! \param[in]  span    The span to add to.
    //! \param[in]  offset  What to add to the span.
    //!
    //! \returns  A new span with the offset increased by the requested amount.
    //!
    template <typename T>
    constexpr Span<T> operator+ (Span<T> span, const size_t offset) noexcept {
        return span += offset;
    }

    //!
    //! \brief  Subtraction operator for spans.
    //!
    //! \tparam  T  The type of the buffer covered by the span.
    //!
    //! \param[in]  span    The span to subtract from.
    //! \param[in]  offset  What to subtract from the span.
    //!
    //! \returns  A new span with the offset decreased by the requested amount.
    //!
    template <typename T>
    constexpr Span<T> operator- (Span<T> span, const size_t offset) noexcept {
        return span -= offset;
    }
}
