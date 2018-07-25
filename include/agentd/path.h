/**
 * \file agentd/path.h
 *
 * \brief Utility methods for resolving paths.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_PATH_HEADER_GUARD
#define AGENTD_PATH_HEADER_GUARD

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Append the default path onto a given path.
 *
 * On success, outpath is updated to the appended path.  This value is owned by
 * the caller and must be free()d when no longer needed.
 *
 * \param path              The path to append.
 * \param outpath           The character pointer that this parameter points to
 *                          is updated to the appended path on success.
 *
 * \returns 0 on success and non-zero on failure.
 */
int path_append_default(const char* path, char** outpath);

/**
 * \brief Given a filename and a path, attempt to resolve the filename using
 * this path.
 *
 * On success, resolved is updated to the resolved filename.  This value is
 * owned by the caller and must be free()d when no longer needed.
 *
 * \param filename          The filename to resolve.
 * \param path              A colon separated list of path values.
 * \param resolved          The character pointer that this parameter points to
 *                          is updated to the resolved path on success.
 *
 * \returns 0 on success and non-zero on failure.
 */
int path_resolve(const char* filename, const char* path, char** resolved);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_PATH_HEADER_GUARD*/
