//
//  main.cpp
//  compress_algo
//
//  Created by Jeremy Jurksztowicz on 2/16/17.
//  Copyright © 2017 Jeremy Jurksztowicz. All rights reserved.
//

#include <string>
#include <iostream>
#include <fstream>
#include <memory>
#include <thread>
#include <future>

#define cimg_display 0
#define DOCTEST_CONFIG_COLORS_NONE
#define DOCTEST_CONFIG_IMPLEMENT

#include "lib/CImg.h"
#include "lib/doctest.h"
#include "lib/RIFF.h"

// algorithm
#include "n_ranges_linear.h"

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

using namespace std;

// forward interface declarations
int program_main(int argc, char** argv);
ifstream& open_RIFF_file(string const& path, RIFF::file_data& out_data);

// private details
namespace {
    std::unique_ptr<std::ifstream> file_ptr;
    
    namespace color {
        unsigned char wave[] = {100, 100, 0};
        unsigned char high[] = {180, 155, 0};
        const unsigned char avg [] = {0, 200, 200};
        const unsigned char med [] = {0, 100, 200};
    }
    
    const size_t width = 1000, height = 200;
    
    template<typename T>
    struct peak {
        peak(T n, T x, T a, T d, double m): min(n), max(x), avg(a), med(d), slope(m) {}
        
            T min, max, avg, med;
        double slope;
    };
    
    template<class RandomIter>
    double median(RandomIter begin, RandomIter end) {
        assert(begin < end);
        const size_t size = distance(begin, end);
        const size_t middle = size/2;
        auto target = begin + middle;
        nth_element(begin, target, end);

        if(size % 2 != 0) { // Odd number of elements
            return *target;
        }
        else { // Even number of elements
            const double a = *target;
            auto targetNeighbor = target-1;
            nth_element(begin, targetNeighbor, end);
            return (a + *targetNeighbor)/2.0;
        }
    }
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

// test harness: see program_main().
int main(int argc, char** argv) {

    doctest::Context context;
    context.setOption("no-breaks", true);
    context.applyCommandLine(argc, argv);
    context.setOption("abort-after", 5);
    context.setOption("sort", "name");

    const int test_return_code = context.run();
    if(context.shouldExit()) {
        return test_return_code;
    }
    return test_return_code + program_main(argc, argv);
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

int program_main(int argc, char** argv) {

// get data:
    RIFF::file_data header;
    istream& ist = open_RIFF_file("example.wav", header);
    
    typedef future<peak<unsigned char>> peak_future;
    
    vector<peak_future> peak_futures;
    vector<unsigned char> data;
    peak_futures.reserve(width);
    data.reserve(header.size);
    
    copy(istream_iterator<unsigned char>(ist), istream_iterator<unsigned char>(), back_inserter(data));
    
    // close input file, we are finished reading
    file_ptr.reset();
    
    
// wave peak algorithm:
    cout << "Compressing " << header.size << " samples into " << width << " peaks at ~" << (header.size/width) << " samples per peak." << endl;

    ec::transform_n_ranges_linear(data.begin(), data.end(), back_inserter(peak_futures), width,
    [](auto begin, auto end) -> peak_future {
        return async(launch::async, [=]() -> peak<unsigned char> {
        // min/max
            const auto minmax = minmax_element(begin, end);
        
        // slope
            double slope = 1.0;
            if(minmax.first != minmax.second) {
                const auto first = minmax.first < minmax.second ? minmax.first : minmax.second;
                const auto second = minmax.first < minmax.second ? minmax.second : minmax.first;
                slope = double(*second - *first)/double(distance(first, second));
            }
            
        // mean average
            const auto avg = accumulate(begin, end, 0)/distance(begin, end);
        
        // median
            vector<unsigned char> mutable_copy;
            mutable_copy.reserve(distance(begin, end));
            copy(begin, end, back_inserter(mutable_copy));
            const unsigned char med = static_cast<unsigned char>(median(mutable_copy.begin(), mutable_copy.end()));
        
        // done:
            return peak<unsigned char>(*minmax.second, *minmax.first, avg, med, slope);
        });
    });
    
    vector<peak<unsigned char>> peaks;
    peaks.reserve(peak_futures.size());
    
    transform(peak_futures.begin(), peak_futures.end(), back_inserter(peaks), [](auto& f) -> peak<unsigned char> {
        return f.get();
    });
    
// draw image:
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
    
    
// cleanup:
    return 0;
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

ifstream& open_RIFF_file(string const& path, RIFF::file_data& out_data) {

    assert(not file_ptr); // Only allowed one file open at a time!
    
    file_ptr = unique_ptr<ifstream>(new ifstream(path));
    
    assert(file_ptr and not file_ptr->bad());
    
    out_data = RIFF::seek_RIFF_data(*file_ptr);
    
    assert(out_data.format == RIFF::format::PCM);
    assert(out_data.size > 0);
    assert(out_data.channels == 1);
    assert(out_data.sample_rate == 11025);
    assert(out_data.bits_per_sample == 8);
    
    return *file_ptr;
}
