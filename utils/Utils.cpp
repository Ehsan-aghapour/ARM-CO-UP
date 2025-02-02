/*
 * Copyright (c) 2017-2019 Arm Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
//Ehsan
//#include <string>

#include "Utils.h"

#ifdef ARM_COMPUTE_CL
#include "arm_compute/core/CL/CLKernelLibrary.h"
#include "arm_compute/runtime/CL/CLScheduler.h"
#endif /* ARM_COMPUTE_CL */

#include <cctype>
#include <cerrno>
#include <iomanip>
#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wstrict-overflow"
#if (defined(__GNUC__) && (__GNUC__ >= 7))
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif // (defined(__GNUC__) && (__GNUC__ >= 7))
#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wparentheses-equality"
#endif // defined(__clang__)
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#pragma GCC diagnostic pop



namespace arm_compute
{
namespace utils
{
namespace
{
/* Advance the iterator to the first character which is not a comment
 *
 * @param[in,out] fs Stream to drop comments from
 */
void discard_comments(std::ifstream &fs)
{
    while(fs.peek() == '#')
    {
        fs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

/* Advance the string iterator to the next character which is neither a space or a comment
 *
 * @param[in,out] fs Stream to drop comments from
 */
void discard_comments_and_spaces(std::ifstream &fs)
{
    while(true)
    {
        discard_comments(fs);

        if(isspace(fs.peek()) == 0)
        {
            break;
        }

        fs.ignore(1);
    }
}
} // namespace
#define Frequency_Setting 1
#ifndef BENCHMARK_EXAMPLES
int LittleFrequencyTable[] = {408000, 600000, 816000, 1008000, 1200000, 1416000};
int BigFrequencyTable[] = {408000, 600000, 816000, 1008000, 1200000, 1416000, 1608000, 1800000};
int GPUFrequencyTable[] = {200000000, 300000000, 400000000, 600000000, 800000000};
bool get_freqs(int &l, int &b, int &g){
	std::string freqs;
	std::cin>>freqs;
	std::cerr<<"the input freq is "<<freqs<<std::endl;
	//bool finished=false;
	if (freqs=="end"){
		return true;
	}
	if (freqs.size() >= 2 && freqs.front() == '{' && freqs.back() == '}') {
			freqs = freqs.substr(1, freqs.size() - 2);
			if (freqs.size() >= 2 && freqs.front() == '{' && freqs.back() == '}'){
				freqs = freqs.substr(1, freqs.size() - 2);
			    std::replace(freqs.begin(), freqs.end(), '-', ' '); // Replace '-' with whitespace
			    std::istringstream iss(freqs);
			    if (!(iss >> l >> b >> g)) {
			        std::cerr << "Parsing error!" << std::endl;
			        return true;
			    }
			}
			else{
				std::cerr<<"freqs format is not supported\n";
			}
	}
	else{
		l=std::stoi(freqs);
		std::cin>>b;
		std::cin>>g;
	}
	if(l<1000){
		l=LittleFrequencyTable[l];
		b=BigFrequencyTable[b];
		g=GPUFrequencyTable[g];
	}
	return false;
}

int run_example(int argc, char **argv, std::unique_ptr<Example> example)
{
    std::cout << "\n"
              << argv[0] << "\n\n";

    try
    {
#if Frequency_Setting
        system("echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
        system("echo userspace > /sys/devices/system/cpu/cpufreq/policy4/scaling_governor");
        system("echo userspace > /sys/devices/platform/ff9a0000.gpu/devfreq/ff9a0000.gpu/governor");
        int f_i=1;
#endif
        bool status = example->do_setup(argc, argv);
        if(!status)
        {
            return 1;
        }
#if Frequency_Setting
        //Min
        //int LFreq=408000, BFreq=408000, GFreq=200000000;
        //Max
        int LFreq=1416000, BFreq=1800000, GFreq=800000000;

        std::string cmd="";

        /*
        //Set Little CPU Frequency
		cmd="echo " + to_string(LFreq) + " > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed";
		system(cmd.c_str());

		//Set Big CPU Frequency
		cmd="echo " + to_string(BFreq) + " > /sys/devices/system/cpu/cpufreq/policy4/scaling_setspeed";
		system(cmd.c_str());

		//Set GPU Frequency
		cmd="echo " + to_string(GFreq) + " > /sys/devices/platform/ff9a0000.gpu/devfreq/ff9a0000.gpu/userspace/set_freq";
		system(cmd.c_str());
		*/
        /*
        std::cin>>LFreq;
		std::cin>>BFreq;
		std::cin>>GFreq;
		*/
        get_freqs(LFreq, BFreq, GFreq);
        //while (BFreq && LFreq && GFreq){
        bool finish=false;
        while (!finish){

        	std::cerr<<f_i++<<" Running Graph with Frequency: "<<LFreq<<','<<BFreq<<','<<GFreq<<std::endl;
			//Set Little CPU Frequency
			cmd="echo " + to_string(LFreq) + " > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed";
			system(cmd.c_str());
			//Set Big CPU Frequency
			cmd="echo " + to_string(BFreq) + " > /sys/devices/system/cpu/cpufreq/policy4/scaling_setspeed";
			system(cmd.c_str());
			//Set GPU Frequency
			cmd="echo " + to_string(GFreq) + " > /sys/devices/platform/ff9a0000.gpu/devfreq/ff9a0000.gpu/userspace/set_freq";
			system(cmd.c_str());
        	sleep(2);
        	example->do_run();
        	std::cerr<<"enter freqs of Little, big, GPU:\n";
        	/*std::cin>>LFreq;
        	std::cin>>BFreq;
        	std::cin>>GFreq;*/
        	finish=get_freqs(LFreq, BFreq, GFreq);

        }
        example->do_finish();
#else
        example->do_run();
        example->do_finish();
        example->do_teardown();
#endif

        std::cerr << "\nTest passed\n";
        return 0;
    }
#ifdef ARM_COMPUTE_CL
    catch(cl::Error &err)
    {
        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << std::endl
                  << "ERROR " << err.what() << "(" << err.err() << ")" << std::endl;
        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    }
#endif /* ARM_COMPUTE_CL */
    catch(std::runtime_error &err)
    {
        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << std::endl
                  << "ERROR " << err.what() << " " << (errno ? strerror(errno) : "") << std::endl;
        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    }

    std::cout << "\nTest FAILED\n";

    return -1;
}
#endif /* BENCHMARK_EXAMPLES */

void draw_detection_rectangle(ITensor *tensor, const DetectionWindow &rect, uint8_t r, uint8_t g, uint8_t b)
{
    ARM_COMPUTE_ERROR_ON_FORMAT_NOT_IN(tensor, Format::RGB888);

    uint8_t *top    = tensor->info()->offset_element_in_bytes(Coordinates(rect.x, rect.y)) + tensor->buffer();
    uint8_t *bottom = tensor->info()->offset_element_in_bytes(Coordinates(rect.x, rect.y + rect.height)) + tensor->buffer();
    uint8_t *left   = top;
    uint8_t *right  = tensor->info()->offset_element_in_bytes(Coordinates(rect.x + rect.width, rect.y)) + tensor->buffer();
    size_t   stride = tensor->info()->strides_in_bytes()[Window::DimY];

    for(size_t x = 0; x < rect.width; ++x)
    {
        top[0]    = r;
        top[1]    = g;
        top[2]    = b;
        bottom[0] = r;
        bottom[1] = g;
        bottom[2] = b;

        top += 3;
        bottom += 3;
    }

    for(size_t y = 0; y < rect.height; ++y)
    {
        left[0]  = r;
        left[1]  = g;
        left[2]  = b;
        right[0] = r;
        right[1] = g;
        right[2] = b;

        left += stride;
        right += stride;
    }
}

ImageType get_image_type_from_file(const std::string &filename)
{
    ImageType type = ImageType::UNKNOWN;

    try
    {
        // Open file
        std::ifstream fs;
        fs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fs.open(filename, std::ios::in | std::ios::binary);

        // Identify type from magic number
        std::array<unsigned char, 2> magic_number{ { 0 } };
        fs >> magic_number[0] >> magic_number[1];

        // PPM check
        if(static_cast<char>(magic_number[0]) == 'P' && static_cast<char>(magic_number[1]) == '6')
        {
            type = ImageType::PPM;
        }
        else if(magic_number[0] == 0xFF && magic_number[1] == 0xD8)
        {
            type = ImageType::JPEG;
        }

        fs.close();
    }
    catch(std::runtime_error &e)
    {
        ARM_COMPUTE_ERROR_VAR("Accessing %s: %s", filename.c_str(), e.what());
    }

    return type;
}

std::tuple<unsigned int, unsigned int, int> parse_ppm_header(std::ifstream &fs)
{
    // Check the PPM magic number is valid
    std::array<char, 2> magic_number{ { 0 } };
    fs >> magic_number[0] >> magic_number[1];
    ARM_COMPUTE_ERROR_ON_MSG(magic_number[0] != 'P' || magic_number[1] != '6', "Invalid file type");
    ARM_COMPUTE_UNUSED(magic_number);

    discard_comments_and_spaces(fs);

    unsigned int width = 0;
    fs >> width;

    discard_comments_and_spaces(fs);

    unsigned int height = 0;
    fs >> height;

    discard_comments_and_spaces(fs);

    int max_val = 0;
    fs >> max_val;

    discard_comments(fs);

    ARM_COMPUTE_ERROR_ON_MSG(isspace(fs.peek()) == 0, "Invalid PPM header");
    fs.ignore(1);

    return std::make_tuple(width, height, max_val);
}

std::tuple<std::vector<unsigned long>, bool, std::string> parse_npy_header(std::ifstream &fs) //NOLINT
{
    std::vector<unsigned long> shape; // NOLINT

    // Read header
    std::string header = npy::read_header(fs);

    // Parse header
    bool        fortran_order = false;
    std::string typestr;
    npy::parse_header(header, typestr, fortran_order, shape);

    std::reverse(shape.begin(), shape.end());

    return std::make_tuple(shape, fortran_order, typestr);
}

/** This function returns the amount of memory free reading from /proc/meminfo
 *
 * @return The free memory in kB
 */
uint64_t get_mem_free_from_meminfo()
{
    std::string   line_attribute;
    std::ifstream file_meminfo("/proc/meminfo");

    if(file_meminfo.is_open())
    {
        while(!(file_meminfo >> line_attribute).fail())
        {
            //Test if is the line containing MemFree
            if(line_attribute == "MemFree:")
            {
                uint64_t mem_available;
                if(!(file_meminfo >> mem_available).fail())
                {
                    return mem_available;
                }
                else
                {
                    return 0;
                }
            }
            // if it's not MemFree ignore rest of the line
            file_meminfo.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    // Nothing found or an error during opening the file
    return 0;
}
} // namespace utils
} // namespace arm_compute
