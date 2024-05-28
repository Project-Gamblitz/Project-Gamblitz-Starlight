/**
 * @file result.h
 * @brief Switch result code tools.
 * @copyright libnx Authors
 */
#pragma once
#include "types.h"

/// Checks whether a result code indicates success.
#define R_SUCCEEDED(res) ((res) == 0)
/// Checks whether a result code indicates failure.
#define R_FAILED(res) ((res) != 0)
/// Returns the module ID of a result code.
#define R_MODULE(res) ((res)&0x1FF)
/// Returns the description of a result code.
#define R_DESCRIPTION(res) (((res) >> 9) & 0x1FFF)
/// Masks out unused bits in a result code, retrieving the actual value for use in comparisons.
#define R_VALUE(res) ((res)&0x3FFFFF)

/// Evaluates an expression that returns a result, and returns the result if it would fail.
#define R_TRY(res_expr)                      \
    ({                                       \
        const auto _tmp_r_try_rc = res_expr; \
        if (R_FAILED(_tmp_r_try_rc)) {       \
            return _tmp_r_try_rc;            \
        }                                    \
    })

/// Builds a result code from its constituent components.
#define MAKERESULT(module, description) ((((module)&0x1FF)) | ((description)&0x1FFF) << 9)

/// Builds a kernel error result code.
#define KERNELRESULT(description) MAKERESULT(Module_Kernel, KernelError_##description)