#include "FileHandler.h"

ostream& operator<<(ostream& os, const file_data& data)
{
	string last_move_data = "Unknown";

	if (data.last_move == READ_OP)
		last_move_data = "Read";
	else if (data.last_move == WRITE_OP)
		last_move_data = "Write";

	os << "The file at: " << data.file_path << " ~(" << data.file_access << ")~  --> At a length of: " << data.file_len <<
		"\n\t-- File's name: " << data.file_name << " - File's extension: " << data.extension <<
		"\n\t-- Buffer's Type: " << data.buffer_type << " (" << data.buffer_type_number << ")" <<
		"\n\t-- Buffer's Size: " << data.buffer_size <<
		"\n\t-- File's last move was: " << last_move_data << " Operation!" << std::endl;

	return os;
}

/*
	The function checks the type of the file and return the right string for the opening mode.
*/
string FileHandler::getFileStreamType(const openFileModes& file_mode) const noexcept
{
	switch(file_mode)
	{
		case openFileModes::read: { return "r"; }
		case openFileModes::read_b: { return "rb"; }
		case openFileModes::read_p: { return "r+"; }
		case openFileModes::read_bp: { return "rb+"; }
		case openFileModes::write: { return "w"; }
		case openFileModes::write_b: { return "wb"; }
		case openFileModes::write_p: { return "w+"; }
		case openFileModes::write_bp: { return "wb+"; }
		case openFileModes::append: { return "a"; }
		case openFileModes::append_b: { return "ab"; }
		case openFileModes::append_p: { return "a+"; }
		case openFileModes::append_bp: { return "ab+"; }
		default:
			return DEFUALT_MODE;
	}

	return DEFUALT_MODE;
}

/*
	The function fixes the string path by replacing back slash to forward slash.
	@ It is a static function.
*/
void FileHandler::fixPath(string& path) noexcept
{
	std::replace(path.begin(), path.end(), '\\', '/');
}

/*
	The function gets file's name out of the path.
	@ It is a static function.
*/
string FileHandler::getFileName(const string& path) noexcept
{
	auto pos1 = path.find_last_of("/");
	auto pos2 = path.find_last_of(".");

	if (pos1 != string::npos && pos2 != string::npos)
	{
		return path.substr(pos1 + 1, pos2 - pos1);
	}

	return "";
}

/*
	The function constructs a FileHandler object by trying to open a file.
*/
FileHandler::FileHandler(const string& path, const openFileModes& file_mode, const bufferType& buff_type, size_t buff_size)
	: file(NULL), file_buffer(NULL), file_buffer_size(0), buffer_type(DEFUALT_BUFFER), file_access(DEFUALT_MODE_ENUM), last_move(0), last_file_place(SEEK_SET), clearCharsCanUse(true)
{
	if (!(this->openFile(path, file_mode, buffer_type, buff_size))) { throw FileHandlerException("Error - FileHandler: File couldn't be opened!"); }
}

/*
	The function gets file's extension out of the path.
	@ It is a static function.
*/
string FileHandler::getFileExtenstion(const string& path) noexcept
{
	auto pos = path.find_last_of(".");

	if (pos != string::npos)
	{
		return path.substr(pos + 1, path.size() - pos);
	}

	return "";
}

/*
	The function gets the file's length.
	--> Might be slow so try to use the function only once and then store the value!
*/
long FileHandler::getFilesLength() noexcept
{
	if (this->file == NULL)
		return -1;

	long curr_pos = ftell(this->file);

	fseek(this->file, 0, SEEK_END);
	long file_size = ftell(this->file);
	fseek(this->file, curr_pos, SEEK_SET);

	return file_size;
}

/*
	The function enables the option to write data into the file.
	--> The function is throwable!
*/
FileHandler& FileHandler::operator<<(const char* str)
{
	if (this->file != NULL)
	{
		this->last_move = WRITE_OP;

		if (str == nullptr) { return *this; }

		unsigned int length = strlen(str);

		if (length <= 0) { return *this; }

		if (this->file_access == openFileModes::write || this->file_access == openFileModes::write_p ||
			this->file_access == openFileModes::append || this->file_access == openFileModes::append_p ||
			this->file_access == openFileModes::read_p || this->file_access == openFileModes::write_b ||
			this->file_access == openFileModes::write_bp || this->file_access == openFileModes::append_b ||
			this->file_access == openFileModes::append_bp || this->file_access == openFileModes::read_bp)
		{
			if (!this->clearCharsCanUse)
			{
				char* ndata = new char[length + 1]();
				unsigned int nsize = 0;

				for (unsigned int i = 0; i < length + 1; i++)
				{
					const char ch = str[i];
					if (this->charsCanUse[(int)ch])
					{
						ndata[nsize] = ch;
						nsize++;
					}
				}

				bool val = fwrite(ndata, sizeof(char), nsize, this->file) == nsize;

				delete ndata;
				ndata = nullptr;

				if (val) return *this;
			}
			else
			{
				if (fwrite(str, sizeof(char), length, this->file) == length) return *this;
			}

			throw FileHandlerException("Error - FileHandler: Failed to write into the file!");
		}

		throw FileHandlerException("Error - FileHandler: The file access type doesn't allow to write into the file!");
	}

	throw FileHandlerException("Error - FileHandler: The file isn't opened!");
}

/*
	The function enables the option to write data into the file.
	--> The function is throwable!
*/
FileHandler& FileHandler::operator<<(const string& str)
{
	if (this->file != NULL)
	{
		this->last_move = WRITE_OP;

		if (str.empty()) { return *this; }

		if (this->file_access == openFileModes::write || this->file_access == openFileModes::write_p ||
			this->file_access == openFileModes::append || this->file_access == openFileModes::append_p ||
			this->file_access == openFileModes::read_p || this->file_access == openFileModes::write_b ||
			this->file_access == openFileModes::write_bp || this->file_access == openFileModes::append_b ||
			this->file_access == openFileModes::append_bp || this->file_access == openFileModes::read_bp)
		{
			if (!this->clearCharsCanUse)
			{
				char* ndata = new char[str.size() + 1]();
				unsigned int nsize = 0;

				for (unsigned int i = 0; i < str.size() + 1; i++)
				{
					const char ch = str[i];
					if (this->charsCanUse[(int)ch])
					{
						ndata[nsize] = ch;
						nsize++;
					}
				}

				bool val = fwrite(ndata, sizeof(char), nsize, this->file) == nsize;

				delete ndata;
				ndata = nullptr;

				if (val) return *this;
			}
			else
			{
				if (fwrite(str.c_str(), sizeof(char), str.size(), this->file) == str.size()) return *this;
			}

			throw FileHandlerException("Error - FileHandler: Failed to write into the file!");
		}

		throw FileHandlerException("Error - FileHandler: The file access type doesn't allow to write into the file!");
	}

	throw FileHandlerException("Error - FileHandler: The file isn't opened!");
}

/*
	The function enables the option to write data into the file.
	--> The function is throwable!
*/
FileHandler& FileHandler::operator<<(string&& str)
{
	if (this->file != NULL)
	{
		this->last_move = WRITE_OP;

		if (str.empty()) { return *this; }

		if (this->file_access == openFileModes::write || this->file_access == openFileModes::write_p ||
			this->file_access == openFileModes::append || this->file_access == openFileModes::append_p ||
			this->file_access == openFileModes::read_p || this->file_access == openFileModes::write_b ||
			this->file_access == openFileModes::write_bp || this->file_access == openFileModes::append_b ||
			this->file_access == openFileModes::append_bp || this->file_access == openFileModes::read_bp)
		{

			if (!this->clearCharsCanUse)
			{
				char* ndata = new char[str.size() + 1]();
				unsigned int nsize = 0;

				for (unsigned int i = 0; i < str.size() + 1; i++)
				{
					const char ch = str[i];
					if (this->charsCanUse[(int)ch])
					{
						ndata[nsize] = ch;
						nsize++;
					}
				}

				bool val = fwrite(ndata, sizeof(char), nsize, this->file) == nsize;

				delete ndata;
				ndata = nullptr;

				if (val) return *this;
			}
			else
			{
				if (fwrite(str.c_str(), sizeof(char), str.size(), this->file) == str.size()) return *this;
			}

			throw FileHandlerException("Error - FileHandler: Failed to write into the file!");
		}

		throw FileHandlerException("Error - FileHandler: The file access type doesn't allow to write into the file!");
	}

	throw FileHandlerException("Error - FileHandler: The file isn't opened!");
}

/*
	The function enables the option to read data from the file.
	--> The function is throwable!
*/
FileHandler& FileHandler::operator>>(char* str)
{
	if (this->file != NULL)
	{
		this->last_move = READ_OP;

		if (feof(this->file)) { throw FileHandlerException("Error - FileHandler: Failed to read from file -> End of file was reached!"); }

		if (this->file_access == openFileModes::read || this->file_access == openFileModes::read_p ||
			this->file_access == openFileModes::write_p || this->file_access == openFileModes::append_p ||
			this->file_access == openFileModes::read_b || this->file_access == openFileModes::read_bp ||
			this->file_access == openFileModes::write_bp || this->file_access == openFileModes::append_bp)
		{

			char tmp = 0;
			char* data = new char[DFLT_BUFF_GLINE_SIZE + 1]();
			unsigned int ccount = 0, buffcount = 1;

			while (!feof(this->file))
			{
				tmp = fgetc(this->file);
				ccount++;
				if (tmp == EOF || tmp == '\0' || tmp == '\n') break;
				if (tmp == '\r') continue;
				data[ccount - 1] = tmp;
				if (ccount > DFLT_BUFF_GLINE_SIZE * buffcount)
				{
					char* ndata = new char[DFLT_BUFF_GLINE_SIZE * (++buffcount) + 1]();
					memcpy(ndata, data, DFLT_BUFF_GLINE_SIZE * (buffcount - 1) + 1);
					delete data;
					data = ndata;
				}
			}
			
			if (!this->clearCharsCanUse)
			{
				char* ndata = new char[ccount]();

				for (unsigned int i = 0, j = 0; i < ccount; i++)
				{
					const char ch = data[i];
					if (this->charsCanUse[(int)ch])
					{
						ndata[j] = ch;
						j++;
					}
				}

				delete data;
				data = ndata;
			}

			str = data;

			return *this;
		}

		throw FileHandlerException("Error - FileHandler: The file access type doesn't allow to read from the file!");
	}

	throw FileHandlerException("Error - FileHandler: The file isn't opened!");
}

/*
	The function enables the option to read data from the file.
	--> The function is throwable!
*/
FileHandler& FileHandler::operator>>(string& str)
{
	if (this->file != NULL)
	{
		this->last_move = READ_OP;

		if (feof(this->file)) { throw FileHandlerException("Error - FileHandler: Failed to read from file -> End of file was reached!"); }

		if (this->file_access == openFileModes::read || this->file_access == openFileModes::read_p ||
			this->file_access == openFileModes::write_p || this->file_access == openFileModes::append_p ||
			this->file_access == openFileModes::read_b || this->file_access == openFileModes::read_bp ||
			this->file_access == openFileModes::write_bp || this->file_access == openFileModes::append_bp)
		{

			char tmp = 0;
			char* data = new char[DFLT_BUFF_GLINE_SIZE + 1]();
			unsigned int ccount = 0, buffcount = 1;

			while (!feof(this->file))
			{
				tmp = fgetc(this->file);
				ccount++;
				if (tmp == EOF || tmp == '\0' || tmp == '\n') break;
				if (tmp == '\r') continue;
				data[ccount - 1] = tmp;
				if (ccount > DFLT_BUFF_GLINE_SIZE * buffcount)
				{
					char* ndata = new char[DFLT_BUFF_GLINE_SIZE * (++buffcount) + 1]();
					memcpy(ndata, data, DFLT_BUFF_GLINE_SIZE * (buffcount - 1) + 1);
					delete data;
					data = ndata;
				}
			}

			if (!this->clearCharsCanUse)
			{
				char* ndata = new char[ccount]();

				for (unsigned int i = 0, j = 0; i < ccount; i++)
				{
					const char ch = data[i];
					if (this->charsCanUse[(int)ch])
					{
						ndata[j] = ch;
						j++;
					}
				}

				delete data;
				data = ndata;
			}

			str = data;
			delete data;
			data = nullptr;

			return *this;
		}

		throw FileHandlerException("Error - FileHandler: The file access type doesn't allow to read from the file!");
	}

	throw FileHandlerException("Error - FileHandler: The file isn't opened!");
}

/*
	The function opens the wanted file by giving the right data.
	@ If a file is already opened then it will try to close the stored file, but if it fails
		it will return false and won't keep on going.
*/
bool FileHandler::openFile(const string& f_path, const openFileModes& file_mode, const bufferType& buff_type, size_t buff_size) noexcept
{
	string open_mode = getFileStreamType(file_mode);
	this->file_access = file_mode;
	
	string fnew_path = f_path;
	fixPath(fnew_path);

	if (this->file != NULL) // Closing old file and if can't close it then return false
	{
		if(!(this->closeFile())) { return false; }
	}

	if ((this->file = fopen(fnew_path.c_str(), open_mode.c_str())) != NULL)
	{
		if (buff_size > MAX_BUFFER_SIZE)
		{
			buff_size = MAX_BUFFER_SIZE;
		}
		else if (buff_size < MIN_BUFFER_SIZE)
		{
			buff_size = MIN_BUFFER_SIZE;
		}

		if (this->file_buffer == NULL && buff_type != bufferType::non_buffer) // Buffer allocation check
		{
			this->file_buffer = new char[buff_size];
			this->file_buffer_size = buff_size * sizeof(char);

			if (this->file_buffer == NULL) // Secondary checking that the allocation/reallocation worked
			{
				fclose(this->file);
				this->file = NULL;
				this->file_buffer_size = 0;
				return false;
			}
		}
		else if (buff_type != bufferType::non_buffer)
		{
			delete[] this->file_buffer;
			this->file_buffer = new char[buff_size];
			this->file_buffer_size = buff_size * sizeof(char);

			if (this->file_buffer == NULL) // Secondary checking that the allocation/reallocation worked
			{
				fclose(this->file);
				this->file = NULL;
				this->file_buffer_size = 0;
				return false;
			}
		}
		else if (this->file_buffer != NULL)
		{
			delete[] this->file_buffer;
			this->file_buffer = NULL;
			this->file_buffer_size = 0;
		}

		switch (buff_type)
		{
		case bufferType::non_buffer: { setvbuf(this->file, this->file_buffer, _IONBF, buff_size);  break; }
		case bufferType::line_buffer: { setvbuf(this->file, this->file_buffer, _IOLBF, buff_size);  break; }
		case bufferType::full_buffer: { setvbuf(this->file, this->file_buffer, _IOFBF, buff_size);  break; }
		default: { setvbuf(this->file, this->file_buffer, DEFUALT_BUFFER_NAME, buff_size); }
		}

		this->file_path = fnew_path;
		this->file_name = getFileName(this->file_path);
		this->extension = getFileExtenstion(this->file_path);

		return true;
	}

	return false;
}

/*
	The function is writing data into a file by the given parameters.
*/
bool FileHandler::writeToFile(const string& data, const int& pos, const bool& auto_rewind, const bool& flush_file) noexcept
{
	if (this->file != NULL)
	{
		this->last_move = WRITE_OP;

		if (flush_file) { this->flushFile(); }
		
		if (data.size() <= 0) { return true; }
		
		if (pos >= 0)
		{
			this->moveCursorInFile(filePosSet::start_file, pos);
		}

		auto rewind_check = [&]()
		{
			if (pos >= 0 && auto_rewind)
			{
				this->rewindFileOneStep();
			}
		};
	
		if (this->file_access == openFileModes::write || this->file_access == openFileModes::write_p ||
			this->file_access == openFileModes::append || this->file_access == openFileModes::append_p ||
			this->file_access == openFileModes::read_p || this->file_access == openFileModes::write_b ||
			this->file_access == openFileModes::write_bp || this->file_access == openFileModes::append_b ||
			this->file_access == openFileModes::append_bp || this->file_access == openFileModes::read_bp)
		{

			bool val = true;
			if (!this->clearCharsCanUse) 
			{
				unsigned int nsize = data.size();
				char* ndata = new char[data.size()]();
				
				for (unsigned int i = 0, j = 0; i < data.size(); i++)
				{
					const char ch = data[i];
					if (this->charsCanUse[(int)ch])
					{
						ndata[j] = ch;
						j++;
					}
					else
					{
						nsize--;
					}
				}

				val = fwrite(ndata, sizeof(char), nsize, this->file) == nsize;
				delete ndata;
				ndata = nullptr;
			}
			else
			{
				val = fwrite(data.c_str(), sizeof(char), data.size(), this->file) == data.size();
			}

			rewind_check();

			return val;
		}

		rewind_check();
	}

	return false;
}

/*
	The function is reading data from the file by the given parameters.
*/
retObj<string> FileHandler::readFromFile(const size_t& count, const int& pos, const bool& auto_rewind, const bool& flush_file) noexcept
{
	if (this->file != NULL)
	{
		this->last_move = READ_OP;

		if (flush_file && this->buffer_type != bufferType::non_buffer) { fflush(this->file); }

		if (count <= 0) { return { "", -1, true }; }

		if (pos >= 0)
		{
			this->moveCursorInFile(filePosSet::start_file, pos);
		}

		auto rewind_check = [&]()
		{
			if (pos >= 0 && auto_rewind)
			{
				this->rewindFileOneStep();
			}
		};

		if (this->file_access == openFileModes::read || this->file_access == openFileModes::read_p ||
			this->file_access == openFileModes::write_p || this->file_access == openFileModes::append_p || 
			this->file_access == openFileModes::read_b || this->file_access == openFileModes::read_bp ||
			this->file_access == openFileModes::write_bp || this->file_access == openFileModes::append_bp)
		{
			char* temp_str = new char[(unsigned int)count + 1]();
			bool read_work = fread(temp_str, sizeof(char), count, this->file) == (count);

			if (!this->clearCharsCanUse)
			{
				char* ndata = new char[(unsigned int)count + 1]();

				for (unsigned int i = 0, j = 0; i < (unsigned int)count + 1; i++)
				{
					const char ch = temp_str[i];
					if (this->charsCanUse[(int)ch])
					{
						ndata[j] = ch;
						j++;
					}
				}

				delete temp_str;
				temp_str = ndata;
			}

			string str = temp_str;
			delete temp_str;
			temp_str = nullptr;

			if (!read_work && feof(this->file))
			{
				rewind_check();
				return { std::to_string(EOF), -1, true };
			}
			else if (!read_work)
			{
				rewind_check();
				return { "", 0, false };
			}
			else if (feof(this->file))
			{
				rewind_check();
				return { str.append(std::to_string(EOF)), -1, true };
			}

			rewind_check();
			return { str, -1, true };
		}

		rewind_check();
	}

	return { "", 0, false };
}

/*
	The function gets a line out of a file.
	@ This file returns error if during the getting line, the file met \0 of EOF operators and the wanted line wasn't reached yet.
*/
retObj<string> FileHandler::getLine(unsigned int numline, const int& pos, unsigned int buff_size, const bool& auto_rewind, const bool& flush_file) noexcept
{
	if (this->file != NULL)
	{
		this->last_move = READ_OP;

		if (flush_file && this->buffer_type != bufferType::non_buffer) { fflush(this->file); }

		if (feof(this->file)) { return { "", -1, true }; }

		if (buff_size < MIN_BUFFER_GETLINE_SIZE || buff_size > MAX_BUFFER_GETLINE_SIZE) { buff_size = DFLT_BUFF_GLINE_SIZE; }
		
		if (pos >= 0)
		{
			this->moveCursorInFile(filePosSet::start_file, pos);
		}

		auto rewind_check = [&]()
		{
			if (pos >= 0 && auto_rewind)
			{
				this->rewindFileOneStep();
			}
		};

		if (this->file_access == openFileModes::read || this->file_access == openFileModes::read_p ||
			this->file_access == openFileModes::write_p || this->file_access == openFileModes::append_p ||
			this->file_access == openFileModes::read_b || this->file_access == openFileModes::read_bp ||
			this->file_access == openFileModes::write_bp || this->file_access == openFileModes::append_bp)
		{
						
			char tmp = 0;
			char* data = new char[buff_size + 1]();
			unsigned int ccount = 0, buffcount = 1;

			while (!feof(this->file))
			{
				tmp = fgetc(this->file);
				ccount++;
				if (tmp == EOF || tmp == '\0' || (numline <= 0 && tmp == '\n')) break;
				numline--;
				if (tmp == '\r') continue;
				if (numline <= 0) data[ccount - 1] = tmp;
				if (ccount > buff_size * buffcount)
				{
					char* ndata = new char[buff_size * (++buffcount) + 1]();
					memcpy(ndata, data, buff_size * (buffcount - 1) + 1);
					delete data;
					data = ndata;
				}
			}

			rewind_check();

			if ((tmp == EOF || tmp == '\0') && numline > 0)  return { "", 0, false };

			if (!this->clearCharsCanUse)
			{
				char* ndata = new char[ccount]();

				for (unsigned int i = 0, j = 0; i < ccount; i++)
				{
					const char ch = data[i];
					if (this->charsCanUse[(int)ch])
					{
						ndata[j] = ch;
						j++;
					}
				}

				delete data;
				data = ndata;
			}

			string str = data;
			delete data;
			data = nullptr;

			return { str, -1, true };
		}

		rewind_check();
	}

	return { "", 0, false };
}

/*
	The function is closing a file and the buffer if opened.
*/
bool FileHandler::closeFile() noexcept { if (this->file_buffer != NULL) { delete[] this->file_buffer; this->file_buffer = NULL; this->file_buffer_size = 0; } file_path = "";  file_name = ""; extension = ""; if (this->file != NULL) { fclose(this->file); this->file = NULL; return true; } return false; }

/*
	The function deletes the function from the computer.
*/
bool FileHandler::removeFile() noexcept { closeFile(); return !remove(this->file_path.c_str()); }

/*
	The function flushed the buffer if existst and if in writing mode, else just return true.
*/
bool FileHandler::flushFile() noexcept
{
	if (this->buffer_type == bufferType::non_buffer) { return true; }

	if(this->file != NULL && (this->file_access == openFileModes::write || this->file_access == openFileModes::write_b ||
		this->file_access == openFileModes::append || this->file_access == openFileModes::append_b || this->last_move == WRITE_OP))
	{
		return !fflush(this->file);
	}

	return false;
}

/*
	The function changes the file buffer type and size.
*/
bool FileHandler::changeFileBuffer(const bufferType& buff_type, size_t buff_size) noexcept
{
	if(file != NULL)
	{
		if (buff_size > MAX_BUFFER_SIZE)
		{
			buff_size = MAX_BUFFER_SIZE;
		}
		else if (buff_size < MIN_BUFFER_SIZE)
		{
			buff_size = MIN_BUFFER_SIZE;
		}

		if (this->file_buffer == NULL && buff_type != bufferType::non_buffer) // Buffer allocation check
		{
			this->file_buffer = new char[buff_size];
			this->file_buffer_size = buff_size * sizeof(char);

			if (this->file_buffer == NULL) // Secondary checking that the allocation/reallocation worked
			{
				fclose(this->file);
				this->file = NULL;
				this->file_buffer_size = 0;
				return false;
			}
		}
		else if (buff_type != bufferType::non_buffer)
		{
			delete[] this->file_buffer;
			this->file_buffer = new char[buff_size];
			this->file_buffer_size = buff_size * sizeof(char);

			if (this->file_buffer == NULL) // Secondary checking that the allocation/reallocation worked
			{
				fclose(this->file);
				this->file = NULL;
				this->file_buffer_size = 0;
				return false;
			}
		}
		else if (this->file_buffer != NULL)
		{
			delete[] this->file_buffer;
			this->file_buffer = NULL;
			this->file_buffer_size = 0;
		}

		switch(buff_type)
		{
		case bufferType::non_buffer: { setvbuf(this->file, this->file_buffer, _IONBF, buff_size);  break; }
		case bufferType::line_buffer: { setvbuf(this->file, this->file_buffer, _IOLBF, buff_size);  break; }
		case bufferType::full_buffer: { setvbuf(this->file, this->file_buffer, _IOFBF, buff_size); break; }
		default: { setvbuf(this->file, this->file_buffer, DEFUALT_BUFFER_NAME, buff_size); }
		}

		return true;
	}

	return false;
}

/*
	The function gets the file's state.
*/
file_data FileHandler::getFileState() noexcept
{
	unsigned int file_len = 0;
	unsigned int buffer_type_number = 0;
	string buffer_type;

	switch(this->buffer_type)
	{
		case bufferType::non_buffer: { buffer_type = "No Buffering"; buffer_type_number = _IONBF; break;}
		case bufferType::line_buffer: { buffer_type = "Line Buffering"; buffer_type_number = _IOLBF;  break;}
		case bufferType::full_buffer: { buffer_type = "Full Buffering"; buffer_type_number = _IOFBF; break;}
		default:
		{
			buffer_type = DEFUALT_BUFFER_NAME_TXT;
			buffer_type_number = DEFUALT_BUFFER_NAME;
		}
	}

	return { this->file_path, this->file_name, this->extension, this->file_buffer,  buffer_type_number, buffer_type,
						getFileStreamType(this->file_access), getFilesLength(), this->file_buffer_size, this->last_move };
}

/*
	The function move's the cursor around the file.
	--> This function is slow and should'nt be used a lot!
*/
bool FileHandler::moveCursorInFile(const filePosSet& pos_set, int offset) noexcept
{
	if(file == NULL)
		return false;

	int curser_pos = 0;

	this->last_file_place = ftell(this->file);

	switch(pos_set)
	{
		case filePosSet::start_file: { curser_pos = SEEK_SET; break;}
		case filePosSet::current_file: { curser_pos = SEEK_CUR; break;}
		case filePosSet::end_file: { curser_pos = SEEK_END; break;}
		default:
			curser_pos = SEEK_SET;
	}

	return !fseek(this->file, (long)offset, curser_pos);
}

/*
	Sets the file's ignoring data.
	--> The ignoring is set both to reading and writing options!
*/
bool FileHandler::setIgnoring(const ignore_data& ignoring) noexcept
{
	if (this->file == NULL) { return false; }

	if (!ignoring.ignore_signle_chars.empty()) { this->clearCharsCanUse = false; }

	for (int i = 0; i < ignoring.ignore_signle_chars.size(); i++)
	{
		this->charsCanUse[ignoring.ignore_signle_chars[i]] = false;
	}

	for (int i = 0; i < ignoring.ignore_range_chars.size(); i++)
	{
		const pair<char, char>& range_data = ignoring.ignore_range_chars[i];

		if (range_data.first <= range_data.second)
		{
			this->clearCharsCanUse = false;

			for (int j = range_data.first; j <= range_data.second; j++)
			{
				this->charsCanUse[j] = false;
			}
		}
	}

	return true;
}

/*
	Clears the file's ignoring data.
	--> The ignoring is cleared both to reading and writing options!
*/
bool FileHandler::clearIngoring() noexcept
{
	if (this->file == NULL) { return false; }

	for (int i = 0; i < MAX_CHAR_CAPACITY; i++)
	{
		this->charsCanUse[i] = true;
	}

	this->clearCharsCanUse = true;

	return true;
}

//pair<bool, char> FileHandler::operator[](const int index)
//{
//	return pair<bool, char>();
//}
//
//pair<bool, string> FileHandler::substr(const int index, const int len)
//{
//	return pair<bool, string>();
//}

/*
	The function rewinds the file one step by the cursor.
	--> This function is slow and should'nt be used a lot!
*/
bool FileHandler::rewindFileOneStep() noexcept
{
	if (this->file != NULL)
	{
		return !fseek(this->file, this->last_file_place, SEEK_SET);
	}

	return false;
}

/*
	The function checks if the cursor at the end of the file.
*/
bool FileHandler::isEndOfFile() const noexcept { if (file != NULL) { return feof(this->file); } return false;}

/*
	The function checks if the file is opened.
*/
bool FileHandler::isFileOpened() const noexcept { return file != NULL; }

/*
	The function checks if the file exists.
*/
bool FileHandler::fileExists(const std::string& f_path) noexcept
{
	FILE* fl = NULL;
	if((fl = fopen(f_path.c_str(), "rb")) != NULL)
	{ fclose(fl); return true; }

	return false;
}