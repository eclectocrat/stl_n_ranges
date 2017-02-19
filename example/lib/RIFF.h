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

#ifndef RIFF_H
#define RIFF_H

#include <iostream>

namespace RIFF {

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

/** Identifiers for RIFF formats.
*/
enum class format {
    bad                 = 0x00,
    PCM                 = 0x01,
    IEEE_floating_point = 0x03,
    a_law               = 0x06,
    mu_law              = 0x07,
    IMAADPCM            = 0x11,
    yamaha_ITUG723ADPCM = 0x16,
    GSM610              = 0x31,
    ITUG721ADPCM        = 0x40,
    MPEG                = 0x50,
    extensible          = 0xFFFE
};


/** Simple RIFF file data structure.
*/
struct file_data {
    format format;
    
    int32_t size;
    int32_t sample_rate;
    int16_t channels;
    int16_t bits_per_sample;
    
    file_data(): format(format::bad), size(0), sample_rate(0), channels(0), bits_per_sample(0) {}
};


/** Raw bits formatted input helper.
*/
template<typename T>
struct rawbits {
    rawbits() { bytes = storage; }
    rawbits(T& v) { bytes = reinterpret_cast<char*>(&v); }
    
    operator T () const { return *reinterpret_cast<T*>(bytes); }
    
    const size_t size = sizeof(T);
    char storage[sizeof(T)];
    char * bytes;
};

template<typename T> std::istream& operator >> (std::istream& ist, rawbits<T> b) { return ist.read(b.bytes, b.size); }
template<typename T> bool operator == (rawbits<T> const& b, std::string const& str) { return str == std::string(b.bytes, b.size); }
template<typename T> bool operator != (rawbits<T> const& b, std::string const& str) { return str != std::string(b.bytes, b.size); }
template<typename T> rawbits<T> wrap_raw(T& v) { return rawbits<T>(v); }

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

/** Reads from the given istream, parsing data until the read pointer is at the first byte of the wave data or parsing fails.
 
    @param ist The open istream reading from the first byte of the RIFF header.
    @return A RIFF_file_data object. If the `format` variable == format::bad, all other values are suspect. 
    
    @note You shouldn't trust the values that you get back, it's quite easy to make a malformed file.
*/
inline file_data seek_RIFF_data(std::istream& ist) {
    using namespace std;

#define RIFF_CHECKED_INPUT(expr) if(not (expr)) { data.format=format::bad; return data; }
    
    rawbits<int32_t> chunk_id;
    bool found_data_chunk = false;
    file_data data;
    
    RIFF_CHECKED_INPUT((ist >> chunk_id) and chunk_id == "RIFF");
    
    while (not found_data_chunk) {
        if(chunk_id == "fmt ") {
            rawbits<int32_t> format_size, bytes_per_second;
            rawbits<int16_t> format_block_align, format;
            RIFF_CHECKED_INPUT(ist
                >> format_size
                >> format
                >> wrap_raw(data.channels)
                >> wrap_raw(data.sample_rate)
                >> bytes_per_second
                >> format_block_align
                >> wrap_raw(data.bits_per_sample)
            );
            data.format = static_cast<enum format>(int16_t(format));
            if(format_size == 18) {
                rawbits<int16_t> extra_data;
                RIFF_CHECKED_INPUT((ist >> extra_data) and ist.seekg(extra_data, ios_base::seekdir::cur));
            }
        }
        else if(chunk_id == "RIFF") {
            rawbits<int32_t> mem_size, riff_style;
            RIFF_CHECKED_INPUT(ist >> mem_size >> riff_style);
        }
        else if(chunk_id == "data") {
            found_data_chunk = true;
            RIFF_CHECKED_INPUT(ist >> wrap_raw(data.size));
        }
        else {
            rawbits<int32_t> skip_size;
            RIFF_CHECKED_INPUT((ist >> skip_size) and ist.seekg(skip_size, ios_base::seekdir::cur));
        }
        
        if(not found_data_chunk) {
            RIFF_CHECKED_INPUT(ist >> chunk_id);
        }
    }
    
#undef RIFF_CHECKED_INPUT

    return data;
}

} // END namespace RIFF

#endif /* RIFF_H */
