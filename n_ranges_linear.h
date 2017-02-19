/*
    Copyright (c) 2017 Jeremy Jurksztowicz

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#ifndef n_ranges_linear_h
#define n_ranges_linear_h

#include <cassert>
#include <cmath>
#include <utility>
#include <iterator>
#include <type_traits>

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest.h>

#include <vector>
#include <string>
#include <numeric>
#include <sstream>
#include <iostream>
#include <exception>
#define assert_true(condition) { if(not (condition)) { \
    throw std::logic_error("n_ranges_linear_h assert violated."); } } // <-- debug assertion here
#else
#define assert_true(condition) assert(condition) // <-- release assertion here
#endif

namespace ec {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
T greatest_common_divisor(T n, T d) {
    return d == 0 ? n : greatest_common_divisor<T>(d, n % d);
}


template<typename T>
std::pair<T, T> positive_ratio(T num, T den) {
    static_assert(std::is_unsigned<T>::value, "positive_ratio() only works on unsigned types.");
    
    const T gcd = greatest_common_divisor(num, den);
    const T result_num = num / gcd;
    const T result_den = den / gcd;
    return std::make_pair(result_num, result_den);
};


/** Visits a sequence of elements in chunks of 'n' 'equalest-possible' ranges.

    Takes a discrete sequence of elements and splits them into roughly equal sized ranges, passing them one at a time to a function taking two random access iterators (begin, end). The resulting ranges are 'distance(begin, end)/ranges_size' in size, with 'distance(begin, end)%ranges_size' ranges having +1 element due to the integer division remainder being distributed linearly across the resulting ranges.
    
    For example, distributing an unknown number of tasks among workers:
        
        for_n_ranges_linear(tasks.begin(), tasks.end(), workers.size(), 
        [&workers](size_t range_index, auto begin, auto end) {
            for_each(begin, end, 
            [&, range_index](auto const& task) {
                workers[range_index].schedule(task);
            });
        });
 
    @param begin The beginning of the input range.
    @param end The end of the input range.
    @param ranges_size The number of ranges to divide this sequence into.
    @param range_func The func that takes a size_t range_index, begin, and end iterators.
    
    @return A pair including:
        - first: The whole number of elements per range.
        - second: TODO (a ratio object describing distribution of remainder along with a progress step in that ratio).
        
    PRECONDITIONS:
        begin <= end
        ranges_size > 0
        ranges_size <= distance(begin, end)
    
    POSTCONDITIONS:
        every element has been ordered into a range and passed to range_func
        return-value.first > 0
*/
template<typename RandomIter, typename IterRangeFunc>
std::pair<size_t, size_t> for_n_ranges_linear (
    RandomIter      begin,
    RandomIter      end,
    const size_t    ranges_size,
    IterRangeFunc   range_func
) {
    using namespace std;

    assert_true(begin <= end);
    
    const size_t input_size = distance(begin, end);

    assert_true(ranges_size > 0);
    assert_true(ranges_size < input_size); // We can only compress, not expand.

    const size_t inputs_per_output = input_size / ranges_size;
    const size_t remainder = input_size % ranges_size;
    
    assert_true(inputs_per_output > 0);
    assert_true(remainder < ranges_size);
    
    const auto remainder_ratio = positive_ratio(remainder, ranges_size);
    for(size_t i = 0; i < ranges_size; ++i) {
        auto b = begin;
        begin += inputs_per_output + ((i % remainder_ratio.second) < remainder_ratio.first ? 1 : 0);
        range_func(i, b, begin);
    }
    
    assert_true(begin == end);

    return std::make_pair(inputs_per_output, 0 /* TODO: steps before adding another remainder here */);
}


/** Transforms a sequence of elements in chunks of 'n' 'equalest-possible' ranges.

    For more about the algorithm @see: for_n_ranges_linear()
    
    For example, given a vector<int> raw: where raw.size()%3 == 0 we can generate a vector<point<int, 3>>:
        
        vector<point<int, 3>> points;
        transform_n_ranges_linear(raw.begin(), raw.end(), back_inserter(points), raw.size()/3, 
        [](auto begin, auto end) -> point<int, 3> {
            assert(distance(begin, end) == 3);
            const auto x=*begin, y=*(begin+1), z=*(begin+2);
            return point<int, 3>(x, y, z);
        });
 
    @param begin The beginning of the input range.
    @param end The end of the input range.
    @param output_iter The write iterator for the results.
    @param ranges_size The number of ranges to divide this sequence into.
    @param range_func Transform func that takes (begin, end) and returns an arbitrary value insertable into output_iter.
    
    @return A pair including:
        - first: The whole number of elements per range.
        - second: TODO (a ratio object describing distribution of remainder along with a progress step in that ratio).
        
    PRECONDITIONS:
    POSTCONDITIONS:
        @see: for_n_ranges_linear()
*/
template<typename RandomIter, typename OutputIter, typename IterRangeFunc>
std::pair<size_t, size_t> transform_n_ranges_linear (
    RandomIter      begin,
    RandomIter      end,
    OutputIter      output_iter,
    const size_t    ranges_size,
    IterRangeFunc   range_func
) {
    return for_n_ranges_linear(begin, end, ranges_size, [&](size_t range_index, auto b, auto e) {
        *output_iter++ = range_func(b, e);
    });
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace test {

using namespace std;

TEST_CASE("[transform_n_ranges_linear] VALID transform_n_ranges_linear(...)") {
    
    vector<int> intin;
    for(int i=0; i<100; ++i) { intin.push_back(i); }
    vector<string> strout;
    vector<int> intout;
    
    SUBCASE("[transform_n_ranges_linear] VALID output size and remainder") {
    
         SUBCASE("[transform_n_ranges_linear] transform_n_ranges_linear(): 100 ints into 50 strings") {
             const auto result = transform_n_ranges_linear(intin.begin(), intin.end(), back_inserter(strout), 50,
             [](auto begin, auto end) -> string {
                 ostringstream ost;
                 for_each(begin, end, [&](int n) { ost << n; });
                 return ost.str();
             });
             CHECK(result.first == 2);
             CHECK(result.second == 0);
             CHECK(strout.size() == 50);
         }
        
         SUBCASE("[transform_n_ranges_linear] transform_n_ranges_linear(): 100 ints into 21 strings") {
             const auto result = transform_n_ranges_linear(intin.begin(), intin.end(), back_inserter(strout), 21,
             [](auto begin, auto end) -> string {
                 ostringstream ost;
                 for_each(begin, end, [&](int n) { ost << n; });
                 return ost.str();
             });
             CHECK(result.first == 4);
             CHECK(result.second == 0);
             CHECK(strout.size() == 21);
         }
    }
}

TEST_CASE("[transform_n_ranges_linear] INVALID transform_n_ranges_linear(...)") {
    
    vector<int> intin;
    for(int i=0; i<100; ++i) { intin.push_back(i); }
    vector<string> strout;
    vector<int> intout;
    
    SUBCASE("[compress_algo] INVALID output size") {
        
        REQUIRE_THROWS(transform_n_ranges_linear(intin.begin(), intin.end(), back_inserter(strout),
            numeric_limits<size_t>::max(), /* <-- Too many regions requested. */
            [](auto begin, auto end) -> string { return ""; }));
        
        REQUIRE_THROWS(transform_n_ranges_linear(intin.begin(), intin.end(), back_inserter(strout),
            0, /* <-- Not enough regions requested. */
            [](auto begin, auto end) -> string { return ""; }));
    }
}

} // END namespace test
} // END namespace ec

#endif // n_ranges_linear_h
