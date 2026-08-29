#pragma once

#include "Defines.h"

/** Holds a handle to a file. */
typedef struct FFileHandle
{
	/** Opaque handle to internal file handle. */
	void* Handle;
	bool8 bIsValid;
} FFileHandle;

typedef enum EFileModes
{
	FILE_MODE_READ  = 0x01,
	FILE_MODE_WRITE = 0x02,
} EFileModes;

/**
 * Checks if a file with the fiven path exists.
 * 
 * @param Path The path of the file to be checked.
 * @returns True if exists; otherwise false.
 */
LUMORA_C_API bool8 FileSystemExist(const char* Path);

/**
 * Attempt to open file located at path.
 * 
 * @param Path The path of the file ot be opened.
 * @param Mode Mode flags for the file when opened (read/write). See EFileModes enum in FileSystem.h.
 * @param bBinary Indicates if the file should be opened in binary mode.
 * @param OutHandle A pointer to a FFileHandle structure which holds the handle information.
 * @returns True if opened successfully; otherwise false.
 */
LUMORA_C_API bool8 FileSystemOpen(const char* Path, EFileModes Mode, bool8 bBinary, FFileHandle* OutHandle);

/**
 * Closes the provided handle to a file.
 * 
 * @param Handle A pointer to a FFileHandle structure which holds the handle to be closed.
 */
LUMORA_C_API void FileSystemClose(FFileHandle* Handle);

/**
 * Reads up to a newline or EOF. Allocates *LineBuf, which must be freed by the caller.
 * 
 * @param Handle A pointer to a FFileHandle structure.
 * @param LineBuf A pointer to a character array which will be allocated and populated by this method.
 * @returns True if successful; otherwise false.
 */
LUMORA_C_API bool8 FileSystemReadLine(FFileHandle* Handle, char** LineBuf);

/**
 * Writes text to the provided file, appending a '\n' afterward.
 * 
 * @param Handle A pointer to a FFileHandle structure.
 * @param Text The text to be written.
 * @returns True if successful; otherwise false.
 */
LUMORA_C_API bool8 FileSystemWriteLine(FFileHandle* Handle, const char* Text);

/**
 * Reads up to DateSize bytes of data into OutBytesRead.
 * Allocates *OutData, which must be freed by the caller.
 * 
 * @param Handle A pointer to a FFileHandle structure.
 * @param DataSize The number of bytes to read.
 * @param OutData A pointer to a block of memory to be populated by this method.
 * @param OutBytesRead A pointer to a number which will be populated with the number of bytes actually read from the file.
 * @returns True if successful; otherwise false.
 */
LUMORA_C_API bool8 FileSystemRead(FFileHandle* Handle, size_t DataSize, void* OutData, size_t* OutBytesRead);


/**
 * Reads up to DataSize bytes of data into OutBytesRead.
 * Allocates OutBytes, which must be freed by the caller.
 * 
 * @param Handle A pointer to a FFileHandle structure.
 * @param OutBytes A pointer to a byte array which will be allocated and populated by this method.
 * @param OutBytesRead  A pointer to a number which will be populated with the number of bytes actually used read from the file.
 * @returns True if successful; otherwise false.
 */
LUMORA_C_API bool8 FileSystemReadAllBytes(FFileHandle* Handle, uint8** OutBytes, size_t* OutBytesRead);

/**
 * Writes provided data to the file.
 * 
 * @param Handle A pointer to a FFileHandle structure.
 * @param DataSize The size of the data in bytes.
 * @param Data The data to be written.
 * @param OutBytesWritten A pointerto a numbe which will be populated wwith the number of bytes actually written to the file.
 * @returns True if successful; otherwise false.
 */
LUMORA_C_API bool8 FileSystemWrite(FFileHandle* Handle, size_t DataSize, const void* Data, size_t* OutBytesWritten);