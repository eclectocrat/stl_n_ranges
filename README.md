
# n_ranges_linear

*Copyright© 2017 Jeremy Jurksztowicz - jurksztowicz@gmail.com*

License is **MIT** style, suitable for open source or closed source, free or commercial usage. See [LICENSE.md](LICENSE.md) for details.

## SUMMARY

A simple STL style library allowing you to divide a discrete range into a number of subranges, each with an equal number of elements +/-1, where ranges with a greater number of elements are distributed linearly across the results.

To traverse a sequence in *almost equally* sized regions:

```c++
template<typename RandomIter, typename IterRangeFunc>
pair<size_t, size_t> for_n_ranges_linear (
    RandomIter      begin,
    RandomIter      end,
    const size_t    ranges_size, // The desired number of output ranges
    IterRangeFunc   range_func   // A function of the form (size_t, Iter, Iter)
)
```

To transform a sequence in *almost equally* sized regions:

```c++
template<typename RandomIter, typename OutputIter, typename IterRangeFunc>
pair<size_t, size_t> transform_n_ranges_linear (
    RandomIter      begin,
    RandomIter      end,
    OutputIter      output_iter,
    const size_t    ranges_size, // The desired number of output transformations
    IterRangeFunc   range_func   // A function of the form (Iter, Iter) -> T
)
```

## EXAMPLES

To walk through a simple example program that uses `n_ranges_linear` to draw a waveform image with various analysis dimensions, see [EXAMPLE.md](example/EXAMPLE.md).

#### Drawing a waveform

A canonical (and motivating) example is compressing an arbitrary number of wave samples into a fixed frame width for visual display. The accompanying example program uses `n_ranges_linear` to draw a waveform like the one shown here:

![Example output image]
(https://github.com/eclectocrat/stl_n_ranges/blob/master/example/example_output.png)

Here is a simplified call to get peaks and mean average:

```c++
transform_n_ranges_linear(samples.begin(), samples.end(), back_inserter(peaks), window_width,
[](auto begin, auto end) -> peak<unsigned char> {
    const auto minmax = minmax_element(begin, end);
    const auto avg = accumulate(begin, end, 0)/distance(begin, end);
    return peak<unsigned char>(*minmax.second, *minmax.first, avg);
});
```

The above snippet will create a compression of samples into wave peaks. Integer division remainders are distributed linearly along the entire output sequence, ensuring that the resulting view is as smooth as possible within the given output size.


#### Distributing tasks to workers

Distributing an unknown number of tasks among a fixed amount of workers as equally as possible:

```c++
for_n_ranges_linear(tasks.begin(), tasks.end(), workers.size(),
[&workers](size_t range_index, auto begin, auto end) {
    for_each(begin, end,
    [&, range_index](auto const& task) {
        workers[range_index].schedule(task);
    });
});
```

#### Splitting flat data

There are many ways to split some flat bytes into regions representing objects; `transform_n_ranges_linear()` provides one more.

Given a `vector<int> raw`: where `raw.size()%3 == 0` we can generate a `vector<point<int, 3>>`:

```c++
vector<point<int, 3>> points;
transform_n_ranges_linear(raw.begin(), raw.end(), back_inserter(points), raw.size()/3,
[](auto begin, auto end) -> point<int, 3> {
    assert(distance(begin, end) == 3);
    const auto x=*begin, y=*(begin+1), z=*(begin+2);
    return point<int, 3>(x, y, z);
});
```
