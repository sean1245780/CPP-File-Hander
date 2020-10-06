#pragma once

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <exception>

using std::string;
using std::ostream;
using std::pair;
using std::vector;

#define DEFUALT_MODE				"rb"
#define DEFUALT_MODE_ENUM			openFileModes::read_b
#define DEFUALT_BUFFER				bufferType::full_buffer
#define DEFUALT_BUFFER_NAME			_IOFBF
#define DEFUALT_BUFFER_NAME_TXT		"Full Buffering"
#define DEFUALT_BUFFER_SIZE			2048
#define MAX_BUFFER_SIZE				16384
#define MIN_BUFFER_SIZE				128
#define MAX_BUFFER_GETLINE_SIZE		1024
#define MIN_BUFFER_GETLINE_SIZE		1

#define WRITE_OP					'w'
#define READ_OP						'r'
#define NON_WORK					-1
#define DFLT_BUFF_GLINE_SIZE		16
#define MAX_CHAR_CAPACITY			256

#define OS_KW_CONST
#if defined(__unix__) || defined(__unix) || defined(__linux__)
#define OS_LINUX
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#define OS_WIN
#elif defined(__APPLE__) || defined(__MACH__)
#define OS_MAC
#else
#warning The used OS is not recognized by the FileHandler.h file!
#define OS_KW_CONST const
#endif


namespace FileObj 
{
#if defined(OS_LINUX) || defined(OS_MAC)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

	static inline time_t _getFileLastModificationTime(const char* fpath)
	{
		struct stat obj {};
		return !stat(fpath, &obj) ? obj.st_mtime : 0;
	}

#elif defined(OS_WIN)
#include <windows.h>
	static inline FILETIME _getFileLastModificationTime(const char* fpath)
	{
		HANDLE file = CreateFileA(fpath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (INVALID_HANDLE_VALUE == INVALID_HANDLE_VALUE) { return { 0, 0 }; }

		FILETIME lpmodf;
		BOOL fileTimeWork = GetFileTime(file, NULL, NULL, &lpmodf);

		CloseHandle(file);

		if (!fileTimeWork) { return { 0, 0 }; }

		return lpmodf;
	}
#endif

	/*
		Answer type for the types that can;t be defined with simple errors.
		O - The object to return.
		E - The error indicator.
	*/

	template <class O>
	struct retObj
	{
		O obj;
		unsigned int statusObj;
	};

	/*
		Explanation:
		read("r") ->	read: Open file for input operations. The file must exist.
		write("w") ->	write: Create an empty file for output operations. If a file with the same name already exists, its contents are discarded and the file is treated as a new empty file.
		append("a") ->	append: Open file for output at the end of a file. Output operations always write data at the end of the file, expanding it. Repositioning operations (fseek, fsetpos, rewind) are ignored. The file is created if it does not exist.
		read_p("r+") ->	read/update: Open a file for update (both for input and output). The file must exist.
		write_p("w+") ->	write/update: Create an empty file and open it for update (both for input and output). If a file with the same name already exists its contents are discarded and the file is treated as a new empty file.
		append_p("a+") ->	append/update: Open a file for update (both for input and output) with all output operations writing data at the end of the file. Repositioning operations (fseek, fsetpos, rewind) affects the next input operations, but output operations move the position back to the end of file. The file is created if it does not exist.

		The 'b' addition just means the file will be treated in a binary form.
	*/
	enum class openFileModes
	{
		read, read_b, read_p, read_bp,
		write, write_b, write_p, write_bp,
		append, append_b, append_p, append_bp
	};


	/*
		This is user for setting the wanted type of buffer.
		full_buffer = _IOFBF --> Full buffering: On output, data is written once the buffer is full (or flushed). On Input, the buffer is filled when an input operation is requested and the buffer is empty.
		line_buffer = _IOLBF --> Line buffering: On output, data is written when a newline character is inserted into the stream or when the buffer is full (or flushed), whatever happens first.
									On Input, the buffer is filled up to the next newline character when an input operation is requested and the buffer is empty.
		non_buffer = _IONBF	 --> No buffering: No buffer is used. Each I/O operation is written as soon as possible. In this case, the buffer and size parameters are ignored.
	*/
	enum class bufferType
	{
		non_buffer, line_buffer, full_buffer
	};

	/*
		This sets the given data for moving the cursor in the file.
		start_file = SEEK_SET --> Beginning of file
		current_file = SEEK_CUR --> Current position of the file pointer
		end_file = SEEK_END --> End of file *
	*/
	enum class filePosSet
	{
		start_file, current_file, end_file
	};

	/*
		This is the enum of the return values.
	*/
	enum returnAns
	{
		ra_faliure = 0b0,			// Just faliure - Technically useless
		ra_succss = 0b1,			// Success
		ra_unknown_fail = 0b10,			// Error - The file is closed and needed to be opened!
		ra_fileisclosed_fail = 0b100,			// Error - The file is closed and needed to be opened!
		ra_fileaccesstype_fail = 0b1000,			// Error - The operation failed because the access type is wrong!
		ra_readfile_fail = 0b10000,		// Error - The file failed to read from the file!
		ra_writefile_fail = 0b100000,		// Error - The file write into the file
		ra_readlinefile_fail = 0b1000000,		// Error - The file failed to read a line from the file!
		ra_endoffile_fail = 0b10000000,		// Error - The file reached EOF!
		ra_updatetimefile_fail = 0b100000000,	// Error - Can't get the update time of the file!
		ra_outofrange_fail = 0b1000000000	// Error - The wanted/given data is out of range!
	};

	typedef struct file_data // Data for thr file
	{
		const string& file_path;
		const string& file_name;
		const string& extension;
		const char* const file_buffer;
		const unsigned int buffer_type_number;
		const string& buffer_type;
		const string& file_access;
		const long file_len;
		const unsigned int buffer_size;
		const unsigned char last_move;

		friend ostream& operator<<(ostream& os, const file_data& data);
	} file_data;

	typedef struct ignore_data // For setting which chars to ignore
	{
		vector<char> ignore_signle_chars; // For ignoring only single chars
		vector<pair<char, char>> ignore_range_chars; // For ignoring a range of chars between the two, including both.

	} ignore_data;

	class FileHandler
	{
	private:
		string file_path;
		string file_name;
		string extension;
		FILE* file;
		char* file_buffer;
		unsigned int file_buffer_size;
		bool charsCanUse[MAX_CHAR_CAPACITY];
		bool clearCharsCanUse;

		bufferType buffer_type;
		openFileModes file_access;
		unsigned char last_move;
		long last_file_place;

		string getFileStreamType(const openFileModes& file_mode) const noexcept;

	public:
		FileHandler() noexcept;
		FileHandler(const string& path, const openFileModes& file_mode = DEFUALT_MODE_ENUM, const bufferType& buff_type = DEFUALT_BUFFER, size_t buff_size = DEFUALT_BUFFER_SIZE);

		~FileHandler() { if (file != NULL) { fclose(file); file = NULL; } if (file_buffer != NULL) { delete[] file_buffer; file_buffer = NULL; this->file_buffer_size = 0; } }

		FileHandler(const FileHandler& other) = delete;
		FileHandler(FileHandler&& other) = delete;
		FileHandler& operator=(const FileHandler& other) = delete;
		FileHandler& operator=(FileHandler&& other) = delete;

		FileHandler& operator<< (const char* str);
		FileHandler& operator<< (const string& str);
		FileHandler& operator<< (string&& str);
		FileHandler& operator>> (char* str);
		FileHandler& operator>> (string& str);
		bool& operator[](const unsigned int index);
		char const* const* const operator()() const noexcept;

		bool openFile(const string& f_name, const openFileModes& file_mode = DEFUALT_MODE_ENUM,
			const bufferType& buff_type = DEFUALT_BUFFER, size_t buff_size = DEFUALT_BUFFER_SIZE) noexcept;
		bool writeToFile(const string& data, const int& pos = NON_WORK, const bool& auto_rewind = false, const bool& flush_file = false) noexcept;
		retObj<string> readFromFile(const size_t& count = 1, const int& pos = NON_WORK, const bool& auto_rewind = true, const bool& flush_file = false) noexcept; // Returns 
		retObj<string> getLine(unsigned int numline = 0, const int& pos = NON_WORK, unsigned int buff_size = DFLT_BUFF_GLINE_SIZE, const bool& auto_rewind = true, const bool& flush_file = false) noexcept;
		bool closeFile() noexcept;
		bool removeFile() noexcept;
		bool flushFile() noexcept; // Safe flush
		bool changeFileBuffer(const bufferType& buff_type = DEFUALT_BUFFER, size_t buff_size = DEFUALT_BUFFER_SIZE) noexcept;
		bool moveCursorInFile(const filePosSet& pos_set, int offset = 0) noexcept;
		bool setIgnoring(const ignore_data& ignoring) noexcept;
		bool clearIngoring() noexcept;

		file_data getFileState() noexcept;
		long getFilesLength() noexcept;
		bool rewindFileOneStep() noexcept;
		bool isEndOfFile() const noexcept;
		bool isFileOpened() const noexcept;

		static bool fileExists(const std::string& f_path) noexcept;
		static void fixPath(string& path) noexcept;
		static string getFileName(const string& path) noexcept;
		static string getFileExtenstion(const string& path) noexcept;
		static retObj<time_t> getFileTime(const string& path) OS_KW_CONST
		{
#if defined(OS_LINUX) || defined(OS_MAC)
			time_t timeOfFile = _getFileLastModificationTime(path.c_str());
			return (timeOfFile) ? {.obj = timeOfFile, .errorObj = -1, .success = true} : {.obj = 0, .errorObj = 0, .success = false };
#elif defined(OS_WIN)
			FILETIME timeOfFile = _getFileLastModificationTime(path.c_str());
			if (timeOfFile.dwHighDateTime == 0 && timeOfFile.dwLowDateTime == 0) return { .obj = 0, ra_unknown_fail };
			ULARGE_INTEGER ulint;
			ulint.LowPart = timeOfFile.dwLowDateTime;
			ulint.HighPart = timeOfFile.dwHighDateTime;
			return { .obj = (time_t)(ulint.QuadPart / 10000000ULL - 11644473600ULL), ra_succss };
#else
			return { .obj = 0, ra_unknown_fail };
#endif
		}
	};

	class FileHandlerException : public std::exception
	{
	protected:
		string obj;
		unsigned int code_val;

	public:
		FileHandlerException(const unsigned int code, const string& str = "") : code_val(code), obj(str) {}
		FileHandlerException(const string& str, const unsigned int code = ra_unknown_fail) : obj(str), code_val(code) {}
		FileHandlerException(const unsigned int code, const char* str = "") : code_val(code), obj(str) {}
		FileHandlerException(const char* str, const unsigned int code = ra_unknown_fail) : obj(str), code_val(code) {}
		virtual ~FileHandlerException() = default;

		virtual char const* what() const noexcept
		{
			return obj.c_str();
		}
	};
}
#undef OS_LINUX
#undef OS_MAC
#undef OS_WIN
#undef OS_KW_CONST