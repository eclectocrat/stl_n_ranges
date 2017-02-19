# example.cpp

The included example.cpp program uses `n_ranges_linear` functions to generate a waveform image from an included example.wav file. The following steps are performed:

1. Use included RIFF library to read a RIFF wave file from an `istream`, copy the file samples into a `vector<unsigned char>`.
2. Apply `transform_n_ranges_linear()` to the vector to output analysis data.
3. Finally draw the analysis data into a bitmap file.

### Open RIFF stream, seek to wave data, copy data

You can take a look at the extremely minimal RIFF file parser in [RIFF.h](RIFF.h) for more details.

```c++
    RIFF::file_data header;
    istream& ist = open_RIFF_file("example.wav", header);
    
    vector<peak<unsigned char>> peaks;
    vector<unsigned char> data;
    peaks.reserve(width);
    data.reserve(header.size);
    
    copy(
    	istream_iterator<unsigned char>(ist), 
    	istream_iterator<unsigned char>(), 
        back_inserter(data)
    );
```   
   
### Use `transform_n_ranges_linear`

Use the range iterators to get analysis data. In this case we don't want to mutate the sequence, so when calculating mean average we make a mutable copy to destructively analyze.
   
```c++
    ec::transform_n_ranges_linear(
    	data.begin(), 
        data.end(), 
        back_inserter(peaks), 
        width,
    [](auto begin, auto end) -> peak<unsigned char> {
    
    // min/max
        const auto minmax = minmax_element(begin, end);
    
    // slope
        double slope = 1.0;
        if(minmax.first != minmax.second) {
            const auto first = minmax.first < minmax.second 
            	? minmax.first 
                : minmax.second;
            const auto second = minmax.first < minmax.second 
            	? minmax.second 
                : minmax.first;
            slope = double(*second - *first)/double(distance(first, second));
        }
        
    // mean average
        const auto avg = accumulate(begin, end, 0)/distance(begin, end);
    
    // median
        vector<unsigned char> mutable_copy;
        mutable_copy.reserve(distance(begin, end));
        copy(begin, end, back_inserter(mutable_copy));
        const auto med = median(mutable_copy.begin(), mutable_copy.end()));
    
    // done:
        return peak<unsigned char>(
        	*minmax.second, 
            *minmax.first, 
            avg, 
            static_cast<unsigned char>(med),
            slope
    	);
    });
```

### Draw analysis data into a bitmap

Draw and save the image using the [CIImg](http://cimg.eu) library.

```c++
	// draw image using the CImg library:
    auto image = cimg_library::CImg<unsigned char>(width, height, 1, 3, 0);
    image.fill(0xff);
    
    const size_t v_center = height/2;
    const double v_amplitude = double(v_center)/127.0;
    
    for(int x=0; x<width; ++x) {
        const auto maxy = v_amplitude * peaks[x].max;
        const auto miny = v_amplitude * peaks[x].min;
        const auto avgy = v_amplitude * peaks[x].avg;
        const auto medy = v_amplitude * peaks[x].med;
        const size_t length = abs(peaks[x].max - peaks[x].min);
        
        // adjust wave peak color based on slope.
        color::wave[2] = peaks[x].slope * 0xff;
        color::high[2] = peaks[x].slope * 0xff;
        
        image.draw_rectangle(x, maxy, x+1, miny, color::wave);
        image.draw_rectangle(x, maxy+length/4, x+1, maxy+length/2, color::high);
        image.draw_rectangle(x, avgy , x+1, avgy+1, color::avg);
        image.draw_rectangle(x, medy , x+1, medy+1, color::med);
    }
    image.save("./output.bmp");
```
