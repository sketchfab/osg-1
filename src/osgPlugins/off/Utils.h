#pragma once

#include <string>
#include <vector>

namespace utils
{
	void	readLines(std::istream& input, std::vector<std::string>& out);	/// Consecutive line returns doesn't add more than one line
	void	explodeString(const std::string& text, const std::vector<char>& separators, std::vector<std::string>& out);

	bool	isACommentOrEmptyLine(const std::string& text);
}
