#include "Utils.h"

namespace utils
{
	// Result as reference parameter to avoid extra copy
	void readLines(std::istream& input, std::vector<std::string>& out)
	{
		const int	bufferCapacity = 256;
		char		buffer[bufferCapacity];
		int			lineCount = 0;

		while (input)
		{
			int	len;
			int scanPos = 0;
			int startPos = 0;

			input.read(buffer, bufferCapacity);
			len = input.gcount();

			while (scanPos < len)
			{
				bool	newLine = false;
				startPos = scanPos;

				for (; scanPos < len; scanPos++)
				{
					if (buffer[scanPos] == '\r')
						break;
					else if (buffer[scanPos] == '\n')
					{
						newLine = true;
						break;
					}
				}

				out.resize(lineCount + 1);
				out[lineCount].append(&buffer[startPos], scanPos - startPos);

				// Skip all empty lines
				for (; scanPos < len; scanPos++)
				{
					if (buffer[scanPos] == '\r')
						continue;
					else if (buffer[scanPos] == '\n')
					{
						newLine = true;
						continue;
					}
					else
						break;
				}

				if (newLine)
					lineCount++;
			}
		}

		out.resize(lineCount);
	}

	// Result as reference parameter to avoid extra copy
	void explodeString(const std::string& text, const std::vector<char>& separators, std::vector<std::string>& out)
	{
		int	i = 0;
		int uPos = 0;

		auto	nextSeparator = [&]()
		{
			for (size_t i = uPos; i < text.size(); i++)
				for (char separator : separators)
					if (text[i] == separator)
						return i;
			return std::string::npos;
		};

		while (i < 1)
		{
			size_t pos = nextSeparator();
			if (pos == text.npos)
			{
				if (text.length() - uPos > 0)
					out.push_back(text.substr(uPos, text.length() - uPos));
				i = 1;
			}
			else
			{
				if (pos - uPos > 0)
					out.push_back(text.substr(uPos, pos - uPos));
				uPos = pos + 1;
			}
		}
	}

	bool isACommentOrEmptyLine(const std::string& text)
	{
		return text.size() ? text[0] == '#' : true;
	}
}
