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

#if !defined( _LIBFSXFS_INTERNAL_VOLUME_H )
#define _LIBFSXFS_INTERNAL_VOLUME_H

#include <common.h>
#include <types.h>

#include "libfsxfs_extern.h"
#include "libfsxfs_file_system.h"
#include "libfsxfs_inode_btree.h"
#include "libfsxfs_io_handle.h"
#include "libfsxfs_libbfio.h"
#include "libfsxfs_libcdata.h"
#include "libfsxfs_libcerror.h"
#include "libfsxfs_libcthreads.h"
#include "libfsxfs_superblock.h"
#include "libfsxfs_types.h"

#if defined( __cplusplus )
extern "C" {
#endif

typedef struct libfsxfs_internal_volume libfsxfs_internal_volume_t;

struct libfsxfs_internal_volume
{
	/* The superblock
	 */
	libfsxfs_superblock_t *superblock;

	/* The file system
	 */
	libfsxfs_file_system_t *file_system;

	/* The inode B+ tree
	 */
	libfsxfs_inode_btree_t *inode_btree;

	/* The IO handle
	 */
	libfsxfs_io_handle_t *io_handle;

	/* The file IO handle
	 */
	libbfio_handle_t *file_io_handle;

	/* Value to indicate if the file IO handle was created inside the library
	 */
	uint8_t file_io_handle_created_in_library;

	/* Value to indicate if the file IO handle was opened inside the library
	 */
	uint8_t file_io_handle_opened_in_library;

#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	/* The read/write lock
	 */
	libcthreads_read_write_lock_t *read_write_lock;
#endif
};

LIBFSXFS_EXTERN \
int libfsxfs_volume_initialize(
     libfsxfs_volume_t **volume,
     libcerror_error_t **error );

LIBFSXFS_EXTERN \
int libfsxfs_volume_free(
     libfsxfs_volume_t **volume,
     libcerror_error_t **error );

LIBFSXFS_EXTERN \
int libfsxfs_volume_signal_abort(
     libfsxfs_volume_t *volume,
     libcerror_error_t **error );

LIBFSXFS_EXTERN \
int libfsxfs_volume_open(
     libfsxfs_volume_t *volume,
     const char *filename,
     int access_flags,
     libcerror_error_t **error );

#if defined( HAVE_WIDE_CHARACTER_TYPE )

LIBFSXFS_EXTERN \
int libfsxfs_volume_open_wide(
     libfsxfs_volume_t *volume,
     const wchar_t *filename,
     int access_flags,
     libcerror_error_t **error );

#endif /* defined( HAVE_WIDE_CHARACTER_TYPE ) */

LIBFSXFS_EXTERN \
int libfsxfs_volume_open_file_io_handle(
     libfsxfs_volume_t *volume,
     libbfio_handle_t *file_io_handle,
     int access_flags,
     libcerror_error_t **error );

LIBFSXFS_EXTERN \
int libfsxfs_volume_close(
     libfsxfs_volume_t *volume,
     libcerror_error_t **error );

int libfsxfs_internal_volume_open_read(
     libfsxfs_internal_volume_t *internal_volume,
     libbfio_handle_t *file_io_handle,
     libcerror_error_t **error );

LIBFSXFS_EXTERN \
int libfsxfs_volume_get_format_version(
     libfsxfs_volume_t *volume,
     uint8_t *format_version,
     libcerror_error_t **error );

LIBFSXFS_EXTERN \
int libfsxfs_volume_get_utf8_label_size(
     libfsxfs_volume_t *volume,
     size_t *utf8_string_size,
     libcerror_error_t **error );

LIBFSXFS_EXTERN \
int libfsxfs_volume_get_utf8_label(
     libfsxfs_volume_t *volume,
     uint8_t *utf8_string,
     size_t utf8_string_size,
     libcerror_error_t **error );

LIBFSXFS_EXTERN \
int libfsxfs_volume_get_utf16_label_size(
     libfsxfs_volume_t *volume,
     size_t *utf16_string_size,
     libcerror_error_t **error );

LIBFSXFS_EXTERN \
int libfsxfs_volume_get_utf16_label(
     libfsxfs_volume_t *volume,
     uint16_t *utf16_string,
     size_t utf16_string_size,
     libcerror_error_t **error );

LIBFSXFS_EXTERN \
int libfsxfs_volume_get_root_directory(
     libfsxfs_volume_t *volume,
     libfsxfs_file_entry_t **file_entry,
     libcerror_error_t **error );

int libfsxfs_internal_volume_get_file_entry_by_inode(
     libfsxfs_internal_volume_t *internal_volume,
     uint64_t inode_number,
     libfsxfs_file_entry_t **file_entry,
     libcerror_error_t **error );

LIBFSXFS_EXTERN \
int libfsxfs_volume_get_file_entry_by_inode(
     libfsxfs_volume_t *volume,
     uint64_t inode_number,
     libfsxfs_file_entry_t **file_entry,
     libcerror_error_t **error );

int libfsxfs_internal_volume_get_file_entry_by_utf8_path(
     libfsxfs_internal_volume_t *internal_volume,
     const uint8_t *utf8_string,
     size_t utf8_string_length,
     libfsxfs_file_entry_t **file_entry,
     libcerror_error_t **error );

LIBFSXFS_EXTERN \
int libfsxfs_volume_get_file_entry_by_utf8_path(
     libfsxfs_volume_t *volume,
     const uint8_t *utf8_string,
     size_t utf8_string_length,
     libfsxfs_file_entry_t **file_entry,
     libcerror_error_t **error );

int libfsxfs_internal_volume_get_file_entry_by_utf16_path(
     libfsxfs_internal_volume_t *internal_volume,
     const uint16_t *utf16_string,
     size_t utf16_string_length,
     libfsxfs_file_entry_t **file_entry,
     libcerror_error_t **error );

LIBFSXFS_EXTERN \
int libfsxfs_volume_get_file_entry_by_utf16_path(
     libfsxfs_volume_t *volume,
     const uint16_t *utf16_string,
     size_t utf16_string_length,
     libfsxfs_file_entry_t **file_entry,
     libcerror_error_t **error );

/* XFS Magic Signatures */
#define LIBFSXFS_AGF_MAGIC_NUMBER           0x58414746
#define LIBFSXFS_AGFL_MAGIC_NUMBER          0x5841464c
#define LIBFSXFS_BB_V4_MAGIC_NUMBER         0x41425442
#define LIBFSXFS_BB_V5_MAGIC_NUMBER         0x41423342
// ...

/* AGF Offsets */
#define LIBFSXFS_AGF_OFFSET_MAGIC           0x00
#define LIBFSXFS_AGF_OFFSET_VERSION         0x04
#define LIBFSXFS_AGF_OFFSET_SEQUENCE_NO     0x08
#define LIBFSXFS_AGF_OFFSET_LENGTH          0x0c
#define LIBFSXFS_AGF_OFFSET_BNOBT_ROOT      0x10
#define LIBFSXFS_AGF_OFFSET_CNTBT_ROOT      0x14
#define LIBFSXFS_AGF_OFFSET_BNOBT_LEVEL     0x1c
#define LIBFSXFS_AGF_OFFSET_CNTBT_LEVEL     0x20
#define LIBFSXFS_AGF_OFFSET_FLFIRST         0x28
#define LIBFSXFS_AGF_OFFSET_FLLAST          0x2c
#define LIBFSXFS_AGF_OFFSET_FLCOUNT         0x30
#define LIBFSXFS_AGF_OFFSET_FREE_BLOCKS     0x34
#define LIBFSXFS_AGF_OFFSET_LONGEST         0x38
#define LIBFSXFS_AGF_OFFSET_BTREE_BLOCKS    0x3c

/* B+ Tree Block Offsets */
#define LIBFSXFS_BB_OFFSET_MAGIC			0x0
#define LIBFSXFS_BB_OFFSET_LEVEL			0x4
#define LIBFSXFS_BB_OFFSET_NUMRECS			0x6
#define LIBFSXFS_BB_OFFSET_LFSBLG			0x0a
#define LIBFSXFS_BB_OFFSET_RTSBLG			0xe
// ...

/* AGFL Offsets */
#define LIBFSXFS_AGFL_OFFSET_MAGIC			0x0
#define LIBFSXFS_AGFL_OFFSET_SEQNO			0x4
// ...

typedef struct libfsxfs_internal_agf libfsxfs_internal_agf_t;

struct libfsxfs_internal_agf {
    uint32_t magicnum;
    uint32_t version;
    uint32_t seqno;
    uint32_t length;
    uint32_t bnobt_root;
    uint32_t cntbt_root;
    uint32_t revbt_root;
    uint32_t bnobt_level;
    uint32_t cntbt_level;
    uint32_t revbt_level;
    uint32_t fl_first;
    uint32_t fl_last;
    uint32_t fl_count;
    uint32_t freeblks;
    uint32_t longest;
    uint32_t btreeblks;
};

typedef struct libfsxfs_internal_agfl libfsxfs_internal_agfl_t;

struct libfsxfs_internal_agfl {
    uint32_t        agfl_magicnum;
    uint32_t        agfl_seqno;
    uint8_t         agfl_uuid[16];
    uint64_t        agfl_lsn;
    uint32_t        agfl_crc;
};

typedef struct xfs_btree_sblock xfs_btree_sblock_t;

struct xfs_btree_sblock {
    uint32_t magic;
    uint16_t level;
    uint16_t numrecs;
    uint32_t leftsib;
    uint32_t rightsib;
    /* version 5 filesystem fields start here */
    uint64_t blkno;
    uint64_t lsn;
    uint8_t uuid[16];
    uint32_t owner;
    uint32_t crc;
};

int libfsxfs_volume_free_blocks_iterate(
        libfsxfs_volume_t *volume,
        void *data,
        void (*clear_callback)(void *data, uint32_t start, uint32_t blocks),
        libcerror_error_t **error );
#if defined( __cplusplus )
}
#endif

#endif /* !defined( _LIBFSXFS_INTERNAL_VOLUME_H ) */

