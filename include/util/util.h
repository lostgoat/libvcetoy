/* * Copyright (C) 2018 Valve Software
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall
 * be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */



#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exception>
#include <memory>

/**
 * This file contains some helpers that may be useful between different
 * projects
 */

namespace util
{
class TerminationException : public std::exception
{
};

/**
 * Print a message and terminate the program
 */
static inline void die(const char* format, ...)
{
    va_list vargs;
    va_start(vargs, format);
    vfprintf(stderr, format, vargs);
    va_end(vargs);
    throw TerminationException();
}

class FailureException : public std::exception
{
};

/**
 * Print an error message about a failed condition
 *
 * Format is compatible with vim quickfix
 */
#define ConditionErrorMsg(condition) \
    do {                                                        \
        fprintf(stderr, "%s:%d: Failed condition check '%s': ", \
                __FILE__, __LINE__, #condition );               \
    } while (0)

/**
 * Print a warning message
 */
#define Warn(format, ...) WarnOn( true, format, ##__VA_ARGS__ );

/**
 * Print a warning message
 */
#define WarnOn(condition, format, ...)                         \
    do {                                                       \
        if (condition) {                                       \
            ConditionErrorMsg(condition);                      \
            fprintf(stderr, format, ##__VA_ARGS__);            \
        }                                                      \
    } while (0)

/**
 * Print an error message and exit
 */
#define FailOn(condition, format, ...)                         \
    do {                                                       \
        if (condition) {                                       \
            ConditionErrorMsg(condition);                      \
            fprintf(stderr, format, ##__VA_ARGS__);            \
            throw util::FailureException();                    \
        }                                                      \
    } while (0)

/**
 * Print an error message and jump to a label if condition is true
 */
#define FailOnTo(condition, label, format, ...)                \
    do {                                                       \
        if (condition) {                                       \
            ConditionErrorMsg(condition);                      \
            fprintf(stderr, format, ##__VA_ARGS__);            \
            goto label;                                        \
        }                                                      \
    } while (0)

/**
 * Quietly jump on error to a label
 */
#define FailOnToQ( condition, label )                          \
    do {                                                       \
        if (condition) {                                       \
            goto label;                                        \
        }                                                      \
    } while (0)

/**
 * Because the C pre-processor is awesome
 */
#define CONCATENATE_IMPL(s1, s2) s1##s2
#define CONCATENATE(s1, s2) CONCATENATE_IMPL(s1, s2)

/**
 * Simple make_unique sample implementation from cppreference
 */
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

/**
 * Get the size of an array
 */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/**
 * Align value up to alignment
 */
#define ALIGN(value, alignment) (((value) + alignment - 1) & ~(alignment - 1))

/**
 * Clamp value to max
 */
#define CLAMP(value, min, max) ((value < min) ? min : ((value > max) ? max : value))


//-----------------------------------------------------------------------------

/**
 * The following section contains Andrei Alexandrescu's scope guard
 * implementation as described in his CppCon talk “Declarative Control Flow"
 *
 * The talk can be found here:
 * https://www.youtube.com/watch?v=WjTrfoiB0MQ
 */

#ifdef __cpp_lib_uncaught_exceptions

/**
 * Allow the creation of an anonymous variable
 */
#ifdef __COUNTER__
#define ANONYMOUS_VARIABLE(str) CONCATENATE(str, __COUNTER__)
#else
#define ANONYMOUS_VARIABLE(str) CONCATENATE(str, __LINE__)
#endif

/**
 * Execute a lambda expression when exiting the scope
 */
#define SCOPE_EXIT                              \
    auto ANONYMOUS_VARIABLE(SCOPE_EXIT_STATE) = \
        ::util::ScopeGuardOnExit() + [&]() noexcept

/**
 * Execute a lambda expression only if we are exiting the
 * scope due to an exception
 */
#define SCOPE_FAIL                              \
    auto ANONYMOUS_VARIABLE(SCOPE_FAIL_STATE) = \
        ::util::ScopeGuardOnFail() + [&]() noexcept

/**
 * Execute a lambda expression only if we are exiting the
 * scope due without encountering an exception
 */
#define SCOPE_SUCCESS \
    auto ANONYMOUS_VARIABLE(SCOPE_SUCCESS_STATE) = ::util::ScopeGuardOnSuccess() + [&]()

class UncaughtExceptionCounter
{
    int getUncaughtExceptionCount() noexcept;
    int exceptionCount_;

  public:
    UncaughtExceptionCounter() : exceptionCount_(std::uncaught_exceptions()) {}
    bool isNewUncaughtException() noexcept
    {
        return std::uncaught_exceptions() > exceptionCount_;
    }
};

typedef enum ExecuteType {
    ExecuteAlways = 0,  // EXIT
    ExecuteOnFail,      // FAIL
    ExecuteOnSuccess    // SUCCESS
} ExecuteType;

template <typename FunctionType, ExecuteType executeOnException>
class ScopeGuardForNewException
{
  private:
    FunctionType function_;
    UncaughtExceptionCounter ec_;

  public:
    ScopeGuardForNewException(const FunctionType& fn) : function_(fn){};
    ScopeGuardForNewException(FunctionType&& fn) : function_(std::move(fn)){};
    ~ScopeGuardForNewException() noexcept(executeOnException)
    {
        bool shouldExec = false;

        switch (executeOnException) {
            case ExecuteAlways:
                shouldExec = true;
                break;
            case ExecuteOnFail:
                shouldExec = ec_.isNewUncaughtException();
                break;
            case ExecuteOnSuccess:
                shouldExec = !ec_.isNewUncaughtException();
                break;
            default:
                die("Invalid scope ExecuteType\n");
        }

        if (shouldExec)
            function_();
    }
};

enum class ScopeGuardOnExit {};

template <typename FunctionType>
ScopeGuardForNewException<typename std::decay<FunctionType>::type, ExecuteAlways>
operator+(util::ScopeGuardOnExit, FunctionType&& fn)
{
    return ScopeGuardForNewException<typename std::decay<FunctionType>::type,
                                     ExecuteAlways>(std::forward<FunctionType>(fn));
}

enum class ScopeGuardOnFail {};

template <typename FunctionType>
ScopeGuardForNewException<typename std::decay<FunctionType>::type, ExecuteOnFail>
operator+(util::ScopeGuardOnFail, FunctionType&& fn)
{
    return ScopeGuardForNewException<typename std::decay<FunctionType>::type,
                                     ExecuteOnFail>(std::forward<FunctionType>(fn));
}

enum class ScopeGuardOnSuccess {};

template <typename FunctionType>
ScopeGuardForNewException<typename std::decay<FunctionType>::type, ExecuteOnSuccess>
operator+(util::ScopeGuardOnSuccess, FunctionType&& fn)
{
    return ScopeGuardForNewException<typename std::decay<FunctionType>::type,
                                     ExecuteOnSuccess>(std::forward<FunctionType>(fn));
}

#endif

//-----------------------------------------------------------------------------

/**
 * This section contains utility functions for dealing with images
 *
 * Data is returned in a heap pointer that the caller is responsible for freeing.
 *
 * Data is in RGBA format
 *
 * A very naive implementation
 *
 * Returns: pointer to data on success, nullptr on failure
 */
static inline uint8_t *GetBmpData( const char *path, uint32_t *pWidth, uint32_t *pHeight, uint32_t *pStrideBytes )
{
    static const int kHeaderSize = 54;

    FILE* bmp = nullptr;
    uint32_t height, width, tmpStride, rgbaStride, offset, bitcount, bpp;
    uint8_t header[kHeaderSize];
    uint8_t *tmpData = nullptr, *rgbaData = nullptr;
    size_t bytesRead = 0;

    FailOnTo( !path, error, "GetBmpData: invalid path\n" );
    FailOnTo( !pWidth || !pHeight || !pStrideBytes , error, "GetBmpData: invalid parameter\n" );

    bmp = fopen( path, "rb" );
    FailOnTo( !bmp, error, "GetBmpData: failed to open file %s\n", path );

    // Read image info from header
    bytesRead = fread(header, sizeof(uint8_t), kHeaderSize, bmp);
    FailOnTo( bytesRead != kHeaderSize, error, "GetBmpData: bad header\n" );

    // Parse the relevant header info
    offset = *(uint32_t*)   &header[10];
    width = *(uint32_t*)    &header[18];
    height = *(uint32_t*)   &header[22];
    bitcount = *(uint32_t*) &header[28];
    bpp = bitcount == 24 ? 3 : 4;
    tmpStride = ALIGN( width * bpp, 4 );

    tmpData = ( uint8_t* ) malloc( tmpStride * height );
    memset( tmpData, 0, sizeof(uint8_t) * tmpStride * height );

    // Get the file data
    fseek( bmp, offset, SEEK_SET );
    bytesRead = fread( tmpData, sizeof(uint8_t), tmpStride * height, bmp);
    FailOnTo( bytesRead != tmpStride * height , error, "GetBmpData: unexpected EOF\n" );

    // Format it to a standard RGBA
    rgbaStride = ALIGN( width *  4 , 4 );
    rgbaData = ( uint8_t* ) malloc( rgbaStride * height );
    memset( rgbaData, 0, sizeof(uint8_t) * rgbaStride * height );

    for ( unsigned i = 0; i < height; ++i ) {
        uint8_t *tmpRow = tmpData + ( i * tmpStride );
        uint8_t *rgbaRow = rgbaData + ( i * rgbaStride );

        for ( unsigned j = 0; j < width; ++j ) {
            uint32_t B = tmpRow[ j * bpp + 0];
            uint32_t G = tmpRow[ j * bpp + 1];
            uint32_t R = tmpRow[ j * bpp + 2];
            uint32_t A = 0;  // ignored

            rgbaRow[ j * 4 + 0 ] = R;
            rgbaRow[ j * 4 + 1 ] = G;
            rgbaRow[ j * 4 + 2 ] = B;
            rgbaRow[ j * 4 + 3 ] = A;
        }
    }

    *pWidth = width;
    *pHeight = height;
    *pStrideBytes = rgbaStride;

    fclose( bmp );
    free( tmpData );

    return rgbaData;

error:
    if ( bmp )
        fclose( bmp );

    free( tmpData );
    free( rgbaData );

    return nullptr;
}

/**
 * Read a bitmap as NV21 data
 *
 * Data is returned in a heap pointer that the caller is responsible for freeing.
 *
 * Data is in NV21 format
 *
 * A very naive implementation
 *
 * Returns: pointer to data on success, nullptr on failure
 */
static inline uint8_t *GetNV21Data( const char *path, uint32_t widthAlignment, uint32_t heightAlignment, uint32_t *pWidth, uint32_t *pHeight )
{
    static const float kNv21Bpp = 1.5;

    size_t size;
    uint32_t width, height, bmpStrideBytes;
    uint32_t alignedWidth, alignedHeight;
    uint8_t *yuvData = nullptr;
    uint8_t *yData = nullptr;
    uint8_t *uvData = nullptr;
    uint8_t *rgbaData = nullptr;
    int32_t R, G, B, Y, U, V;

    FailOnTo( !path || !pWidth || !pHeight , error, "GetNV21Data: invalid parameter\n" );

    rgbaData = GetBmpData( path, &width, &height, &bmpStrideBytes);
    FailOnTo( !rgbaData, error, "GetNV21Data: Failed to read bmp\n" );

    alignedWidth = ALIGN( width, widthAlignment );
    alignedHeight = ALIGN( height, heightAlignment );
    size = alignedWidth * alignedHeight * kNv21Bpp;

    yuvData = ( uint8_t* )malloc( size );
    memset( yuvData, 0 , size );

    yData = yuvData;
    uvData = yuvData + ( alignedWidth * alignedHeight );

    for ( unsigned i = 0; i < height; i++ ) {
        uint8_t *rgbaRow = rgbaData + ( i * bmpStrideBytes );
        uint8_t *yRow = yData + ( i * alignedWidth );
        uint8_t *uvRow = uvData + ( i/2 * alignedWidth );
        int uvIndex = 0;

        for ( unsigned j = 0; j < width; j++ ) {
            R = rgbaRow[ j * 4 ];
            G = rgbaRow[ j * 4 + 1];
            B = rgbaRow[ j * 4 + 2];

            // RGB to YUV conversion
            Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
            U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
            V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;

            // NV21 is semi-planar:
            //  8 bit luminance sample plane, Y
            //  8 bit interleaved 2x2 subsampled chroma planes, V->U
            yRow[ j ] = CLAMP( Y, 0, 255 );
            if ( i % 2 == 0 && j % 2 == 0) {
                uvRow[ uvIndex++ ] = CLAMP( U, 0, 255 );
                uvRow[ uvIndex++ ] = CLAMP( V, 0, 255 );
            }
        }
    }

    *pWidth = width;
    *pHeight = height;

    free( rgbaData );
    return yuvData;

error:
    free( rgbaData );
    free( yuvData );
    return nullptr;
}

//-----------------------------------------------------------------------------
};  // namespace util
