// This code is based on the CUDA code described in
// the documentation of the "CUDAMCML" package which can be found here:
// http://www.atomic.physics.lu.se/fileadmin/atomfysik/Biophotonics/Software/CUDAMCML.pdf

#ifndef MWCRNG_INIT_H
#define MWCRNG_INIT_H

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif 
#include <inttypes.h>

#include <cstdio>
#include <cstring>
#include <stdint.h>
#include <string>
#include <boost/filesystem.hpp>
#include <icetray/open.h>

#include "phys-services/I3RandomService.h"

// Initialize random number generator 
inline int init_MWC_RNG(uint64_t *x, uint32_t *a, 
                 const uint32_t n_rng,
                 I3RandomServicePtr randomService,
                 std::string safeprimes_file="")
{
    // uint32_t fora,tmp1,tmp2;
    // int64_t forab;
    int64_t multiplier;

    if (safeprimes_file == "")
    {
        namespace fs = boost::filesystem;
        const fs::path I3_SRC(getenv("I3_SRC"));
        const fs::path I3_DATA(getenv("I3_DATA"));
        if (fs::exists(I3_SRC/"clsim/resources/safeprimes_base32.txt"))
            safeprimes_file = (I3_SRC/"/clsim/resources/safeprimes_base32.txt").string();
        else if (fs::exists(I3_DATA/"safeprimes_base32.txt"))
            safeprimes_file = (I3_DATA/"safeprimes_base32.txt").string();
        else
            // Try to find it in the local directory
            safeprimes_file = "safeprimes_base32.txt";
    }
    
    boost::iostreams::filtering_istream ifs;
    I3::dataio::open(ifs, safeprimes_file);
    if (!ifs.good()) {
        log_error("Could not find the safeprimes file (%s)! Terminating!", safeprimes_file.c_str());
        return 1;
    }
    
    bool plaintext = false;
    {
        // Detect newer binary file format
        char tag[18];
        ifs.read(tag, 17);
        tag[17] = '\0';
        if (strcmp(tag, "safeprimes_base32") != 0) {
            plaintext = true;
            I3::dataio::open(ifs, safeprimes_file);
        }
    }

    // Here we set up a loop, using the first multiplier in the file to generate x's and c's
    // There are some restictions to these two numbers:
    // 0<=c<a and 0<=x<b, where a is the multiplier and b is the base (2^32)
    // also [x,c]=[0,0] and [b-1,a-1] are not allowed.
    
    for (uint32_t i=0;i < n_rng;i++) {
        if (ifs.eof())
            log_error("File ended before %u primes could be read!", i+1);
        if (plaintext) {
            ifs >> multiplier;
            if (ifs.fail()) {
                log_error("Couldn't parse prime at line %u", i+1);
                return 1;
            }
            char c;
            while (!ifs.eof() && ifs.peek() != '\n')
                ifs.read(&c, 1);
        } else {
            ifs.read(reinterpret_cast<char*>(&multiplier), sizeof(multiplier));
            if (ifs.fail()) {
                log_error("Couldn't read prime #%u", i+1);
                return 1;
            }
        }
        
        if (multiplier < std::numeric_limits<uint32_t>::min() || multiplier > std::numeric_limits<uint32_t>::max()) {
            log_error("Prime #%u (%ld) is out of range!", i+1, multiplier);
            return 1;
        }
        
        a[i]=multiplier; // primes from file go to a[]

        // generate x[] from the supplied rng
        x[i]=0;
        while( (x[i]==0) | (((uint32_t)(x[i]>>32))>=(a[i]-1)) | (((uint32_t)x[i])>=0xfffffffful))
        {
            // generate a random numbers for x and x (both are stored in the "x" array
            x[i] = static_cast<uint32_t>(randomService->Integer(0xffffffff));
            x[i]=x[i]<<32;
            x[i] += static_cast<uint32_t>(randomService->Integer(0xffffffff));
        }
    }
    
    return 0;
}

#endif
