
*** n_ranges_linear v1.0 ***

author: Jeremy Jurksztowicz
contact: jurksztowicz@gmail.com

A simple STL style library allowing you to divide a discrete range into a number of subranges, each
with an equal number of elements +/-1, where ranges with a greater number of elements are
distributed linearly across the resulting ranges.

License is MIT, suitable for open source or closed source, free or commercial usage.
See accompanying LICENSE.txt or contact jurksztowicz@gmail.com for complete license information.


*** examples ***

* A canonical (and motivating) example is compressing an arbitrary number of wave samples into a
fixed frame width for visual display:

    transform_n_ranges_linear(samples.begin(), samples.end(), back_inserter(peaks), window_width,
    [](auto begin, auto end) -> peak<unsigned char> {
        const auto minmax = minmax_element(begin, end);
        const auto avg = accumulate(begin, end, 0)/distance(begin, end);
        return peak<unsigned char>(*minmax.second, *minmax.first, avg);
    });

The above snippet will create a compression of samples into wave peaks. Integer division remainders
are distributed linearly along the entire output sequence, ensuring that the resulting view is as
smooth as possible within the given output size.

* Given a vector<int> raw: where raw.size()%3 == 0 we can generate a vector<point<int, 3>>:
        
    vector<point<int, 3>> points;
    transform_n_ranges_linear(raw.begin(), raw.end(), back_inserter(points), raw.size()/3,
    [](auto begin, auto end) -> point<int, 3> {
        assert(distance(begin, end) == 3);
        const auto x=*begin, y=*(begin+1), z=*(begin+2);
        return point<int, 3>(x, y, z);
    });

* Distributing an unknown number of tasks among workers as equally as possible:
        
    for_n_ranges_linear(tasks.begin(), tasks.end(), workers.size(),
    [&workers](size_t range_index, auto begin, auto end) {
        for_each(begin, end,
        [&, range_index](auto const& task) {
            workers[range_index].schedule(task);
        });
    });
