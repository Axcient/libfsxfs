/*
 * Volume functions
 *
 * Copyright (C) 2020-2025, Joachim Metz <joachim.metz@gmail.com>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <memory.h>
#include <narrow_string.h>
#include <types.h>
#include <wide_string.h>

#include "libfsxfs_debug.h"
#include "libfsxfs_definitions.h"
#include "libfsxfs_directory_entry.h"
#include "libfsxfs_file_entry.h"
#include "libfsxfs_file_system.h"
#include "libfsxfs_inode.h"
#include "libfsxfs_io_handle.h"
#include "libfsxfs_libcdata.h"
#include "libfsxfs_libcerror.h"
#include "libfsxfs_libcnotify.h"
#include "libfsxfs_libcthreads.h"
#include "libfsxfs_libuna.h"
#include "libfsxfs_superblock.h"
#include "libfsxfs_volume.h"

/* Creates a volume
 * Make sure the value volume is referencing, is set to NULL
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_volume_initialize(
     libfsxfs_volume_t **volume,
     libcerror_error_t **error )
{
	libfsxfs_internal_volume_t *internal_volume = NULL;
	static char *function                       = "libfsxfs_volume_initialize";

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	if( *volume != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid volume value already set.",
		 function );

		return( -1 );
	}
	internal_volume = memory_allocate_structure(
	                   libfsxfs_internal_volume_t );

	if( internal_volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create volume.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     internal_volume,
	     0,
	     sizeof( libfsxfs_internal_volume_t ) ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear volume.",
		 function );

		memory_free(
		 internal_volume );

		return( -1 );
	}
	if( libfsxfs_io_handle_initialize(
	     &( internal_volume->io_handle ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create IO handle.",
		 function );

		goto on_error;
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_initialize(
	     &( internal_volume->read_write_lock ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to initialize read/write lock.",
		 function );

		goto on_error;
	}
#endif
	*volume = (libfsxfs_volume_t *) internal_volume;

	return( 1 );

on_error:
	if( internal_volume != NULL )
	{
		if( internal_volume->io_handle != NULL )
		{
			libfsxfs_io_handle_free(
			 &( internal_volume->io_handle ),
			 NULL );
		}
		memory_free(
		 internal_volume );
	}
	return( -1 );
}

/* Frees a volume
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_volume_free(
     libfsxfs_volume_t **volume,
     libcerror_error_t **error )
{
	libfsxfs_internal_volume_t *internal_volume = NULL;
	static char *function                       = "libfsxfs_volume_free";
	int result                                  = 1;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	if( *volume != NULL )
	{
		internal_volume = (libfsxfs_internal_volume_t *) *volume;

		if( internal_volume->file_io_handle != NULL )
		{
			if( libfsxfs_volume_close(
			     *volume,
			     error ) != 0 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_IO,
				 LIBCERROR_IO_ERROR_CLOSE_FAILED,
				 "%s: unable to close volume.",
				 function );

				result = -1;
			}
		}
		*volume = NULL;

#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
		if( libcthreads_read_write_lock_free(
		     &( internal_volume->read_write_lock ),
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free read/write lock.",
			 function );

			result = -1;
		}
#endif
		if( libfsxfs_io_handle_free(
		     &( internal_volume->io_handle ),
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free IO handle.",
			 function );

			result = -1;
		}
		memory_free(
		 internal_volume );
	}
	return( result );
}

/* Signals the volume to abort its current activity
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_volume_signal_abort(
     libfsxfs_volume_t *volume,
     libcerror_error_t **error )
{
	libfsxfs_internal_volume_t *internal_volume = NULL;
	static char *function                       = "libfsxfs_volume_signal_abort";

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libfsxfs_internal_volume_t *) volume;

	if( internal_volume->io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid volume - missing IO handle.",
		 function );

		return( -1 );
	}
	internal_volume->io_handle->abort = 1;

	return( 1 );
}

/* Opens a volume
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_volume_open(
     libfsxfs_volume_t *volume,
     const char *filename,
     int access_flags,
     libcerror_error_t **error )
{
	libbfio_handle_t *file_io_handle            = NULL;
	libfsxfs_internal_volume_t *internal_volume = NULL;
	static char *function                       = "libfsxfs_volume_open";

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libfsxfs_internal_volume_t *) volume;

	if( filename == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filename.",
		 function );

		return( -1 );
	}
	if( ( ( access_flags & LIBFSXFS_ACCESS_FLAG_READ ) == 0 )
	 && ( ( access_flags & LIBFSXFS_ACCESS_FLAG_WRITE ) == 0 ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported access flags.",
		 function );

		return( -1 );
	}
	if( ( access_flags & LIBFSXFS_ACCESS_FLAG_WRITE ) != 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: write access currently not supported.",
		 function );

		return( -1 );
	}
	if( libbfio_file_initialize(
	     &file_io_handle,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create file IO handle.",
		 function );

		goto on_error;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libbfio_handle_set_track_offsets_read(
	     file_io_handle,
	     1,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set track offsets read in file IO handle.",
		 function );

		goto on_error;
	}
#endif
	if( libbfio_file_set_name(
	     file_io_handle,
	     filename,
	     narrow_string_length(
	      filename ) + 1,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set filename in file IO handle.",
		 function );

		goto on_error;
	}
	if( libfsxfs_volume_open_file_io_handle(
	     volume,
	     file_io_handle,
	     access_flags,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_OPEN_FAILED,
		 "%s: unable to open volume: %s.",
		 function,
		 filename );

		goto on_error;
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		goto on_error;
	}
#endif
	internal_volume->file_io_handle_created_in_library = 1;

#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		internal_volume->file_io_handle_created_in_library = 0;

		goto on_error;
	}
#endif
	return( 1 );

on_error:
	if( file_io_handle != NULL )
	{
		libbfio_handle_free(
		 &file_io_handle,
		 NULL );
	}
	return( -1 );
}

#if defined( HAVE_WIDE_CHARACTER_TYPE )

/* Opens a volume
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_volume_open_wide(
     libfsxfs_volume_t *volume,
     const wchar_t *filename,
     int access_flags,
     libcerror_error_t **error )
{
	libbfio_handle_t *file_io_handle            = NULL;
	libfsxfs_internal_volume_t *internal_volume = NULL;
	static char *function                       = "libfsxfs_volume_open_wide";

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libfsxfs_internal_volume_t *) volume;

	if( filename == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filename.",
		 function );

		return( -1 );
	}
	if( ( ( access_flags & LIBFSXFS_ACCESS_FLAG_READ ) == 0 )
	 && ( ( access_flags & LIBFSXFS_ACCESS_FLAG_WRITE ) == 0 ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported access flags.",
		 function );

		return( -1 );
	}
	if( ( access_flags & LIBFSXFS_ACCESS_FLAG_WRITE ) != 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: write access currently not supported.",
		 function );

		return( -1 );
	}
	if( libbfio_file_initialize(
	     &file_io_handle,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create file IO handle.",
		 function );

		goto on_error;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libbfio_handle_set_track_offsets_read(
	     file_io_handle,
	     1,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set track offsets read in file IO handle.",
		 function );

		goto on_error;
	}
#endif
	if( libbfio_file_set_name_wide(
	     file_io_handle,
	     filename,
	     wide_string_length(
	      filename ) + 1,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set filename in file IO handle.",
		 function );

		goto on_error;
	}
	if( libfsxfs_volume_open_file_io_handle(
	     volume,
	     file_io_handle,
	     access_flags,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_OPEN_FAILED,
		 "%s: unable to open volume: %ls.",
		 function,
		 filename );

		goto on_error;
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		goto on_error;
	}
#endif
	internal_volume->file_io_handle_created_in_library = 1;

#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		internal_volume->file_io_handle_created_in_library = 0;

		goto on_error;
	}
#endif
	return( 1 );

on_error:
	if( file_io_handle != NULL )
	{
		libbfio_handle_free(
		 &file_io_handle,
		 NULL );
	}
	return( -1 );
}

#endif /* defined( HAVE_WIDE_CHARACTER_TYPE ) */

/* Opens a volume using a Basic File IO (bfio) handle
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_volume_open_file_io_handle(
     libfsxfs_volume_t *volume,
     libbfio_handle_t *file_io_handle,
     int access_flags,
     libcerror_error_t **error )
{
	libfsxfs_internal_volume_t *internal_volume = NULL;
	static char *function                       = "libfsxfs_volume_open_file_io_handle";
	uint8_t file_io_handle_opened_in_library    = 0;
	int bfio_access_flags                       = 0;
	int file_io_handle_is_open                  = 0;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libfsxfs_internal_volume_t *) volume;

	if( internal_volume->file_io_handle != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid volume - file IO handle already set.",
		 function );

		return( -1 );
	}
	if( file_io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file IO handle.",
		 function );

		return( -1 );
	}
	if( ( ( access_flags & LIBFSXFS_ACCESS_FLAG_READ ) == 0 )
	 && ( ( access_flags & LIBFSXFS_ACCESS_FLAG_WRITE ) == 0 ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported access flags.",
		 function );

		return( -1 );
	}
	if( ( access_flags & LIBFSXFS_ACCESS_FLAG_WRITE ) != 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: write access currently not supported.",
		 function );

		return( -1 );
	}
	if( ( access_flags & LIBFSXFS_ACCESS_FLAG_READ ) != 0 )
	{
		bfio_access_flags = LIBBFIO_ACCESS_FLAG_READ;
	}
	file_io_handle_is_open = libbfio_handle_is_open(
	                          file_io_handle,
	                          error );

	if( file_io_handle_is_open == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_OPEN_FAILED,
		 "%s: unable to open volume.",
		 function );

		goto on_error;
	}
	else if( file_io_handle_is_open == 0 )
	{
		if( libbfio_handle_open(
		     file_io_handle,
		     bfio_access_flags,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_OPEN_FAILED,
			 "%s: unable to open file IO handle.",
			 function );

			goto on_error;
		}
		file_io_handle_opened_in_library = 1;
	}
	if( libfsxfs_internal_volume_open_read(
	     internal_volume,
	     file_io_handle,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read from file IO handle.",
		 function );

		goto on_error;
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		goto on_error;
	}
#endif
	internal_volume->file_io_handle                   = file_io_handle;
	internal_volume->file_io_handle_opened_in_library = file_io_handle_opened_in_library;

#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		internal_volume->file_io_handle                   = NULL;
		internal_volume->file_io_handle_opened_in_library = 0;

		goto on_error;
	}
#endif
	return( 1 );

on_error:
	if( file_io_handle_opened_in_library != 0 )
	{
		libbfio_handle_close(
		 file_io_handle,
		 error );
	}
	return( -1 );
}

/* Closes a volume
 * Returns 0 if successful or -1 on error
 */
int libfsxfs_volume_close(
     libfsxfs_volume_t *volume,
     libcerror_error_t **error )
{
	libfsxfs_internal_volume_t *internal_volume = NULL;
	static char *function                       = "libfsxfs_volume_close";
	int result                                  = 0;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libfsxfs_internal_volume_t *) volume;

	if( internal_volume->file_io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid volume - missing file IO handle.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		if( internal_volume->file_io_handle_created_in_library != 0 )
		{
			if( libfsxfs_debug_print_read_offsets(
			     internal_volume->file_io_handle,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
				 "%s: unable to print the read offsets.",
				 function );

				result = -1;
			}
		}
	}
#endif
	if( internal_volume->file_io_handle_opened_in_library != 0 )
	{
		if( libbfio_handle_close(
		     internal_volume->file_io_handle,
		     error ) != 0 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_CLOSE_FAILED,
			 "%s: unable to close file IO handle.",
			 function );

			result = -1;
		}
		internal_volume->file_io_handle_opened_in_library = 0;
	}
	if( internal_volume->file_io_handle_created_in_library != 0 )
	{
		if( libbfio_handle_free(
		     &( internal_volume->file_io_handle ),
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free file IO handle.",
			 function );

			result = -1;
		}
		internal_volume->file_io_handle_created_in_library = 0;
	}
	internal_volume->file_io_handle = NULL;

	if( libfsxfs_io_handle_clear(
	     internal_volume->io_handle,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to clear IO handle.",
		 function );

		result = -1;
	}
	if( internal_volume->superblock != NULL )
	{
		if( libfsxfs_superblock_free(
		     &( internal_volume->superblock ),
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free superblock.",
			 function );

			result = -1;
		}
	}
	if( internal_volume->file_system != NULL )
	{
		if( libfsxfs_file_system_free(
		     &( internal_volume->file_system ),
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free file system.",
			 function );

			result = -1;
		}
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Opens a volume for reading
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_internal_volume_open_read(
     libfsxfs_internal_volume_t *internal_volume,
     libbfio_handle_t *file_io_handle,
     libcerror_error_t **error )
{
	libfsxfs_superblock_t *superblock = NULL;
	static char *function             = "libfsxfs_internal_volume_open_read";
	off64_t allocation_group_size     = 0;
	off64_t inode_information_offset  = 0;
	off64_t superblock_offset         = 0;
	uint32_t allocation_group_index   = 0;

	if( internal_volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid internal volume.",
		 function );

		return( -1 );
	}
	if( internal_volume->io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal volume - missing IO handle.",
		 function );

		return( -1 );
	}
	if( internal_volume->superblock != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid volume - superblock value already set.",
		 function );

		return( -1 );
	}
	if( internal_volume->file_system != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid volume - file system value already set.",
		 function );

		return( -1 );
	}
	do
	{
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "Reading superblock: %" PRIu32 " at offset: %" PRIi64 " (0x%08" PRIx64 ").\n",
			 allocation_group_index,
			 superblock_offset,
			 superblock_offset );
		}
#endif
		if( libfsxfs_superblock_initialize(
		     &superblock,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create superblock: %" PRIu32 ".",
			 function,
			 allocation_group_index );

			goto on_error;
		}
		if( libfsxfs_superblock_read_file_io_handle(
		     superblock,
		     file_io_handle,
		     superblock_offset,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read superblock: %" PRIu32 " at offset: %" PRIi64 " (0x%08" PRIx64 ").",
			 function,
			 allocation_group_index,
			 superblock_offset,
			 superblock_offset );

			goto on_error;
		}
		if( internal_volume->superblock == NULL )
		{
			internal_volume->superblock                                      = superblock;
			internal_volume->io_handle->format_version                       = superblock->format_version;
			internal_volume->io_handle->secondary_feature_flags              = superblock->secondary_feature_flags;
			internal_volume->io_handle->block_size                           = superblock->block_size;
			internal_volume->io_handle->allocation_group_size                = superblock->allocation_group_size;
			internal_volume->io_handle->inode_size                           = superblock->inode_size;
			internal_volume->io_handle->directory_block_size                 = superblock->directory_block_size;
			internal_volume->io_handle->number_of_relative_block_number_bits = superblock->number_of_relative_block_number_bits;
			internal_volume->io_handle->number_of_relative_inode_number_bits = superblock->number_of_relative_inode_number_bits;

			superblock = NULL;
		}
		inode_information_offset = superblock_offset + 2 * internal_volume->superblock->sector_size;

#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "Reading inode information: %" PRIu32 " at offset: %" PRIi64 " (0x%08" PRIx64 ").\n",
			 allocation_group_index,
			 inode_information_offset,
			 inode_information_offset );
		}
#endif
		if( internal_volume->file_system == NULL )
		{
			if( libfsxfs_file_system_initialize(
			     &( internal_volume->file_system ),
			     internal_volume->superblock->root_directory_inode_number,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
				 "%s: unable to create file system.",
				 function );

				goto on_error;
			}
		}
		if( libfsxfs_file_system_read_inode_information(
		     internal_volume->file_system,
		     internal_volume->io_handle,
		     file_io_handle,
		     inode_information_offset,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read inode B+ tree.",
			 function );

			goto on_error;
		}
		if( superblock != NULL )
		{
			if( libfsxfs_superblock_free(
			     &superblock,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				 "%s: unable to free superblock.",
				 function );

				goto on_error;
			}
		}
		allocation_group_size = (off64_t) internal_volume->io_handle->allocation_group_size * internal_volume->io_handle->block_size;

		if( ( allocation_group_size == 0 )
		 || ( superblock_offset > ( (off64_t) INT64_MAX - allocation_group_size ) ) )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
			 "%s: invalid next superblock offset value out of bounds.",
			 function );

			goto on_error;
		}
		superblock_offset += allocation_group_size;

		allocation_group_index++;
	}
	while( allocation_group_index < internal_volume->superblock->number_of_allocation_groups );

	if( allocation_group_index != internal_volume->superblock->number_of_allocation_groups )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid last allocation group index value out of bounds.",
		 function );

		goto on_error;
	}
	return( 1 );

on_error:
	if( internal_volume->file_system != NULL )
	{
		libfsxfs_file_system_free(
		 &( internal_volume->file_system ),
		 NULL );
	}
	if( internal_volume->superblock != NULL )
	{
		libfsxfs_superblock_free(
		 &( internal_volume->superblock ),
		 NULL );
	}
	if( superblock != NULL )
	{
		libfsxfs_superblock_free(
		 &superblock,
		 NULL );
	}
	return( -1 );
}

/* Retrieves the format version
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_volume_get_format_version(
     libfsxfs_volume_t *volume,
     uint8_t *format_version,
     libcerror_error_t **error )
{
	libfsxfs_internal_volume_t *internal_volume = NULL;
	static char *function                       = "libfsxfs_volume_get_format_version";

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libfsxfs_internal_volume_t *) volume;

	if( internal_volume->io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid volume - missing IO handle.",
		 function );

		return( -1 );
	}
	if( format_version == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid format version.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	*format_version = internal_volume->io_handle->format_version;

#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( 1 );
}

/* Retrieves the size of the UTF-8 encoded label
 * The returned size includes the end of string character
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_volume_get_utf8_label_size(
     libfsxfs_volume_t *volume,
     size_t *utf8_string_size,
     libcerror_error_t **error )
{
	libfsxfs_internal_volume_t *internal_volume = NULL;
	static char *function                       = "libfsxfs_volume_get_utf8_label_size";
	int result                                  = 1;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libfsxfs_internal_volume_t *) volume;

	if( internal_volume->superblock == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal volume - missing superblock.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( libfsxfs_superblock_get_utf8_volume_label_size(
	     internal_volume->superblock,
	     utf8_string_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve UTF-8 volume label string size.",
		 function );

		result = -1;
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves the UTF-8 encoded label
 * The size should include the end of string character
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_volume_get_utf8_label(
     libfsxfs_volume_t *volume,
     uint8_t *utf8_string,
     size_t utf8_string_size,
     libcerror_error_t **error )
{
	libfsxfs_internal_volume_t *internal_volume = NULL;
	static char *function                       = "libfsxfs_volume_get_utf8_label";
	int result                                  = 1;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libfsxfs_internal_volume_t *) volume;

	if( internal_volume->superblock == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal volume - missing superblock.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( libfsxfs_superblock_get_utf8_volume_label(
	     internal_volume->superblock,
	     utf8_string,
	     utf8_string_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve UTF-8 volume label string.",
		 function );

		result = -1;
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves the size of the UTF-16 encoded label
 * The returned size includes the end of string character
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_volume_get_utf16_label_size(
     libfsxfs_volume_t *volume,
     size_t *utf16_string_size,
     libcerror_error_t **error )
{
	libfsxfs_internal_volume_t *internal_volume = NULL;
	static char *function                       = "libfsxfs_volume_get_utf16_label_size";
	int result                                  = 1;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libfsxfs_internal_volume_t *) volume;

	if( internal_volume->superblock == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal volume - missing superblock.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( libfsxfs_superblock_get_utf16_volume_label_size(
	     internal_volume->superblock,
	     utf16_string_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve UTF-16 volume label string size.",
		 function );

		result = -1;
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves the UTF-16 encoded label
 * The size should include the end of string character
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_volume_get_utf16_label(
     libfsxfs_volume_t *volume,
     uint16_t *utf16_string,
     size_t utf16_string_size,
     libcerror_error_t **error )
{
	libfsxfs_internal_volume_t *internal_volume = NULL;
	static char *function                       = "libfsxfs_volume_get_utf16_label";
	int result                                  = 1;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libfsxfs_internal_volume_t *) volume;

	if( internal_volume->superblock == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal volume - missing superblock.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( libfsxfs_superblock_get_utf16_volume_label(
	     internal_volume->superblock,
	     utf16_string,
	     utf16_string_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve UTF-16 volume label string.",
		 function );

		result = -1;
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves the root directory file entry
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_volume_get_root_directory(
     libfsxfs_volume_t *volume,
     libfsxfs_file_entry_t **file_entry,
     libcerror_error_t **error )
{
	libfsxfs_inode_t *inode                     = NULL;
	libfsxfs_internal_volume_t *internal_volume = NULL;
	static char *function                       = "libfsxfs_volume_get_root_directory";
	int result                                  = 1;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libfsxfs_internal_volume_t *) volume;

	if( internal_volume->superblock == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal volume - missing superblock.",
		 function );

		return( -1 );
	}
	if( file_entry == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	if( *file_entry != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid file entry value already set.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	if( libfsxfs_file_system_get_inode_by_number(
	     internal_volume->file_system,
	     internal_volume->io_handle,
	     internal_volume->file_io_handle,
	     internal_volume->superblock->root_directory_inode_number,
	     &inode,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve inode: %" PRIu64 ".",
		 function,
		 internal_volume->superblock->root_directory_inode_number );

		result = -1;
	}
	else if( libfsxfs_file_entry_initialize(
	          file_entry,
	          internal_volume->io_handle,
	          internal_volume->file_io_handle,
	          internal_volume->file_system,
	          internal_volume->superblock->root_directory_inode_number,
	          inode,
	          NULL,
	          error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create file entry.",
		 function );

		libfsxfs_inode_free(
		 &inode,
		 NULL );

		result = -1;
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		libfsxfs_file_entry_free(
		 file_entry,
		 NULL );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves the file entry of a specific inode
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_internal_volume_get_file_entry_by_inode(
     libfsxfs_internal_volume_t *internal_volume,
     uint64_t inode_number,
     libfsxfs_file_entry_t **file_entry,
     libcerror_error_t **error )
{
	libfsxfs_inode_t *inode = NULL;
	static char *function   = "libfsxfs_internal_volume_get_file_entry_by_inode";

	if( internal_volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	if( file_entry == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	if( *file_entry != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid file entry value already set.",
		 function );

		return( -1 );
	}
	if( libfsxfs_file_system_get_inode_by_number(
	     internal_volume->file_system,
	     internal_volume->io_handle,
	     internal_volume->file_io_handle,
	     inode_number,
	     &inode,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve inode: %" PRIu64 ".",
		 function,
		 inode_number );

		goto on_error;
	}
	/* libfsxfs_file_entry_initialize takes over management of inode
	 */
	if( libfsxfs_file_entry_initialize(
	     file_entry,
	     internal_volume->io_handle,
	     internal_volume->file_io_handle,
	     internal_volume->file_system,
	     inode_number,
	     inode,
	     NULL,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create file entry.",
		 function );

		goto on_error;
	}
	return( 1 );

on_error:
	if( inode != NULL )
	{
		libfsxfs_inode_free(
		 &inode,
		 NULL );
	}
	return( -1 );
}

/* Retrieves the file entry of a specific inode
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_volume_get_file_entry_by_inode(
     libfsxfs_volume_t *volume,
     uint64_t inode_number,
     libfsxfs_file_entry_t **file_entry,
     libcerror_error_t **error )
{
	libfsxfs_internal_volume_t *internal_volume = NULL;
	static char *function                       = "libfsxfs_volume_get_file_entry_by_inode";
	int result                                  = 1;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libfsxfs_internal_volume_t *) volume;

	if( file_entry == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	if( *file_entry != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid file entry value already set.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBFSEXT_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	if( libfsxfs_internal_volume_get_file_entry_by_inode(
	     internal_volume,
	     inode_number,
	     file_entry,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve inode: %" PRIu64 ".",
		 function,
		 inode_number );

		result = -1;
	}
#if defined( HAVE_LIBFSEXT_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		libfsxfs_file_entry_free(
		 file_entry,
		 NULL );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves the file entry for an UTF-8 encoded path
 * Returns 1 if successful, 0 if no such file entry or -1 on error
 */
int libfsxfs_internal_volume_get_file_entry_by_utf8_path(
     libfsxfs_internal_volume_t *internal_volume,
     const uint8_t *utf8_string,
     size_t utf8_string_length,
     libfsxfs_file_entry_t **file_entry,
     libcerror_error_t **error )
{
	libfsxfs_directory_entry_t *directory_entry = NULL;
	libfsxfs_inode_t *inode                     = NULL;
	static char *function                       = "libfsxfs_internal_volume_get_file_entry_by_utf8_path";
	uint64_t inode_number                       = 0;
	int result                                  = 0;

	if( internal_volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	result = libfsxfs_file_system_get_directory_entry_by_utf8_path(
	          internal_volume->file_system,
	          internal_volume->io_handle,
	          internal_volume->file_io_handle,
	          utf8_string,
	          utf8_string_length,
	          &inode_number,
	          &inode,
	          &directory_entry,
	          error );

	if( result == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve directory entry by UTF-8 path.",
		 function );

		goto on_error;
	}
	else if( result != 0 )
	{
		/* libfsxfs_file_entry_initialize takes over management of inode and directory_entry
		 */
		if( libfsxfs_file_entry_initialize(
		     file_entry,
		     internal_volume->io_handle,
		     internal_volume->file_io_handle,
		     internal_volume->file_system,
		     inode_number,
		     inode,
		     directory_entry,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create file entry.",
			 function );

			goto on_error;
		}
	}
	return( result );

on_error:
	if( inode != NULL )
	{
		libfsxfs_inode_free(
		 &inode,
		 NULL );
	}
	if( directory_entry != NULL )
	{
		libfsxfs_directory_entry_free(
		 &directory_entry,
		 NULL );
	}
	return( -1 );
}

/* Retrieves the file entry for an UTF-8 encoded path
 * Returns 1 if successful, 0 if no such file entry or -1 on error
 */
int libfsxfs_volume_get_file_entry_by_utf8_path(
     libfsxfs_volume_t *volume,
     const uint8_t *utf8_string,
     size_t utf8_string_length,
     libfsxfs_file_entry_t **file_entry,
     libcerror_error_t **error )
{
	libfsxfs_internal_volume_t *internal_volume = NULL;
	static char *function                       = "libfsxfs_volume_get_file_entry_by_utf8_path";
	int result                                  = 0;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libfsxfs_internal_volume_t *) volume;

	if( file_entry == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	if( *file_entry != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid file entry value already set.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	result = libfsxfs_internal_volume_get_file_entry_by_utf8_path(
	          internal_volume,
	          utf8_string,
	          utf8_string_length,
	          file_entry,
	          error );

	if( result == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve file entry by UTF-8 encoded path.",
		 function );

		result = -1;
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		libfsxfs_file_entry_free(
		 file_entry,
		 NULL );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves the file entry for an UTF-16 encoded path
 * Returns 1 if successful, 0 if no such file entry or -1 on error
 */
int libfsxfs_internal_volume_get_file_entry_by_utf16_path(
     libfsxfs_internal_volume_t *internal_volume,
     const uint16_t *utf16_string,
     size_t utf16_string_length,
     libfsxfs_file_entry_t **file_entry,
     libcerror_error_t **error )
{
	libfsxfs_directory_entry_t *directory_entry = NULL;
	libfsxfs_inode_t *inode                     = NULL;
	static char *function                       = "libfsxfs_internal_volume_get_file_entry_by_utf16_path";
	uint64_t inode_number                       = 0;
	int result                                  = 0;

	if( internal_volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	result = libfsxfs_file_system_get_directory_entry_by_utf16_path(
	          internal_volume->file_system,
	          internal_volume->io_handle,
	          internal_volume->file_io_handle,
	          utf16_string,
	          utf16_string_length,
	          &inode_number,
	          &inode,
	          &directory_entry,
	          error );

	if( result == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve directory entry by UTF-16 path.",
		 function );

		goto on_error;
	}
	else if( result != 0 )
	{
		/* libfsxfs_file_entry_initialize takes over management of inode and directory_entry
		 */
		if( libfsxfs_file_entry_initialize(
		     file_entry,
		     internal_volume->io_handle,
		     internal_volume->file_io_handle,
		     internal_volume->file_system,
		     inode_number,
		     inode,
		     directory_entry,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create file entry.",
			 function );

			goto on_error;
		}
	}
	return( result );

on_error:
	if( inode != NULL )
	{
		libfsxfs_inode_free(
		 &inode,
		 NULL );
	}
	if( directory_entry != NULL )
	{
		libfsxfs_directory_entry_free(
		 &directory_entry,
		 NULL );
	}
	return( -1 );
}

/* Retrieves the file entry for an UTF-16 encoded path
 * Returns 1 if successful, 0 if no such file entry or -1 on error
 */
int libfsxfs_volume_get_file_entry_by_utf16_path(
     libfsxfs_volume_t *volume,
     const uint16_t *utf16_string,
     size_t utf16_string_length,
     libfsxfs_file_entry_t **file_entry,
     libcerror_error_t **error )
{
	libfsxfs_internal_volume_t *internal_volume = NULL;
	static char *function                       = "libfsxfs_volume_get_file_entry_by_utf16_path";
	int result                                  = 0;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libfsxfs_internal_volume_t *) volume;

	if( file_entry == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	if( *file_entry != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid file entry value already set.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	result = libfsxfs_internal_volume_get_file_entry_by_utf16_path(
	          internal_volume,
	          utf16_string,
	          utf16_string_length,
	          file_entry,
	          error );

	if( result == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve file entry by UTF-16 encoded path.",
		 function );

		result = -1;
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     internal_volume->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		libfsxfs_file_entry_free(
		 file_entry,
		 NULL );

		return( -1 );
	}
#endif
	return( result );
}

/* Traverses an individual allocation group free-space B+ tree node recursively.
 * Returns 1 if successful or -1 on error.
 */
int libfsxfs_allocation_group_traverse_alloc_tree(
        libbfio_handle_t *file_io_handle,
        uint32_t ag_block_number,
        uint32_t ag_no,
        uint32_t ag_block_cnt,
        uint32_t block_size,
        uint16_t tree_level,
        void *data,
        void (*clear_callback)(void *data, uint32_t start, uint32_t blocks),
        libcerror_error_t **error )
{
    uint8_t *ag_block      = NULL;
    static char *function  = "libfsxfs_allocation_group_traverse_alloc_tree";
    uint64_t physical_offset = 0;
    size_t header_size     = 0;
    ssize_t bytes_read     = 0;
    uint16_t max_records   = 0;
    uint16_t num_records   = 0;
    uint16_t record_index  = 0;
    uint32_t magic_number  = 0;

    if( file_io_handle == NULL )
    {
        libcerror_error_set(
                error,
                LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
                LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
                "%s: invalid file I/O handle.",
                function );

        return( -1 );
    }
    if( clear_callback == NULL )
    {
        libcerror_error_set(
                error,
                LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
                LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
                "%s: invalid clear callback context handler.",
                function );

        return( -1 );
    }

    ag_block = (uint8_t *) memory_allocate( sizeof( uint8_t ) * block_size );

    if( ag_block == NULL )
    {
        libcerror_error_set(
                error,
                LIBCERROR_ERROR_DOMAIN_MEMORY,
                LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
                "%s: unable to allocate allocation group block storage buffer.",
                function );

        return( -1 );
    }

    physical_offset = ( (uint64_t) ag_no * ag_block_cnt * block_size ) + ( (uint64_t) ag_block_number * block_size );

    bytes_read = libbfio_handle_read_buffer_at_offset(
            file_io_handle,
            ag_block,
            block_size,
            (off64_t) physical_offset,
            error );

    if( bytes_read != (ssize_t) block_size )
    {
        libcerror_error_set(
                error,
                LIBCERROR_ERROR_DOMAIN_IO,
                LIBCERROR_IO_ERROR_READ_FAILED,
                "%s: unable to read allocation tree block data at offset: %" PRIu64 ".",
                function,
                physical_offset );

        memory_free( ag_block );
        return( -1 );
    }

    byte_stream_copy_to_uint32_big_endian(
            ( ag_block + LIBFSXFS_BB_OFFSET_MAGIC ),
            magic_number );

    if( magic_number == LIBFSXFS_BB_V5_MAGIC_NUMBER )
    {
        header_size = 56;
        max_records = 505;
    }
    else if( magic_number == LIBFSXFS_BB_V4_MAGIC_NUMBER )
    {
        header_size = 18;
        max_records = 509;
    }
    else
    {
        libcerror_error_set(
                error,
                LIBCERROR_ERROR_DOMAIN_RUNTIME,
                LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
                "%s: unsupported or unrecognized B+ tree short block magic number: 0x%08" PRIx32 ".",
                function,
                magic_number );

        memory_free( ag_block );
        return( -1 );
    }

    byte_stream_copy_to_uint16_big_endian(
            ( ag_block + LIBFSXFS_BB_OFFSET_NUMRECS ),
            num_records );

    if( tree_level > 1 )
    {
        size_t key_size            = 4;
        size_t ptr_size            = 4;
        size_t pointer_array_start = header_size + ( max_records * key_size );

        for( record_index = 0; record_index < num_records; record_index++ )
        {
            uint32_t child_ag_block = 0;
            size_t pointer_offset   = pointer_array_start + ( record_index * ptr_size );

            byte_stream_copy_to_uint32_big_endian( ( ag_block + pointer_offset ), child_ag_block );

            if( libfsxfs_allocation_group_traverse_alloc_tree(
                        file_io_handle,
                        child_ag_block,
                        ag_no,
                        ag_block_cnt,
                        block_size,
                        tree_level - 1,
                        data,
                        clear_callback,
                        error ) != 1 )
            {
                libcerror_error_set(
                        error,
                        LIBCERROR_ERROR_DOMAIN_RUNTIME,
                        LIBCERROR_RUNTIME_ERROR_GET_FAILED,
                        "%s: unable to parse child index node at nested entry point block %" PRIu32 ".",
                        function,
                        child_ag_block );

                memory_free( ag_block );
                return( -1 );
            }
        }
    }
    else if( tree_level == 1 )
    {
        size_t rec_size = 8;

        for( record_index = 0; record_index < num_records; record_index++ )
        {
            uint32_t start_block = 0;
            uint32_t block_count = 0;
            size_t rec_offset    = header_size + ( record_index * rec_size );

            byte_stream_copy_to_uint32_big_endian( ( ag_block + rec_offset ), start_block );
            byte_stream_copy_to_uint32_big_endian( ( ag_block + rec_offset + 4 ), block_count );
            // Convert AG-relative start_block to the physical one
            clear_callback( data, ag_no * ag_block_cnt + start_block, block_count );
        }
    }

    memory_free( ag_block );
    return( 1 );
}

/* Iterates across every active filesystem allocation group, extracting unallocated extents.
 * Returns 1 if successful or -1 on error.
 */
int libfsxfs_volume_free_blocks_iterate(
        libfsxfs_volume_t *volume,
        void *data,
        void (*clear_callback)(void *data, uint32_t start, uint32_t blocks),
        libcerror_error_t **error )
{
    libfsxfs_internal_volume_t *internal_volume = NULL;
    uint8_t *agf_buffer                         = NULL;
    uint8_t *agfl_buffer                        = NULL;
    static char *function                       = "libfsxfs_volume_free_blocks_iterate";
    off64_t ag_offset                           = 0;
    off64_t agf_offset                          = 0;
    off64_t agfl_offset                         = 0;
    ssize_t bytes_read                          = 0;
    size_t agfl_header_size                     = 0;
    uint32_t ag_block_cnt                       = 0;
    uint32_t max_ag_no                          = 0;
    uint32_t block_size                         = 0;
    uint32_t sector_size                        = 0;
    uint32_t max_agfl_slots                     = 0;
    int ag_no                                   = 0;
    int ret                                     = 0;

    if( volume == NULL )
    {
        libcerror_error_set(
                error,
                LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
                LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
                "%s: invalid volume context reference pointer.",
                function );

        return( -1 );
    }

    internal_volume = (libfsxfs_internal_volume_t *) volume;

    ag_block_cnt = internal_volume->superblock->allocation_group_size;
    block_size   = internal_volume->superblock->block_size;
    sector_size  = internal_volume->superblock->sector_size;
    max_ag_no    = internal_volume->superblock->number_of_allocation_groups;

    agf_buffer  = (uint8_t *) memory_allocate( sizeof( uint8_t ) * sector_size );
    agfl_buffer = (uint8_t *) memory_allocate( sizeof( uint8_t ) * sector_size );

    if( ( agf_buffer == NULL ) || ( agfl_buffer == NULL ) )
    {
        libcerror_error_set(
                error,
                LIBCERROR_ERROR_DOMAIN_MEMORY,
                LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
                "%s: unable to allocate local sector structural reference buffers.",
                function );

        if( agf_buffer != NULL ) memory_free( agf_buffer );
        if( agfl_buffer != NULL ) memory_free( agfl_buffer );
        return( -1 );
    }

    for( ag_no = 0; ag_no < (int) max_ag_no; ag_no++ )
    {
        libfsxfs_internal_agf_t agf_metadata;
        libfsxfs_internal_agfl_t agfl_metadata;

        ag_offset   = (off64_t) ag_no * ag_block_cnt * block_size;
        agf_offset  = ag_offset + ( (off64_t) 1 * sector_size );
        agfl_offset = ag_offset + ( (off64_t) 3 * sector_size );

        bytes_read = libbfio_handle_read_buffer_at_offset(
                internal_volume->file_io_handle,
                agf_buffer,
                sector_size,
                agf_offset,
                error );

        if( bytes_read != (ssize_t) sector_size )
        {
            libcerror_error_set(
                    error,
                    LIBCERROR_ERROR_DOMAIN_IO,
                    LIBCERROR_IO_ERROR_READ_FAILED,
                    "%s: unable to read AGF primary metadata sector block at offset %" PRIu64 ".",
                    function,
                    agf_offset );

            goto error_cleanup;
        }

        byte_stream_copy_to_uint32_big_endian( ( agf_buffer + LIBFSXFS_AGF_OFFSET_MAGIC ), agf_metadata.magicnum );

        if( agf_metadata.magicnum != LIBFSXFS_AGF_MAGIC_NUMBER )
        {
            libcerror_error_set(
                    error,
                    LIBCERROR_ERROR_DOMAIN_RUNTIME,
                    LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
                    "%s: invalid or corrupted allocation group framework signature validation map.",
                    function );

            goto error_cleanup;
        }

        byte_stream_copy_to_uint32_big_endian( ( agf_buffer + LIBFSXFS_AGF_OFFSET_SEQUENCE_NO ), agf_metadata.seqno );
        byte_stream_copy_to_uint32_big_endian( ( agf_buffer + LIBFSXFS_AGF_OFFSET_LENGTH ), agf_metadata.length );
        byte_stream_copy_to_uint32_big_endian( ( agf_buffer + LIBFSXFS_AGF_OFFSET_BNOBT_ROOT ), agf_metadata.bnobt_root );
        byte_stream_copy_to_uint32_big_endian( ( agf_buffer + LIBFSXFS_AGF_OFFSET_CNTBT_ROOT ), agf_metadata.cntbt_root );
        byte_stream_copy_to_uint32_big_endian( ( agf_buffer + LIBFSXFS_AGF_OFFSET_BNOBT_LEVEL ), agf_metadata.bnobt_level );
        byte_stream_copy_to_uint32_big_endian( ( agf_buffer + LIBFSXFS_AGF_OFFSET_CNTBT_LEVEL ), agf_metadata.cntbt_level );
        byte_stream_copy_to_uint32_big_endian( ( agf_buffer + LIBFSXFS_AGF_OFFSET_FLFIRST ), agf_metadata.fl_first );
        byte_stream_copy_to_uint32_big_endian( ( agf_buffer + LIBFSXFS_AGF_OFFSET_FLLAST ), agf_metadata.fl_last );
        byte_stream_copy_to_uint32_big_endian( ( agf_buffer + LIBFSXFS_AGF_OFFSET_FLCOUNT ), agf_metadata.fl_count );
        byte_stream_copy_to_uint32_big_endian( ( agf_buffer + LIBFSXFS_AGF_OFFSET_FREE_BLOCKS ), agf_metadata.freeblks );

#if defined( HAVE_DEBUG_OUTPUT )
        libcnotify_printf( "AGF seqno: %" PRIu32 "\n", agf_metadata.seqno );
        libcnotify_printf( "BNOBT root block: %" PRIu32 "\n", agf_metadata.bnobt_root );
        libcnotify_printf( "BNOBT B+ tree height: %" PRIu32 "\n", agf_metadata.bnobt_level );
        libcnotify_printf( "Total AG free blocks: %" PRIu32 "\n", agf_metadata.freeblks );
#endif

        bytes_read = libbfio_handle_read_buffer_at_offset(
                internal_volume->file_io_handle,
                agfl_buffer,
                sector_size,
                agfl_offset,
                error );

        if( bytes_read != (ssize_t) sector_size )
        {
            libcerror_error_set(
                    error,
                    LIBCERROR_ERROR_DOMAIN_IO,
                    LIBCERROR_IO_ERROR_READ_FAILED,
                    "%s: unable to read allocation group free list (AGFL) array payload.",
                    function );

            goto error_cleanup;
        }


#if defined( HAVE_DEBUG_OUTPUT )
        libfsxfs_debug_dump_buffer( agfl_buffer, sector_size );
#endif

        byte_stream_copy_to_uint32_big_endian( ( agfl_buffer + LIBFSXFS_AGFL_OFFSET_MAGIC ), agfl_metadata.agfl_magicnum );
        byte_stream_copy_to_uint32_big_endian( ( agfl_buffer + LIBFSXFS_AGFL_OFFSET_SEQNO ), agfl_metadata.agfl_seqno );

        /* 36-byte V5 header is only present if magic exists */
        if( agfl_metadata.agfl_magicnum == LIBFSXFS_AGFL_MAGIC_NUMBER )
        {
            agfl_header_size = 36;
        }
        else
        {
            agfl_header_size = 0;
        }

        max_agfl_slots = ( sector_size - agfl_header_size ) / sizeof (uint32_t );

#if defined( HAVE_DEBUG_OUTPUT )
        libcnotify_printf(
                "fl_start: %" PRIu32 ", fl_count: %" PRIu32 ", fl_last: %" PRIu32 "\n",
                agf_metadata.fl_first,
                agf_metadata.fl_count,
                agf_metadata.fl_last );
#endif

        if( agf_metadata.fl_count > 0 )
        {
            uint32_t slot_idx    = agf_metadata.fl_first;
            uint32_t items_found = 0;

            while( items_found < agf_metadata.fl_count )
            {
                uint32_t active_block = 0;
                size_t cell_offset    = agfl_header_size + ( slot_idx * sizeof( uint32_t ) );

                byte_stream_copy_to_uint32_big_endian( ( agfl_buffer + cell_offset ), active_block );
                if( active_block != 0xffffffff )
                {
                    clear_callback( data, active_block, 1 );
                }

                slot_idx = ( slot_idx + 1 ) % max_agfl_slots;
                items_found++;
            }
        }

        ret = libfsxfs_allocation_group_traverse_alloc_tree(
                internal_volume->file_io_handle,
                agf_metadata.bnobt_root,
                (uint32_t) ag_no,
                ag_block_cnt,
                block_size,
                (uint16_t) agf_metadata.bnobt_level,
                data,
                clear_callback,
                error );

        if( ret != 1 )
        {
            libcerror_error_set(
                    error,
                    LIBCERROR_ERROR_DOMAIN_RUNTIME,
                    LIBCERROR_RUNTIME_ERROR_GET_FAILED,
                    "%s: allocation tree parsing pipeline failure encountered on AG %d.",
                    function,
                    ag_no );

            goto error_cleanup;
        }
    }

    memory_free( agf_buffer );
    memory_free( agfl_buffer );
    return( 1 );

error_cleanup:
    if( agf_buffer != NULL )  memory_free( agf_buffer );
    if( agfl_buffer != NULL ) memory_free( agfl_buffer );
    return( -1 );
}
