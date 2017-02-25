
# n_ranges_linear

*Copyright© 2017 Jeremy Jurksztowicz - jurksztowicz@gmail.com*

License is **MIT**-style, suitable for open source or closed source, free or commercial usage. See [LICENSE.md](LICENSE.md) for details.

## SUMMARY

A simple STL style library allowing you to divide a discrete range into a number of subranges, each with an equal number of elements +/-1, where ranges with a greater number of elements are distributed linearly across the results.

To traverse a sequence in *almost equally* sized regions:

```c++
template<typename RandomIter, typename IterRangeFunc>
void for_n_ranges_linear (
    RandomIter      begin,
    RandomIter      end,
    const size_t    ranges_size,            // The desired number of output ranges
    const size_t    distribution_offset,    // See discussion below
    IterRangeFunc   range_func              // A function of the form (size_t, Iter, Iter)
)
```

To transform a sequence in *almost equally* sized regions:

```c++
template<typename RandomIter, typename OutputIter, typename IterRangeFunc>
void transform_n_ranges_linear (
    RandomIter      begin,
    RandomIter      end,
    OutputIter      output_iter,
    const size_t    ranges_size,            // The desired number of output transformations
    const size_t    distribution_offset,    // See discussion below
    IterRangeFunc   range_func              // A function of the form (Iter, Iter) -> T
)
```

## DESCRIPTION

Given a sequence of some length, how can we divide up the elements into ranges so that each range has the same amount of elements +/-1, and the elements are distributed so that ranges with the same size do not "clump" together?

For example, given a range with 27 elements:

![Sequence, no ranges]
(https://github.com/eclectocrat/stl_n_ranges/blob/master/doc/algorithm-step1.png)

If we want to split it into exactly 6 ranges, using integer division we will get 6 ranges of 4 elements each, and 3 remainder elements:

![Sequence, ranges and remainder]
(https://github.com/eclectocrat/stl_n_ranges/blob/master/doc/algorithm-step2.png)

The `n_ranges_linear` functions generate ranges so that integer remainders are distributed linearly across the resulting ranges without affecting sequence order, like so:

![Sequence, ranges solved]
(https://github.com/eclectocrat/stl_n_ranges/blob/master/doc/algorithm-step3.png)

#### `distribution_offset` parameter

In the above example, you may have noticed that ranges with an extra element come before those without. You may want to offset the distribution of your remainder to manipulate where larger ranges end up in your output. For example, with a `distribution_offset` value of 1 (or any value `x` where `x % remainder_size == 1`), the above ranges look like this:

![Sequence, ranges solved, distribution offset]
(https://github.com/eclectocrat/stl_n_ranges/blob/master/doc/algorithm-distribution-offset.png)

## EXAMPLES

To walk through a simple example program that uses `n_ranges_linear` to draw a waveform image with various analysis dimensions, see [EXAMPLE.md](example/EXAMPLE.md).

#### Drawing a waveform

A canonical (and motivating) example is compressing an arbitrary number of wave samples into a fixed frame width for visual display. The accompanying [example program](example/EXAMPLE.md) uses `n_ranges_linear` to draw a waveform like the one shown here:

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
